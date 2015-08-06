# C++11 Task Scheduler
[![Build Status](https://travis-ci.org/Naios/TaskScheduler.svg?branch=master)](https://travis-ci.org/Naios/TaskScheduler)
![Preview](https://raw.githubusercontent.com/Naios/TaskScheduler/master/doc/preview/Preview.gif)


> Use **std::chrono::duration** to schedule functional types (**std::function**, std::bind, lambdas and functors) in the near future. Easy and safe to use and **dependency free**. Offers the possibility to reschedule, **repeat**, manipulate and **cancel tasks**.

***

## Table of Contents

* **[Usage Instructions](#usage-instructions)**
  * **[Basics](#Basics)**
  * **[Groups](#Groups)**
  * **[Contexts](#Contexts)**
* **[Requirements](#requirements)**
* **[License](#licence)**

## Usage Instructions

**TODO:** Instructions are not complete yet.

### Basics

```c++
#include <iostream>
#include "TaskScheduler.hpp"
using namespace tsc;

TaskScheduler scheduler;

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
To access the TaskScheduler safely from within a scheduled function there is a TaskContext provided. **Never pass the TaskScheduler as lambda capture!!!**, The task context provides the ability to reschedule the executed function, cancel groups or schedule new tasks.

If you schedule new tasks from within a context the timepoint is calculated from the context, its possible that the function gets executed at the same update tick!

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

## Requirements
The TaskScheduler is **dependency free** and was tested with following **c++11 capable** compiler and confirmed to work:

* Visual Studio 2013 Update 4
* **Visual Studio 2015**
* **GNU GCC 4.8+**
* **Clang 3.4+**

It's recommended to **enable C++14** if the project and compiler supports it to make use of [**std::duration_literals**](http://en.cppreference.com/w/cpp/chrono/operator%22%22ms) and improved lambda capture, however the TaskScheduler is still able to work with C++11.

## License
The TaskScheduler is licensed under the [Apache 2 License](https://raw.githubusercontent.com/Naios/TaskScheduler/master/LICENSE).
