#pragma once


#include <jive/thread_pool.h>
#include <fields/fields.h>
#include <pex/group.h>
#include <wxpex/async_range.h>


namespace iris
{


template<typename T>
struct ThreadPoolFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::loadFactor, "loadFactor"),
        fields::Field(&T::concurrency, "concurrency"),
        fields::Field(&T::queuedCount, "queuedCount"),
        fields::Field(&T::activeCount, "activeCount"),
        fields::Field(&T::pressure, "pressure"));
};


template<template<typename> typename T>
struct ThreadPoolTemplate
{
    using LoadFactor =
        wxpex::AsyncRange<double, pex::Limit<0, 1, 100>, pex::Limit<1>>;

    T<LoadFactor> loadFactor;

    T<wxpex::ReadOnlyAsync<size_t>> concurrency;
    T<wxpex::ReadOnlyAsync<size_t>> queuedCount;
    T<wxpex::ReadOnlyAsync<int64_t>> activeCount;
    T<wxpex::ReadOnlyAsync<double>> pressure;

    static constexpr auto fields = ThreadPoolFields<ThreadPoolTemplate>::fields;
};


struct ThreadPoolCustom
{
    template<typename Base>
    struct Plain: public Base
    {
    public:
        Plain()
            :
            Base{}
        {
            this->loadFactor = 0.5;
        }
    };

    template<typename Base>
    struct Model: public Base
    {
    public:
        Model()
            :
            Base{},
            isRunning_(true),
            mutex_(),
            ignore_(false),
            threadPool_(jive::GetThreadPool()),

            loadFactorEndpoint_(
                this,
                this->loadFactor,
                &Model::OnLoadFactor_),

            monitorThread_()
        {
            {
                jive::ScopeFlag ignore(this->ignore_);

                this->loadFactor.SetMinimum(
                    this->threadPool_->GetMinLoadFactor());
            }

            this->OnLoadFactor_(this->loadFactor.Get());

            this->monitorThread_ =
                std::thread(std::bind(&Model::Monitor_, this));
        }

        void Shutdown()
        {
            this->isRunning_ = false;

            if (this->monitorThread_.joinable())
            {
                this->monitorThread_.join();
            }
        }

        ~Model()
        {
            this->Shutdown();
        }

    private:
        void OnLoadFactor_(double loadFactor_)
        {
            if (this->ignore_)
            {
                return;
            }

            this->threadPool_->SetLoadFactor(loadFactor_);

            jive::ScopeFlag ignore(this->ignore_);
            this->loadFactor.Set(this->threadPool_->GetLoadFactor());
        }

        void Monitor_()
        {
            auto workerConcurrency = this->concurrency.GetWorkerControl();
            auto workerQueuedCount = this->queuedCount.GetWorkerControl();
            auto workerActiveCount = this->activeCount.GetWorkerControl();
            auto workerPressure = this->pressure.GetWorkerControl();

            while (this->isRunning_)
            {
                using namespace std::chrono_literals;

                std::this_thread::sleep_for(100ms);

                pex::AccessReference(workerConcurrency)
                    .SetOverride(this->threadPool_->GetConcurrency());

                pex::AccessReference(workerQueuedCount)
                    .SetOverride(this->threadPool_->GetQueuedCount());

                pex::AccessReference(workerActiveCount)
                    .SetOverride(this->threadPool_->GetActiveCount());

                pex::AccessReference(workerPressure)
                    .SetOverride(this->threadPool_->GetPressure());
            }
        }

    private:
        bool isRunning_;
        std::mutex mutex_;
        bool ignore_;
        std::shared_ptr<jive::ThreadPool> threadPool_;

        using LoadFactorEndpoint =
            pex::Endpoint<Model, decltype(Model::loadFactor)>;

        LoadFactorEndpoint loadFactorEndpoint_;

        std::thread monitorThread_;
    };
};


using ThreadPoolGroup = pex::Group<ThreadPoolTemplate, ThreadPoolCustom>;

using ThreadPool = typename ThreadPoolGroup::Plain;
using ThreadPoolModel = typename ThreadPoolGroup::Model;
using ThreadPoolControl = typename ThreadPoolGroup::DefaultControl;


} // end namespace iris
