#pragma once


#include <mutex>
#include <optional>
#include <pex/endpoint.h>
#include <tau/convolve.h>
#include <iris/filter_result.h>
#include "iris/default.h"

// #define ENABLE_NODE_CHRONO

#ifdef ENABLE_NODE_CHRONO
#include <chrono>
#include <iostream>
#endif


#ifdef ENABLE_NODE_LOG
#include <pex/log.h>

#define NODE_LOG(...) \
\
    pex::ToStream( \
        std::cout, \
        "[node:", \
        jive::path::Base(__FILE__), \
        ":", \
        __LINE__, \
        "] ", \
        __VA_ARGS__); assert(std::cout.good())

#else

#define NODE_LOG(...)

#endif // ENABLE_NODE_LOG

#ifdef ENABLE_NODE_CHRONO
        using Period = std::chrono::duration<double, std::micro>;
        using Clock = std::chrono::steady_clock;
        using TimePoint = std::chrono::time_point<Clock, Period>;
#endif


namespace iris
{


using Cancel = pex::model::Value<bool>;
using CancelControl = pex::control::Value<Cancel>;

using MarginModel = pex::model::Value<tau::Margins>;
using MarginControl = pex::control::Value<MarginModel>;


template<typename Data>
std::shared_ptr<const Data> ChangeMargins(
    [[maybe_unused]] const tau::Margins &oldMargins,
    const tau::Margins &newMargins,
    const Data &data)
{
    if constexpr (tau::HasAddMargin<Data> && tau::HasRemoveMargin<Data>)
    {
        // Remove the old margin.
        auto removed = data.RemoveMargin(oldMargins);

        // Use the new margin to expand the data.
        return std::make_shared<Data>(removed.AddMargin(newMargins));
    }
    else if constexpr (
        tau::HasAddMargin<Data> && tau::HasRemoveMarginVoid<Data>)
    {
        // data knows what the old margin was.
        // Remove the old margin.
        auto removed = data.RemoveMargin();

        // Use the new margin to expand the data.
        return std::make_shared<Data>(removed.AddMargin(newMargins));
    }
    else
    {
        // Margins knows how to operate on Eigen arrays.
        static_assert(tau::IsEigen<std::remove_cvref_t<Data>>);

        // Remove the old margin.
        // Then use the new margin to expand the data.
        return std::make_shared<Data>(
            newMargins.AddMargin(oldMargins.RemoveMargin(data)));
    }
}


template
<
    typename InputNode,
    typename Control,
    typename Result_,
    typename Derived
>
class NodeBase
{
public:
    using Result = Result_;
    using ResultPtr = std::shared_ptr<const Result>;
    using Input = typename InputNode::Result;
    using InputPtr = typename InputNode::ResultPtr;
    using Settings = typename Control::Type;
    using NodeEndpoint = pex::Endpoint<NodeBase, Control>;

    NodeBase(InputNode &&, Control, CancelControl) = delete;
    NodeBase(const NodeBase &other) = delete;
    NodeBase(NodeBase &&other) = delete;
    NodeBase & operator=(const NodeBase &other) = delete;
    NodeBase & operator=(NodeBase &&other) = delete;

    NodeBase(
        const std::string &name,
        InputNode &input,
        Control control,
        CancelControl cancel)
        :
        mutex_(),
        input_(input),
        settings_(control.Get()),
        settingsChanged_(false),

        endpoint_(
            PEX_THIS("NodeBase"),
            control,
            &NodeBase::OnSettingsChanged),

        name_(name),
        cancel_(cancel),
        result_()
    {

    }

    bool HasResult() const
    {
        bool hasResult;

        {
            std::lock_guard lock(this->mutex_);
            hasResult = !!this->result_;
        }

        if (!this->input_.HasResult())
        {
            std::lock_guard lock(this->mutex_);
            this->result_.reset();

            return false;
        }

        return hasResult;
    }

    void OnSettingsChanged(const Settings &settings)
    {
        {
            std::lock_guard lock(this->mutex_);
            this->settings_ = settings;
            this->settingsChanged_ = true;
            static_cast<Derived *>(this)->SettingsChanged(this->settings_);
            this->result_.reset();
        }

        auto filterMargins = this->ComputeRequiredMargins();

        if (filterMargins.HasMargin())
        {
            auto inputMargins = this->input_.GetMargins();

            if (!inputMargins.Contains(filterMargins))
            {
                // The input margins are not large enough to enclose the
                // new filter requirements.
                // Pass the new requirement up the processing chain.
                this->input_.SetMargins(filterMargins);
            }
        }
    }

    // Derived classes may not care about changed settings.
    void SettingsChanged(const Settings &)
    {

    }

    tau::Margins GetMargins() const
    {
        return this->input_.GetMargins();
    }

