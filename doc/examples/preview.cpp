
#include <iostream>

#include "TaskScheduler.hpp"

int main(int, char**)
{
    int diff = 0;

    using namespace std::literals;
    using tsc::TaskScheduler;
    using tsc::TaskContext;

    TaskScheduler scheduler;

    // In a time between 10s and 20s...
    scheduler.Schedule(10s, 20s, [](TaskContext context)
    {
        // ... schedule a periodic task which ticks every 5s...
        context.Schedule(5s, [](TaskContext context)
        {
            std::cout << "Tick" << std::endl;

            // ... but 5 times max!
            if (context.GetRepeatCounter() < 5)
                context.Repeat();
        });
    });

    // Update the scheduler with static intervals...
    scheduler.Update(15min);

    // ... or in your application loop.
    scheduler.Update(diff);
    return 0;
}
