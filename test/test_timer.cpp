///
/// @file test_timer.cpp
///
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <gtest/gtest.h>
#include "timer.h"

class TimerShould : public ::testing::Test
{
protected:
    std::unique_ptr<timer::Timer> timer_;

    void SetUp(uint32_t tick_time)
    {
        timer_ = std::make_unique<timer::Timer>(tick_time);
    }
};

TEST_F(TimerShould, SubscribeCallbackButDontStartTimer)
{
    auto counter = 0;
    SetUp(2);
    timer_->SubscribeTimerCallback([&counter]() { counter++; }, 4);

    ASSERT_EQ(counter, 0);
}

TEST_F(TimerShould, SubscribeCallbackAndVerifyCounterValue)
{
    auto counter = 0;
    const auto expected = 10;
    SetUp(1);
    auto stop = false;
    auto cv = std::condition_variable{};
    timer_->SubscribeTimerCallback(
        [&counter, &stop, &expected, &cv]() {
            if (!stop)
            {
                counter++;
                stop = (counter == expected);
                if (stop)
                    cv.notify_one();
            }
        },
        2);

    timer_->Start();

    auto dummy_lock = std::mutex{};
    auto ulk = std::unique_lock{dummy_lock};
    cv.wait(ulk);

    ASSERT_EQ(counter, expected);
}

TEST_F(TimerShould, SubscribeTwoCallbacksAndVerifyCounterValues)
{
    auto counter1 = 0;
    auto counter2 = 0;
    const auto expected1 = 10;
    const auto expected2 = 20;
    auto cv = std::condition_variable{};

    auto finish = [&counter1, &counter2, &cv, &expected1, &expected2](){
                if (counter1 == expected1 && counter2 == expected2)
                    cv.notify_one();
    };

    SetUp(1);
    timer_->SubscribeTimerCallback(
        [&counter1, &expected1, &finish]() {
            if (counter1 != expected1)
                counter1++;
            else
                finish();
        },
        2);

    timer_->Start();

    timer_->SubscribeTimerCallback(
        [&counter2, expected2, &finish]() {
            if (counter2 != expected2)
                counter2++;
            else
                finish();
        },
        3);

    auto dummy_lock = std::mutex{};
    auto ulk = std::unique_lock{dummy_lock};
    cv.wait(ulk);

    EXPECT_EQ(counter1, expected1);
    EXPECT_EQ(counter2, expected2);
}

TEST_F(TimerShould, SubscribeCallbackWithNonMultipleOfTickDuration)
{
    auto counter = 0;
    const auto expected = 0;
    SetUp(2);
    timer_->SubscribeTimerCallback([&counter, &expected] { counter++; }, 17);
    timer_->Start();

    timer::WaitForDuration(20);

    EXPECT_EQ(counter, expected);
}

TEST_F(TimerShould, SubscribeAndUnsubscribeCallbackAndVerifyCounterValue)
{
    auto counter = 0;
    const auto expected = 5;
    SetUp(1);
    auto& tok = timer_->SubscribeTimerCallback([&counter] { counter++; }, 2);
    timer_->Start();

    timer::WaitForDuration(11);

    timer_->UnsubscribeTimerCallback(tok);

    timer::WaitForDuration(10);

    EXPECT_LE(counter, expected);
}

TEST_F(TimerShould, SubscribeAndStopShouldStopCallback)
{
    SetUp(1);
    auto stop = false;
    auto callback_called_after_stop = false;
    auto& tok = timer_->SubscribeTimerCallback(
        [&stop, &callback_called_after_stop] {
            callback_called_after_stop = stop;
        },
        2);

    timer_->Start();

    timer::WaitForDuration(10);

    timer_->Stop();

    ASSERT_FALSE(callback_called_after_stop);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
