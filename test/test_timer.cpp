///
/// @file test_timer.cpp
///
#include <atomic>
#include <condition_variable>
#include <gtest/gtest.h>
#include "timer.h"

using namespace std::chrono_literals;

void WaitForCondition(std::function<bool()> predicate)
{
    auto cv = std::condition_variable{};
    auto lock = std::mutex{};
    auto ulk = std::unique_lock{lock};

    cv.wait(ulk, predicate);
}

TEST(TimerTest, SubscribeCallbackButDontStartTimer)
{
    auto counter = 0;
    auto t = timer::Timer{2};
    t.SubscribeTimerCallback([&counter]() { counter++; }, 4);

    ASSERT_EQ(counter, 0);
}

TEST(TimerTest, SubscribeCallbackAndVerifyCounterValue)
{
    auto counter = 0;
    const auto expected = 10;
    auto t = timer::Timer{1};
    auto stop = std::atomic{false};
    t.SubscribeTimerCallback(
        [&counter, &stop, &expected]() {
            if (!stop)
            {
                counter++;
                std::cout << "Counter value: " << counter << std::endl;
                stop = (counter == expected);
            }
        },
        2);
    t.Start();

    timer::WaitForDuration(100);

    ASSERT_EQ(counter, expected);
}

TEST(TimerTest, SubscribeTwoCallbacksAndVerifyCounterValues)
{
    auto counter1 = 0;
    auto counter2 = 0;
    const auto expected1 = 10;
    const auto expected2 = 20;
    auto t = timer::Timer{1};
    t.SubscribeTimerCallback(
        [&counter1, expected1]() {
            if (counter1 != expected1)
                counter1++;
        },
        2);
    t.Start();

    t.SubscribeTimerCallback(
        [&counter2, expected2]() {
            if (counter2 != expected2)
                counter2++;
        },
        3);

    timer::WaitForDuration(100);

    EXPECT_EQ(counter1, expected1);
    EXPECT_EQ(counter2, expected2);
}

TEST(TimerTest, SubscribeCallbackWithNonMultipleOfTickDuration)
{
    auto counter = 0;
    const auto expected = 0;
    auto t = timer::Timer{2};
    t.SubscribeTimerCallback([&counter, &expected] { counter++; }, 17);
    t.Start();

    timer::WaitForDuration(20);

    EXPECT_EQ(counter, expected);
}

TEST(TimerTest, SubscribeAndUnsubscribeCallbackAndVerifyCounterValue)
{
    auto counter = 0;
    const auto expected = 5;
    auto t = timer::Timer{1};
    auto& tok = t.SubscribeTimerCallback([&counter, &expected] { counter++; }, 2);
    t.Start();
    timer::WaitForDuration(11);

    t.UnsubscribeTimerCallback(tok);

    timer::WaitForDuration(10);

    EXPECT_LE(counter, expected);
}

TEST(TimerTest, SubscribeAndStopShouldStopCallback)
{
    auto t = timer::Timer{1};
    auto stop = false;
    auto callback_called_after_stop = false;
    auto& tok = t.SubscribeTimerCallback(
        [&stop, &callback_called_after_stop] {
            if (stop)
            {
                callback_called_after_stop = true;
            }
        },
        2);

    t.Start();

    timer::WaitForDuration(10);

    t.Stop();

    ASSERT_FALSE(callback_called_after_stop);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
