# C++11 Task Scheduler
[![Build Status](https://travis-ci.org/Naios/TaskScheduler.svg?branch=master)](https://travis-ci.org/Naios/TaskScheduler) [![Coverity Scan Build Status](https://scan.coverity.com/projects/5998/badge.svg)](https://scan.coverity.com/projects/5998)

[![Preview](https://raw.githubusercontent.com/Naios/TaskScheduler/master/doc/preview/Preview.gif)](https://github.com/Naios/TaskScheduler/blob/master/doc/examples/preview.cpp)

> Use **std::chrono::duration** to schedule functional types (**std::function**, std::bind, lambdas and functors) in the near future. Easy and safe to use and **dependency free**. Offers the possibility to reschedule, **repeat**, manipulate and **cancel tasks**.

***

## Table of Contents

* **[Usage Instructions](#usage-instructions)**
  * **[Scheduling Tasks](#scheduling-tasks)**
  * **[Duration literals and typedefs](#duration-literals-and-typedefs)**
  * **[Updating the Scheduler](#updating-the-scheduler)**
  * **[Groups](#groups)**
  * **[Contexts](#contexts)**
* **[Coverage](#coverage)**
* **[Requirements](#requirements)**
* **[License](#licence)**

## Usage Instructions
### Scheduling tasks

Scheduling a task is very easy, just declare a `TaskScheduler` and use `TaskScheduler::Schedule`.

```c++
#include <iostream>
#include <chrono>
#include "TaskScheduler.hpp"

using namespace tsc;

TaskScheduler scheduler;

scheduler.Schedule(std::chrono::seconds(10), [](TaskContext context)
{
    std::cout << "This is executed once." << std::endl;
});
```

### Duration literals and typedefs

If your compiler and projects supports C++14 use **[std::duration_literals](http://en.cppreference.com/w/cpp/chrono/operator%22%22ms)** for better readability like:

```c++
using namespace std::literals;

scheduler.Schedule(10s, ...);
```

Using C++11 you can also define `std::chrono::duration` shorthand typedefs for easier usage:

```c++
typedef std::chrono::milliseconds Milliseconds;
typedef std::chrono::seconds Seconds;
typedef std::chrono::minutes Minutes;
typedef std::chrono::hours Hours;

scheduler.Schedule(Seconds(10), ...);
```

### Updating the Scheduler

In most cases you will use an application-/ gameloop which is called in a constant interval, otherwise you need to create a thread to do so:

```c++
// An update loop where diff is in milliseconds
void Update(unsigned int diff)
{
    scheduler.Update(std::chrono::milliseconds(diff));
}

// An update loop where diff is in seconds
void Update(std::chrono::seconds& diff)
{
    scheduler.Update(diff);
}

// An update loop where no diff time is given
void Update()
{
    scheduler.Update();
}

```

### Reapeat Tasks

```c++
// Schedule a simple function that is executed every 10s
scheduler
	.Schedule(std::chrono::seconds(10), [](TaskContext context)
	{
	    std::cout << "This is executed once." << std::endl;
	})
	// Lazy overloading, just concat everything together...
	.Schedule(std::chrono::seconds(10), [](TaskContext context)
	{
	    std::cout << "This is executed every 10s..." << std::endl;

	    // Repeat this event.
	    context.Repeat();
	});

// Update the scheduler to now
scheduler.Update();

// ...or add a difftime, useful if you are using a global update tick.
scheduler.Update(std::chrono::milliseconds(200));
```

### Groups
Since you can't compare std::functions to each other  you can organize scheduled functions in groups to manage it.
```c++
enum ScheduledGroups
{
    GROUP_FIRST, // Group 0 is also a group ,-)
    GROUP_SECOND,
};

scheduler.Schedule(std::chrono::seconds(5), GROUP_FIRST, [](TaskContext context)
{
    std::cout << "You won't see me..." << std::endl;
});

// Cancel the group GROUP_FIRST
scheduler.CancelGroup(GROUP_FIRST);

// ... or cancel all goups.
scheduler.CancelAll();
```

### Contexts
To access the TaskScheduler safely from within a scheduled function there is a TaskContext provided. **It's important that you never pass the TaskScheduler as lambda capture! Use the TaskContext instead.** The task context provides the ability to reschedule the executed function, cancel groups or schedule new tasks.

If you schedule new tasks from within a context the time is calculated from the context, it's possible that the function gets executed at the same update tick!

```c++
scheduler.Schedule(std::chrono::milliseconds(1500), [](TaskContext context)
{
    std::cout << "Message (1/3)" << std::endl;

    context
        .Schedule(std::chrono::milliseconds(800), [](TaskContext context)
        {
            std::cout << "Message (2/3)" << std::endl;
        })
        .Schedule(std::chrono::milliseconds(900), [](TaskContext context)
        {
            std::cout << "Message (3/3)" << std::endl;
            context->CancelAll();
        });
});

// Only 1 update is called and all 3 functions are executed.
scheduler.Update(std::chrono::seconds(60));
```

## Coverage
Automatic unit tests are automatically executed through Travis CI to check the code integrity.
See the [tests for details](https://github.com/Naios/TaskScheduler/blob/master/Test.cpp).

## Requirements
The TaskScheduler is **dependency free** and was tested with following **c++11 capable** compiler and confirmed to work:

* Visual Studio 2013 Update 4
* Visual Studio 2015
* GNU GCC 4.8+
* Clang 3.4+

## License
The TaskScheduler is licensed under the [Apache 2 License](https://raw.githubusercontent.com/Naios/TaskScheduler/master/LICENSE).
