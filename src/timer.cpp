///
/// @file timer.cpp
///
#include "timer.h"
#include <chrono>
#include <condition_variable>
#include <future>
#include <mutex>
#include <vector>

namespace timer
{

using timer_callback_list_t = std::unordered_map<Token, std::pair<time_ms, timer_callback_t>>;

struct Token
{
    decltype(std::chrono::high_resolution_clock::now()) time_point;

    auto operator==(const Token& that) const { return time_point == that.time_point; }
};

}  // namespace timer

namespace std
{
template <>
struct hash<timer::Token>
{
    auto operator()(const timer::Token& val) const { return val.time_point.time_since_epoch().count(); }
};
}  // namespace std

namespace timer_internal
{
struct Data
{
    std::mutex callback_list_lock_;
    timer::timer_callback_list_t callbacks_;
    bool stop_;
    std::condition_variable cv_{};
    std::future<void> timer_thread_fut_;
    timer::time_ms elapsed_time_;
    const timer::time_ms tick_duration_ms_;

    Data(timer::time_ms tick_duration_ms)
        : callback_list_lock_{},
          callbacks_{},
          stop_{false},
          cv_{},
          timer_thread_fut_{},
          elapsed_time_{0},
          tick_duration_ms_{tick_duration_ms}
    {
    }
};

}  // namespace timer_internal

namespace timer
{
Timer::Timer(time_ms duration) : data_{std::make_unique<timer_internal::Data>(duration)} {}

// Move constructor is needed to be implemented since
// we are using a forward declaration of Data.
Timer::Timer(Timer&& that) : data_{std::move(that.data_)} {}

Timer::~Timer()
{
    Stop();
}

auto Timer::SubscribeTimerCallback(timer_callback_t callback, time_ms frequency) -> const Token&
{
    const auto tok = Token{std::chrono::high_resolution_clock::now()};
    auto ulk = std::scoped_lock(data_->callback_list_lock_);
    data_->callbacks_[tok] = {frequency, callback};

    return data_->callbacks_.find(tok)->first;
}

auto Timer::UnsubscribeTimerCallback(const Token& token) -> bool
{
    auto ulk = std::scoped_lock(data_->callback_list_lock_);

    return data_->callbacks_.erase(token) == 1;
}

auto Timer::Start() -> void
{
    if (data_->timer_thread_fut_.valid())
    {
        return;
    }

    data_->stop_ = false;
    data_->elapsed_time_ = 0;

    using namespace std::chrono;
    data_->timer_thread_fut_ = std::async([this]() -> void {
        const auto duration = std::chrono::duration<time_ms, std::milli>(data_->tick_duration_ms_);
        auto dummy_lock = std::mutex{};
        auto ulk = std::unique_lock{dummy_lock};

        while (!data_->stop_)
        {
            data_->cv_.wait_until(ulk, system_clock::now() + duration, [this]() { return data_->stop_; });

            data_->elapsed_time_ += data_->tick_duration_ms_;

            if (data_->stop_)
            {
                break;
            }

            // TODO: Currently the time stops, when all the callbacks are being called.
            // TODO: Should the time continue and have this executed in another thread?
            // TODO: Use a multimap to store the callbacks, and then use std::upper_bound
            // TODO: to find out the number of functions to be called, rather than looping over all
            // TODO: Also, to reduce latency, and let the timer thread continue, push the callback
            // TODO: objects into a queue, which would then be called from another thread.
            auto clk = std::scoped_lock{data_->callback_list_lock_};
            for (const auto& key_val_pair : data_->callbacks_)
            {
                if (data_->elapsed_time_ % key_val_pair.second.first == 0)
                {
                    key_val_pair.second.second();
                }
                if (data_->stop_)
                {
                    break;
                }
            }
        }
    });
}

auto Timer::Stop() -> void
{
    // The check for the presence of data is needed,
    // since when the move constructor resets the data to null,
    // it causes failure here.
    if (data_ && data_->timer_thread_fut_.valid())
    {
        data_->stop_ = true;
        data_->cv_.notify_one();
        data_->timer_thread_fut_.get();
    }
}

auto WaitForDuration(int64_t wait_duration_ms) -> void
{
    std::condition_variable cv{};
    std::mutex lock {};
    std::unique_lock<std::mutex> ulk{lock};

    cv.wait_until(ulk,
                std::chrono::system_clock::now() + std::chrono::duration<int64_t, std::milli>(wait_duration_ms),
                [](){ return false; });
}

auto GetCurrentSystemTimeNS() -> int64_t
{
    return std::chrono::high_resolution_clock::now().time_since_epoch().count();
}

}  // namespace timer
