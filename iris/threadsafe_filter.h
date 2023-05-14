#pragma once
#include <pex/locks.h>


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
    using Control = typename Group::template Control<void>;
    using Terminus = typename Group::template Terminus<ThreadsafeFilter>;

    ThreadsafeFilter(Control control)
        :
        mutex_(),
        terminus_(this, control),
        filter_(control.Get())
    {
        this->terminus_.Connect(&ThreadsafeFilter::OnSettings_);
    }

    // Allows move-construction.
    // Derived classes must WriteLock the mutex prior to moving.
    // The mutex is never moved.
    ThreadsafeFilter(ThreadsafeFilter &&other)
        :
        ThreadsafeFilter(std::move(other), WriteLock(other.mutex_))
    {
        this->terminus_.Connect(&ThreadsafeFilter::OnSettings_);
    }

    template<typename... Inputs>
    auto Filter(Inputs&&... inputs) const
    {
        ReadLock lock(this->mutex_);
        return this->filter_.Filter(std::forward<Inputs>(inputs)...);
    }

private:
    ThreadsafeFilter(ThreadsafeFilter &&other, const WriteLock &)
        :
        mutex_(),
        terminus_(this, std::move(other.terminus_)),
        filter_(other.terminus_.Get())
    {

    }

    void OnSettings_(const Settings &settings)
    {
        WriteLock lock(this->mutex_);
        this->filter_ = Filter_(settings);
    }

private:
    mutable Mutex mutex_;
    Terminus terminus_;
    Filter_ filter_;
};


} // end namespace iris
