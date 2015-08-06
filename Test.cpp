
/**
 * Copyright 2014-2015 Denis Blank <denis.blank@outlook.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "TaskScheduler.hpp"

#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

using namespace tsc;

using Seconds = std::chrono::seconds;

int main(int argc, char* const argv[])
{
    int const result = Catch::Session().run(argc, argv);

    // Attach breakpoint here ,-)
    return result;
}

enum Group
{
    GROUP_0,
    GROUP_1,
    GROUP_2,
    GROUP_3,
    GROUP_4,
    GROUP_5,
    GROUP_6
};

TEST_CASE("TaskScheduler task scheduling basics", "[TaskScheduler]" )
{
    TaskScheduler scheduler;

    std::size_t invoked = 0;
    auto invoke = [&] { ++invoked; };

    SECTION("Tasks are scheduleable")
    {
        scheduler.Schedule(Seconds(2), std::bind(invoke));

        REQUIRE(!invoked);

        scheduler.Update(Seconds(3));

        REQUIRE(invoked);
    }

    SECTION("Tasks are not invoked when its time hasn't come yet")
    {
        scheduler.Schedule(Seconds(10), std::bind(invoke));

        REQUIRE(!invoked);

        scheduler.Update(Seconds(5));

        REQUIRE(!invoked);
    }

    SECTION("Asyncs are invoked immediatly on next update tick")
    {
        scheduler.Async(invoke);

        REQUIRE(!invoked);

        scheduler.Update(Seconds(0));

        REQUIRE(invoked);
    }

    SECTION("Tasks are scheduleable in a group")
    {
        scheduler.Schedule(Seconds(2), GROUP_0, std::bind(invoke));

        REQUIRE(!invoked);

        scheduler.Update(Seconds(3));

        REQUIRE(invoked);
    }

    SECTION("Tasks are scheduleable with a random time between min and max")
    {
        scheduler.Schedule(Seconds(1), Seconds(2), std::bind(invoke));

        REQUIRE(!invoked);

        scheduler.Update(Seconds(3));

        REQUIRE(invoked);
    }

    SECTION("Tasks are invoked depending on its time order")
    {
        scheduler
            // Second
            .Schedule(Seconds(2), [&](TaskContext)
            {
                REQUIRE(invoked == 1);
                invoked = 2;
            })
            // First
            .Schedule(Seconds(1), [&](TaskContext)
            {
                REQUIRE(invoked == 0);
                invoked = 1;
            })
            // Third
            .Schedule(Seconds(3), [&](TaskContext)
            {
                REQUIRE(invoked == 2);
                invoked = 3;
            });

        scheduler.Update(Seconds(10));

        REQUIRE(invoked == 3);

        scheduler.Update(Seconds(10));
    }
}

TEST_CASE("TaskScheduler task canceling", "[TaskScheduler]" )
{
    TaskScheduler scheduler;

    bool aInvoked = false;
    bool bInvoked = false;
    auto invokeA = [&]
    {
        REQUIRE(!aInvoked);
        aInvoked = true;
    };
    auto invokeB = [&]
    {
        REQUIRE(!bInvoked);
        bInvoked = true;
    };

    SECTION("Test Case Test")
    {
        invokeA();
        REQUIRE(aInvoked);

        invokeB();
        REQUIRE(bInvoked);
    }

    SECTION("Tasks are cancelable")
    {
        scheduler.Schedule(Seconds(1), std::bind(invokeA));

        REQUIRE(!aInvoked);

        scheduler.CancelAll();

        scheduler.Update(Seconds(2));

        REQUIRE(!aInvoked);
    }

    SECTION("Tasks are cancelable through its group")
    {
        scheduler.Schedule(Seconds(1), GROUP_0, std::bind(invokeA));
        scheduler.Schedule(Seconds(2), Seconds(3), GROUP_1, std::bind(invokeB));

        REQUIRE(!aInvoked);
        REQUIRE(!bInvoked);

        scheduler.CancelGroup(GROUP_0);

        scheduler.Update(Seconds(10));

        REQUIRE(!aInvoked);
        REQUIRE(bInvoked);
    }

    SECTION("Tasks are cancelable through multiple groups")
    {
        scheduler.Schedule(Seconds(1), GROUP_0, std::bind(invokeA));
        scheduler.Schedule(Seconds(1), GROUP_1, std::bind(invokeB));

        REQUIRE(!aInvoked);
        REQUIRE(!bInvoked);

        scheduler.CancelGroupsOf({ GROUP_0, GROUP_1 });

        scheduler.Update(Seconds(2));

        REQUIRE(!aInvoked);
        REQUIRE(!bInvoked);
    }
}

TEST_CASE("TaskScheduler task delaying", "[TaskScheduler]" )
{
    TaskScheduler scheduler;

    bool aInvoked = false;
    bool bInvoked = false;
    auto invokeA = [&]
    {
        REQUIRE(!aInvoked);
        aInvoked = true;
    };
    auto invokeB = [&]
    {
        REQUIRE(!bInvoked);
        bInvoked = true;
    };

    SECTION("Test Case Test")
    {
        invokeA();
        REQUIRE(aInvoked);

        invokeB();
        REQUIRE(bInvoked);
    }

    SECTION("Tasks are delayable")
    {
        scheduler.Schedule(Seconds(2), std::bind(invokeA));

        scheduler.Update(Seconds(1));
        REQUIRE(!aInvoked);

        for (int i = 0; i < 5; ++i)
        {
            scheduler.DelayAll(Seconds(1));
            REQUIRE(!aInvoked);

            scheduler.Update(Seconds(1));
            REQUIRE(!aInvoked);
        }

        scheduler.Update(Seconds(1));

        REQUIRE(aInvoked);
    }

    SECTION("Tasks are delayable through groups")
    {
        scheduler.Schedule(Seconds(1), GROUP_0, std::bind(invokeA));
        scheduler.Schedule(Seconds(2), GROUP_1, std::bind(invokeB));

        // a = 1, b = 2
        scheduler.DelayGroup(GROUP_0, Seconds(1));
        REQUIRE(!aInvoked);
        REQUIRE(!bInvoked);

        // a = 2, b = 2
        scheduler.Update(Seconds(1));
        REQUIRE(!aInvoked);
        REQUIRE(!bInvoked);

        // a = 1, b = 1
        scheduler.DelayGroup(GROUP_1, Seconds(1));
        REQUIRE(!aInvoked);
        REQUIRE(!bInvoked);

        // a = 1, b = 2
        scheduler.DelayAll(Seconds(1));
        REQUIRE(!aInvoked);
        REQUIRE(!bInvoked);

        // a = 2, b = 3
        scheduler.Update(Seconds(2));
        REQUIRE(aInvoked);
        REQUIRE(!bInvoked);

        // a = x, b = 1
        scheduler.Update(Seconds(1));
        REQUIRE(aInvoked);
        REQUIRE(bInvoked);
    }
}

TEST_CASE("TaskScheduler task rescheduling", "[TaskScheduler]" )
{
    TaskScheduler scheduler;

    bool aInvoked = false;
    bool bInvoked = false;
    auto invokeA = [&]
    {
        REQUIRE(!aInvoked);
        aInvoked = true;
    };
    auto invokeB = [&]
    {
        REQUIRE(!bInvoked);
        bInvoked = true;
    };

    SECTION("Test Case Test")
    {
        invokeA();
        REQUIRE(aInvoked);

        invokeB();
        REQUIRE(bInvoked);
    }

    SECTION("All tasks are rescheduleable")
    {
        scheduler.Schedule(Seconds(2), std::bind(invokeA));
        scheduler.Schedule(Seconds(10), std::bind(invokeB));

        scheduler.Update(Seconds(1));

        REQUIRE(!aInvoked);
        REQUIRE(!bInvoked);

        scheduler.RescheduleAll(Seconds(2));

        scheduler.Update(Seconds(1));

        REQUIRE(!aInvoked);
        REQUIRE(!bInvoked);

        scheduler.Update(Seconds(1));

        REQUIRE(aInvoked);
        REQUIRE(bInvoked);
    }

    SECTION("Tasks are rescheduleable through groups")
    {
        scheduler.Schedule(Seconds(2), GROUP_0, std::bind(invokeA));
        scheduler.Schedule(Seconds(10), GROUP_1, std::bind(invokeB));

        scheduler.Update(Seconds(1));

        REQUIRE(!aInvoked);
        REQUIRE(!bInvoked);

        scheduler.RescheduleGroup(GROUP_0, Seconds(4));

        REQUIRE(!aInvoked);
        REQUIRE(!bInvoked);

        // a=4, b=9

        scheduler.Update(Seconds(1));

        REQUIRE(!aInvoked);
        REQUIRE(!bInvoked);

        // a=3, b=8

        scheduler.RescheduleGroup(GROUP_1, Seconds(4));

        REQUIRE(!aInvoked);
        REQUIRE(!bInvoked);

        // a=3, b=4

        scheduler.Update(Seconds(2));

        REQUIRE(!aInvoked);
        REQUIRE(!bInvoked);

        // a=1, b=2

        scheduler.Update(Seconds(1));

        REQUIRE(aInvoked);
        REQUIRE(!bInvoked);

        // a=x, b=1

        scheduler.Update(Seconds(1));

        REQUIRE(aInvoked);
        REQUIRE(bInvoked);
    }
}

TEST_CASE("TaskScheduler validator and success hook", "[TaskScheduler]" )
{
    bool valid = true;

    TaskScheduler scheduler([&]
    {
        return valid;
    });

    std::size_t invoked = 0;
    auto invoke = [&] { ++invoked; };

    REQUIRE(!invoked);

    SECTION("Tasks are validateable")
    {
        valid = false;

        scheduler.Schedule(Seconds(2), std::bind(invoke));

        REQUIRE(!invoked);

        scheduler.Update(Seconds(3));

        REQUIRE(!invoked);

        valid = true;

        scheduler.Update(Seconds(3));

        REQUIRE(invoked);
    }

    SECTION("On success hook is executed without any function")
    {
        bool success = false;

        scheduler.Update(Seconds(3), [&]
        {
            success = true;
        });

        REQUIRE(success);
    }

    SECTION("On success hook is executed with a function")
    {
        scheduler.Schedule(Seconds(2), std::bind(invoke));

        bool success = false;

        scheduler.Update(Seconds(3), [&]
        {
            REQUIRE(!success);
            success = true;
        });

        REQUIRE(success);
    }

    SECTION("On success hook is not executed when validator returns false")
    {
        valid = false;

        scheduler.Schedule(Seconds(2), std::bind(invoke));

        bool success = false;

        scheduler.Update(Seconds(3), [&]
        {
            success = true;
        });

        REQUIRE(!success);
    }
}

TEST_CASE("TaskContext and repeatable tasks", "[TaskContext]" )
{
    TaskScheduler scheduler;

    std::size_t invoked = 0;

    SECTION("Tasks are repeatable and beyond of time scheduling is supported.")
    {
        scheduler.Schedule(Seconds(1), [&](TaskContext context)
        {
            ++invoked;
            context.Repeat();
        });

        scheduler.Update(Seconds(10));

        REQUIRE(invoked == 10);
    }

    SECTION("Beyond of time scheduling with new tasks, also tests if new tasks are scheduleable from within the context")
    {
        std::function<void(TaskContext)> task;
        task = [&](TaskContext context)
        {
            ++invoked;
            context.Schedule(Seconds(1), task);
        };

        scheduler.Schedule(Seconds(1), task);

        scheduler.Update(Seconds(10));

        REQUIRE(invoked == 10);
    }

    SECTION("In context scheduling")
    {
        scheduler.Schedule(Seconds(1), [&](TaskContext context)
        {
            REQUIRE(invoked == 0);
            invoked = 1;

            context.Schedule(Seconds(2), [&](TaskContext)
            {
                REQUIRE(invoked == 1);
                invoked = 2;
            });
        });

        scheduler.Update(Seconds(2));
        REQUIRE(invoked == 1);

        scheduler.Update(Seconds(1));
        REQUIRE(invoked == 2);
    }

    SECTION("Repeat counter is working")
    {
        std::size_t counter = 0;

        scheduler.Schedule(Seconds(1), [&](TaskContext context)
        {
            REQUIRE(counter == context.GetRepeatCounter());

            invoked += context.GetRepeatCounter();

            counter++;
            context.Repeat();
        });

        scheduler.Update(Seconds(4));

        REQUIRE(invoked == (0 + 1 + 2 + 3));
    }

    SECTION("Crash safe context handling")
    {
        TaskContext leakedContext;

        {
            TaskScheduler invalidScheduler;

            invalidScheduler.Schedule(Seconds(1), [&](TaskContext context)
            {
                REQUIRE(!context.IsExpired());

                leakedContext = std::move(context);

                // TODO
                // REQUIRE(context.IsExpired());
            });

            invalidScheduler.Update(Seconds(2));

            REQUIRE(!leakedContext.IsExpired());

            // invalidScheduler is destroyed here and the context gets invalidated
        }

        REQUIRE(leakedContext.IsExpired());

        // Its also safe to schedule new tasks which is crash safe
        leakedContext.Async([]
        {
            int i = 0;
            ++i;
        });
    }
}
