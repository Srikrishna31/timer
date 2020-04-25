///
/// @file timer.h
///
#ifndef TIMER_INCLUDE_TIMER_H
#define TIMER_INCLUDE_TIMER_H

#include <functional>
#include <memory>

namespace timer_internal
{
struct Data;
}

namespace timer
{

/// @brief A token object that will be returned when a callback is registered.
/// This object is needed to stop receiving the callbacks.
struct Token;

/// @brief The callback function
using timer_callback_t = std::function<void()>;

/// @brief Time type which is currently only in milliseconds.
using time_ms = std::uint32_t;

/// @brief The Timer class is a simple utility class which
/// provides time based callback facilities. Anyone with a
/// requirement to have a callback(s) with a particular frequency(s)
/// can utilize this class to have the callback automatically
/// called.
/// The callbacks can be added even after the timer has been started.
/// It is the client's responsibility to make sure that the timer object
/// is created with LCD(Least Common Denominator) of all the frequencies
/// of the callback objects. If this is not followed, then the behavior is
/// undefined - callbacks will be called depending on the factor of the
/// tick_duration.
class Timer final
{
  public:
    /// @brief Timer
    /// @param tick_duration: Minimum duration in milliseconds, with which the time shall be counted.
    explicit Timer(time_ms tick_duration_ms);

    Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;
    Timer& operator=(Timer&&) = delete;

    Timer(Timer&& that);
    ~Timer();

    /// @brief SubscribeTimerCallback : Function to register a callback.
    /// @param callback: The callback function that should be called.
    /// @param frequency: The frequency with which this callback function should be called.
    /// @return Token: A token, which should only be stored as a reference by the clients.
    /// It cannot be captured by value.
    auto SubscribeTimerCallback(timer_callback_t callback, time_ms frequency) -> const Token&;

    /// @brief UnsubscribeTimerCallback: Function to stop receiving the callback.
    /// @param token: The token that was provided when the corresponding callback was registered.
    /// @return bool: Status indicating if the token was found and callback was disabled.
    auto UnsubscribeTimerCallback(const Token& token) -> bool;

    /// @brief Start: Start the timer, to start receiving the callbacks.
    auto Start() -> void;

    /// @brief Stop: Stop the timer, to stop receiving all the callbacks.
    /// The function tries to return as soon as possible, so it is not guaranteed
    /// that all the callbacks will be called once this function is called.
    auto Stop() -> void;

  private:
    std::unique_ptr<timer_internal::Data> data_;
};

auto WaitForDuration(int64_t wait_duration_ms) -> void;

auto GetCurrentSystemTimeNS() -> int64_t;
}  // namespace timer
#endif  // TIMER_INCLUDE_TIMER_H
