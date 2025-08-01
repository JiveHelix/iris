#pragma once


#include <pex/locks.h>
#include <pex/endpoint.h>


namespace iris
{


using Mutex = pex::Mutex;
using ReadLock = pex::ReadLock;
using WriteLock = pex::WriteLock;


template<typename Group, typename Filter_>
class ThreadsafeFilter
{
public:
    using Settings = typename Group::Plain;
    using Control = typename Group::Control;
    using SettingsEndpoint = pex::Endpoint<ThreadsafeFilter, Control>;

    ThreadsafeFilter(Control control)
        :
        mutex_(),

        endpoint_(
            PEX_THIS("ThreadsafeFilter"),
            control,
            &ThreadsafeFilter::OnSettings_),

        filter_(control.Get())
    {

    }

    // Allows move-construction.
    // Derived classes must WriteLock the mutex prior to moving.
    // The mutex is never moved.
    ThreadsafeFilter(ThreadsafeFilter &&other)
        :
        ThreadsafeFilter(std::move(other), WriteLock(other.mutex_))
    {

    }

    template<typename... Inputs>
    auto Filter(Inputs&&... inputs) const
    {
        ReadLock lock(this->mutex_);
        return this->filter_.Filter(std::forward<Inputs>(inputs)...);
    }

    Settings GetSettings() const
    {
        return this->endpoint_.Get();
    }

private:
    ThreadsafeFilter(ThreadsafeFilter &&other, const WriteLock &)
        :
        mutex_(),
        endpoint_(
            this,
            other.endpoint_),
        filter_(this->endpoint_.Get())
    {

    }

    void OnSettings_(const Settings &settings)
    {
        WriteLock lock(this->mutex_);
        this->filter_ = Filter_(settings);
    }

private:
    mutable Mutex mutex_;
    SettingsEndpoint endpoint_;
    Filter_ filter_;
};


} // end namespace iris