    tau::Margins ComputeRequiredMargins() const
    {
        return static_cast<const Derived *>(this)->DoComputeRequiredMargins();
    }

    tau::Margins ComputeMinimumMargins() const
    {
        return tau::ComputeMaximumMargins(
            this->input_.ComputeMinimumMargins(),
            this->ComputeRequiredMargins());
    }

    void SetMargins(const tau::Margins &margins)
    {
        if (this->HasResult())
        {
            auto oldMargins = this->input_.GetMargins();

            std::lock_guard lock(this->mutex_);

            if (this->result_)
            {
                this->result_ = ChangeMargins(
                    oldMargins,
                    margins,
                    *this->result_);
            }
        }

        this->input_.SetMargins(margins);
    }

    ResultPtr GetResult()
    {
        if (this->cancel_.Get())
        {
            NODE_LOG("Node canceled: ", this->name_);
            return {};
        }

        if (this->HasResult())
        {
            // HasResult checks the input node as well our result_.
            // Between releasing the lock in HasResult and reacquiring it here,
            // it is possible that result_ has been reset by a call to
            // OnSettingsChanged.
            // While it is tempting to make the mutex recursive, and hold it
            // from before the call to HasResult, it would mean attempting to
            // acquire the mutexes of other nodes while holding our own.
            std::lock_guard lock(this->mutex_);

            if (this->result_)
            {
                NODE_LOG("Returning cached result: ", this->name_);
                return this->result_;
            }
        }

        NODE_LOG("Computing new result: ", this->name_);

        {
            std::lock_guard lock(this->mutex_);
            this->settingsChanged_ = false;
        }

        auto resultPtr = static_cast<Derived *>(this)->DoGetResult();

        std::lock_guard lock(this->mutex_);

        if (this->settingsChanged_)
        {
            NODE_LOG("settingsChanged_, no result for you: ", this->name_);
            return {};
        }

        if (!resultPtr)
        {
            NODE_LOG(
                this->name_,
                " DoGetResult() did not return a valid result, "
                "but settings did NOT change.");

            return {};
        }

        NODE_LOG("Cache and return resultPtr: ", this->name_);

        return this->result_ = resultPtr;
    }

protected:
    mutable std::mutex mutex_;
    InputNode & input_;
    Settings settings_;
    bool settingsChanged_;
    NodeEndpoint endpoint_;
    std::string name_;

private:
    CancelControl cancel_;
    mutable ResultPtr result_;
};


template<typename T>
tau::Size<Eigen::Index> GetSize(const T &input)
{
    if constexpr (tau::IsEigen<T>)
    {
        return tau::Size<Eigen::Index>(input);
    }
    else
    {
        return input.GetSize();
    }
}


template<typename T>
void Resize(const tau::Size<Eigen::Index> &size, T &object)
{
    if constexpr (tau::IsEigen<T>)
    {
        object.resize(size.height, size.width);
    }
    else
    {
        object.Resize(size);
    }
}


template<typename Filter>
concept FilterWantsMargins = Filter::wantsMargins;


template
<
    typename InputNode,
    typename FilterClass,
    typename Control
>
class Node
    :
    public NodeBase
        <
            InputNode,
            Control,
            typename FilterClass::Result,
            Node<InputNode, FilterClass, Control>
        >
{
public:
    // A FilterClass's Filter function can have template arguments, but
    // Result must not depend on those template arguments.
    using Base =
        NodeBase
        <
            InputNode,
            Control,
            typename FilterClass::Result,
            Node<InputNode, FilterClass, Control>
        >;

    using Settings = typename Base::Settings;
    using Result = typename Base::Result;
    using ResultPtr = typename Base::ResultPtr;
    using Input = typename Base::Input;
    using InputPtr = typename Base::InputPtr;

#if 0
    Node(InputNode &input, const Control &control, const CancelControl &cancel)
        :
        Base(input, control, cancel),
        filter_(this->settings_)
    {

    }
#endif

    Node(
        const std::string &name,
        InputNode &input,
        Control control,
        CancelControl cancel)
        :
        Base(name, input, control, cancel),
        filter_(this->settings_)
    {
        auto filterMargins = this->DoComputeRequiredMargins();

        if (filterMargins.HasMargin())
        {
            auto inputMargins = this->input_.GetMargins();

            if (!inputMargins.Contains(filterMargins))
            {
                // The input margins are not large enough to enclose the
                // new filter requirements.
                // Pass the new requirement up the processing chain.
                this->input_.SetMargins(filterMargins);
            }
        }
    }

    const void * GetFilterAddress() const
    {
        return &this->filter_;
    }

    void SettingsChanged(const Settings &settings)
    {
        // Do not acquire the lock here.
        // NodeBase will call this while holding the mutex.
        this->filter_ = FilterClass(settings);
    }

    bool Process(const Input &input, Result &result)
    {
        FilterClass filter;

        {
            std::lock_guard lock(this->mutex_);
            this->settingsChanged_ = false;
            filter = this->filter_;
        }

        bool filterSuccess;
        auto inputMargins = this->input_.GetMargins();
        auto inputSize = GetSize(input);
        auto resultSize = GetSize(result);

        if (!resultSize.Contains(inputSize))
        {
            Resize(inputSize, result);
        }

        if constexpr (FilterWantsMargins<FilterClass>)
        {
            filterSuccess = filter.Filter(input, result, inputMargins);
        }
        else
        {
            filterSuccess = filter.Filter(input, result);
        }

        std::lock_guard lock(this->mutex_);

        if (this->settingsChanged_)
        {
            return false;
        }

        return filterSuccess;
    }

    tau::Margins DoComputeRequiredMargins() const
    {
        std::lock_guard lock(this->mutex_);

        return this->filter_.ComputeRequiredMargins();
    }

    ResultPtr DoGetResult()
    {
        auto inputPtr = this->input_.GetResult();

        if (!inputPtr)
        {
            NODE_LOG(this->name_, " has no input");
            return {};
        }

        auto resultPtr = std::make_shared<Result>();

        if (!this->Process(*inputPtr, *resultPtr))
        {
            NODE_LOG(this->name_, " filter.Filter returned no result.");
            return {};
        }

        if constexpr (std::derived_from<Result, FilterResult>)
        {
            resultPtr->SetMargins(this->GetMargins());
        }

        return resultPtr;
    }

private:
    FilterClass filter_;
};


template<typename Data>
class Source
{
public:
    using Result = Data;
    using ResultPtr = std::shared_ptr<const Result>;

