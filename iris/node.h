#pragma once


#include <mutex>
#include <optional>
#include <pex/terminus.h>



namespace iris
{


using Cancel = pex::model::Value<bool>;
using CancelControl = pex::control::Value<void, Cancel>;


template
<
    typename InputNode,
    typename FilterClass,
    typename Control
>
class Node
{
public:
    using Result = typename FilterClass::Result;
    using Settings = typename Control::Type;
    using Terminus = pex::Terminus<Node, Control>;

    Node(InputNode &&input, Control control) = delete;
    Node(const Node &other) = delete;
    Node(Node &&other) = delete;
    Node & operator=(const Node &other) = delete;
    Node & operator=(Node &&other) = delete;

    Node(InputNode &input, Control control, CancelControl cancel)
        :
        mutex_(),
        input_(input),
        settings_(control.Get()),
        terminus_(this, control),
        cancel_(cancel),
        filter_(this->settings_),
        result_()
    {
        this->terminus_.Connect(&Node::OnSettings_);
    }

    bool HasResult() const
    {
        bool hasResult;

        {
            std::lock_guard lock(this->mutex_);
            hasResult = !!this->result_;
        }

        return (hasResult && this->input_.HasResult());
    }

    void OnSettings_(const Settings &settings)
    {
        std::lock_guard lock(this->mutex_);
        this->settings_ = settings;
        this->result_.reset();
    }

    std::optional<Result> GetResult()
    {
        if (this->cancel_.Get())
        {
            return {};
        }

        if (this->HasResult())
        {
            return this->result_;
        }

        {
            std::lock_guard lock(this->mutex_);
            this->filter_ = FilterClass(this->settings_);
        }

        auto input = this->input_.GetResult();

        if (!input || this->cancel_.Get())
        {
            return {};
        }

        this->result_ = this->filter_.Filter(*input);

        return this->result_;
    }

private:
    mutable std::mutex mutex_;
    InputNode & input_;
    Settings settings_;
    Terminus terminus_;
    CancelControl cancel_;
    FilterClass filter_;
    std::optional<Result> result_;
};


} // end namespace iris
