#pragma once


#include <mutex>
#include <optional>
#include <pex/endpoint.h>
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
        __FUNCTION__, \
        ":", \
        __LINE__, \
        "] ", \
        __VA_ARGS__); assert(std::cout.good())

#else

#define NODE_LOG(...)

#endif // ENABLE_NODE_LOG


namespace iris
{


using Cancel = pex::model::Value<bool>;
using CancelControl = pex::control::Value<Cancel>;



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
    using Settings = typename Control::Type;
    using NodeEndpoint = pex::Endpoint<NodeBase, Control>;

    NodeBase(InputNode &&, Control, CancelControl) = delete;
    NodeBase(const NodeBase &other) = delete;
    NodeBase(NodeBase &&other) = delete;
    NodeBase & operator=(const NodeBase &other) = delete;
    NodeBase & operator=(NodeBase &&other) = delete;

#if 0
    NodeBase(InputNode &input, Control control, CancelControl cancel)
        :
        mutex_(),
        input_(input),
        settings_(control.Get()),
        settingsChanged_(false),
        endpoint_(this, control, &NodeBase::OnSettingsChanged),
        name_(),
        cancel_(cancel),
        result_()
    {

    }
#endif

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
        endpoint_(this, control, &NodeBase::OnSettingsChanged),
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
            hasResult = this->result_.has_value();
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
        std::lock_guard lock(this->mutex_);
        this->settings_ = settings;
        this->settingsChanged_ = true;
        static_cast<Derived *>(this)->SettingsChanged(this->settings_);
        this->result_.reset();
    }

    // Derived classes may not care about changed settings.
    void SettingsChanged(const Settings &)
    {

    }

    std::optional<Result> GetResult()
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

            if (this->result_.has_value())
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

        auto result = static_cast<Derived *>(this)->DoGetResult();

        std::lock_guard lock(this->mutex_);

        if (this->settingsChanged_)
        {
            NODE_LOG("settingsChanged_, no result for you: ", this->name_);
            return {};
        }

        if (!result)
        {
            NODE_LOG(
                this->name_,
                " DoGetResult() did not return a valid result, "
                "but settings did NOT change.");

            return {};
        }

        NODE_LOG("Cache and return result: ", this->name_);

        return this->result_ = result;
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
    mutable std::optional<Result> result_;

};


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

#if 0
    Node(InputNode &input, Control control, CancelControl cancel)
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

    }

    const void * GetFilterAddress() const
    {
        return &this->filter_;
    }

    void SettingsChanged(const Settings &settings)
    {
        this->filter_ = FilterClass(settings);
    }

    std::optional<Result> DoGetResult()
    {
        FilterClass filter;

        {
            std::lock_guard lock(this->mutex_);
            filter = this->filter_;
        }

        auto input = this->input_.GetResult();

        if (!input)
        {
            NODE_LOG(this->name_, " has no input");
            return {};
        }

        {
            std::lock_guard lock(this->mutex_);

            if (this->settingsChanged_)
            {
                // The settings changed while waiting for input.
                NODE_LOG("Settings changed while waiting for input!");
                return {};
            }
        }

#ifdef ENABLE_NODE_CHRONO
        using Period = std::chrono::duration<double, std::micro>;
        using Clock = std::chrono::steady_clock;
        using TimePoint = std::chrono::time_point<Clock, Period>;

        TimePoint start = Clock::now();
#endif

        auto result = filter.Filter(*input);

#ifdef ENABLE_NODE_CHRONO
        TimePoint end = Clock::now();
        std::cout << this->name_ << ": " << (end - start).count() << " µs\n";
#endif

        std::lock_guard lock(this->mutex_);

        if (!this->settingsChanged_)
        {
            // Cache the filter for later use.
            this->filter_ = filter;
        }

        if (!result)
        {
            NODE_LOG(this->name_, " filter.Filter returned no result.");
        }

        return result;
    }

private:
    FilterClass filter_;
};


template<typename Data>
class Source
{
public:
    using Result = Data;

    Source()
        :
        hasFreshData_(false),
        data_()
    {

    }

    void SetData(const Data &data)
    {
        NODE_LOG("Source::SetData");
        this->data_ = data;
        this->hasFreshData_ = true;
    }

    // Returns true if this data has been retrieved before.
    // Allows the processing chain to decide whether it can use cached results.
    bool HasResult() const
    {
        return !this->hasFreshData_;
    }

    std::optional<Data> GetResult()
    {
        this->hasFreshData_ = false;
        return this->data_;
    }

private:
    bool hasFreshData_;
    std::optional<Data> data_;
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
    using FirstResult = typename FirstNode::Result;
    using SecondResult = typename SecondNode::Result;

    Mix(FirstNode &first, SecondNode &second, CancelControl cancel)
        :
        mutex_(),
        first_(first),
        second_(second),
        cancel_(cancel),
        firstResult_(),
        secondResult_()
    {

    }

    Mix(FirstNode &&, SecondNode &&, CancelControl) = delete;
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

        return (hasResult
            && this->first_.HasResult()
            && this->second_.HasResult());
    }

    std::optional<Result> GetResult()
    {
        if (this->cancel_.Get())
        {
            return {};
        }

        if (this->HasResult())
        {
            return Result{*this->firstResult_, *this->secondResult_};
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

        return Result{*firstResult, *secondResult};
    }

private:
    mutable std::mutex mutex_;
    FirstNode & first_;
    SecondNode & second_;
    CancelControl cancel_;
    mutable std::optional<FirstResult> firstResult_;
    mutable std::optional<SecondResult> secondResult_;
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
    using FirstResult = typename FirstNode::Result;
    using SecondResult = typename SecondNode::Result;
    using MuxModel = pex::model::Value<bool>;
    using MuxControl = pex::control::Value<MuxModel>;

    using Result = Result_;

    Mux(
        FirstNode &first,
        SecondNode &second,
        MuxControl muxControl,
        CancelControl cancel)
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
            return this->firstResult_.has_value();
        }
        else
        {
            return this->secondResult_.has_value();
        }
    }

    std::optional<Result> GetResult()
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
                return Result{this->firstResult_, {}};
            }

            return Result{{}, this->secondResult_};
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

            return Result{firstResult, {}};
        }

        auto secondResult = this->second_.GetResult();

        std::lock_guard lock(this->mutex_);
        this->secondResult_ = secondResult;

        if (!secondResult || this->cancel_.Get())
        {
            return {};
        }

        return Result{{}, secondResult};
    }

private:
    mutable std::mutex mutex_;
    FirstNode & first_;
    SecondNode & second_;
    MuxControl muxControl_;
    CancelControl cancel_;
    mutable std::optional<FirstResult> firstResult_;
    mutable std::optional<SecondResult> secondResult_;
};


} // end namespace iris