    Source(const tau::Margins &margins = tau::Margins{0, 0})
        :
        margins_(margins),
        hasFreshData_(false),
        data_()
    {

    }

    tau::Margins GetMargins() const
    {
        std::lock_guard lock(this->mutex_);

        return this->margins_;
    }

    tau::Margins ComputeMinimumMargins() const
    {
        // Source has no minimum margin requirement.

        return {0, 0};
    }

    void SetMargins(const tau::Margins &margins)
    {
        std::lock_guard lock(this->mutex_);

        if (this->data_)
        {
            this->data_ = ChangeMargins(
                this->margins_,
                margins,
                *this->data_);
        }

        this->margins_ = margins;

        if (this->data_)
        {
            this->hasFreshData_ = true;
        }
    }

    void SetData(const Data &data)
    {
        std::lock_guard lock(this->mutex_);

        NODE_LOG("Source::SetData");

        // Copy data
        if constexpr (tau::HasAddMargin<Data> && tau::HasRemoveMargin<Data>)
        {
            this->data_ =
                std::make_shared<Result>(data.AddMargin(this->margins_));
        }
        else
        {
            this->data_ =
                std::make_shared<Result>(this->margins_.AddMargin(data));
        }

        this->hasFreshData_ = true;
    }

    // Returns true if this data has been retrieved before.
    // Allows the processing chain to decide whether it can use cached results.
    bool HasResult() const
    {
        std::lock_guard lock(this->mutex_);

        return !this->hasFreshData_ && this->data_;
    }

    ResultPtr GetResult() const
    {
        std::lock_guard lock(this->mutex_);

        this->hasFreshData_ = false;
        return this->data_;
    }

private:
    mutable std::mutex mutex_;
    tau::Margins margins_;
    mutable bool hasFreshData_;
    ResultPtr data_;
};


extern template class Source<ProcessMatrix>;
using DefaultSource = Source<ProcessMatrix>;


template
<
    typename FirstNode,
    typename SecondNode,
    typename Result_
>
class Mix
{
public:
    using Result = Result_;
    using ResultPtr = std::shared_ptr<Result_>;
    using FirstResult = typename FirstNode::ResultPtr;
    using SecondResult = typename SecondNode::ResultPtr;

    Mix(FirstNode &first, SecondNode &second, const CancelControl &cancel)
        :
        mutex_(),
        first_(first),
        second_(second),
        cancel_(cancel),
        firstResult_(),
        secondResult_()
    {

    }

    Mix(FirstNode &&, SecondNode &&, const CancelControl &) = delete;
    Mix(const Mix &other) = delete;
    Mix(Mix &&other) = delete;
    Mix & operator=(const Mix &other) = delete;
    Mix & operator=(Mix &&other) = delete;

    bool HasResult() const
    {
        bool hasResult;

        {
            std::lock_guard lock(this->mutex_);
            hasResult = (this->firstResult_ && this->secondResult_);
        }

        if (!this->first_.HasResult())
        {
            this->firstResult_.reset();
        }

        if (!this->second_.HasResult())
        {
            this->secondResult_.reset();
        }

        return (
            hasResult
            && this->first_.HasResult()
            && this->second_.HasResult());
    }

    tau::Margins GetMargins() const
    {
        assert(
            this->first_.GetMargins()
            == this->second_.GetMargins());

        return this->first_.GetMargins();
    }

    tau::Margins ComputeRequiredMargins() const
    {
        return tau::ComputeMaximumMargins(
            this->first_.ComputeRequiredMargins(),
            this->second_.ComputeRequiredMargins());
    }

    void SetMargins(const tau::Margins &margins)
    {
        this->first_.SetMargins(margins);
        this->second_.SetMargins(margins);
    }

    ResultPtr GetResult()
    {
        if (this->cancel_.Get())
        {
            return {};
        }

        if (this->HasResult())
        {
            return std::make_shared<Result_>(
                *this->firstResult_,
                *this->secondResult_);
        }

        auto firstResult = this->first_.GetResult();
        auto secondResult = this->second_.GetResult();

        std::lock_guard lock(this->mutex_);
        this->firstResult_ = firstResult;
        this->secondResult_ = secondResult;

        if (
            !(firstResult && secondResult)
            || this->cancel_.Get())
        {
            return {};
        }

        return std::make_shared<Result_>(
            *this->firstResult_,
            *this->secondResult_);
    }

private:
    mutable std::mutex mutex_;
    FirstNode & first_;
    SecondNode & second_;
    CancelControl cancel_;
    mutable FirstResult firstResult_;
    mutable SecondResult secondResult_;
};


template
<
    typename FirstNode,
    typename SecondNode,
    typename Result_
>
class Mux
{
public:
    using FirstResult = typename FirstNode::ResultPtr;
    using SecondResult = typename SecondNode::ResultPtr;
    using MuxModel = pex::model::Value<bool>;
    using MuxControl = pex::control::Value<MuxModel>;

    using Result = Result_;
    using ResultPtr = std::shared_ptr<Result_>;

    Mux(
        FirstNode &first,
        SecondNode &second,
        const MuxControl &muxControl,
        const CancelControl &cancel)
        :
        mutex_(),
        first_(first),
        second_(second),
        muxControl_(muxControl),
        cancel_(cancel),
        firstResult_(),
        secondResult_()
    {

    }

    Mux(FirstNode &&, SecondNode &&, CancelControl) = delete;
    Mux(const Mux &other) = delete;
    Mux(Mux &&other) = delete;
    Mux & operator=(const Mux &other) = delete;
    Mux & operator=(Mux &&other) = delete;

    bool HasResult() const
    {
        bool muxFirst = this->muxControl_.Get();

        std::lock_guard lock(this->mutex_);

        if (!this->first_.HasResult())
        {
            this->firstResult_.reset();
        }

        if (!this->second_.HasResult())
        {
            this->secondResult_.reset();
        }

        if (muxFirst)
        {
            return this->firstResult_;
        }
        else
        {
            return this->secondResult_;
        }
    }

    tau::Margins GetMargins() const
    {
        assert(
            this->first_.GetMargins()
            == this->second_.GetMargins());

        return this->first_.GetMargins();
    }

    tau::Margins ComputeRequiredMargins() const
    {
        return tau::ComputeMaximumMargins(
            this->first_.ComputeRequiredMargins(),
            this->second_.ComputeRequiredMargins());
    }

    void SetMargins(const tau::Margins &margins)
    {
        this->first_.SetMargins(margins);
        this->second_.SetMargins(margins);
    }

    ResultPtr GetResult()
    {
        if (this->cancel_.Get())
        {
            return {};
        }

        bool muxFirst = this->muxControl_.Get();

        if (this->HasResult())
        {
            if (muxFirst)
            {
                return std::make_shared<Result_>(this->firstResult_, {});
            }

            return std::make_shared<Result_>({}, this->secondResult_);
        }

        if (muxFirst)
        {
            auto firstResult = this->first_.GetResult();

            std::lock_guard lock(this->mutex_);
            this->firstResult_ = firstResult;

            if (!firstResult || this->cancel_.Get())
            {
                return {};
            }

            return std::make_shared<Result_>(firstResult, {});
        }

        auto secondResult = this->second_.GetResult();

        std::lock_guard lock(this->mutex_);
        this->secondResult_ = secondResult;

        if (!secondResult || this->cancel_.Get())
        {
            return {};
        }

        return std::make_shared<Result_>({}, secondResult);
    }

private:
    mutable std::mutex mutex_;
    FirstNode & first_;
    SecondNode & second_;
    MuxControl muxControl_;
    CancelControl cancel_;
    mutable FirstResult firstResult_;
    mutable SecondResult secondResult_;
};


} // end namespace iris
