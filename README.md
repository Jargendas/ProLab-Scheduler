#AVR-Scheduler by ProLab at Technische Universität Berlin

This is a scheduler for AVR-devices to perform simple asynchron tasks 
after a given amount of time or pause the program for a given amount of 
time.
It is written for ATMEGA16, but can, due to its extremely compact design, also be run 
on ATTINY-devices with small modifications.

##Features

-Very compact design (in mode 2 below 1kB)
-Time-resolution of about one millisecond
-Interrupt-triggered function execution
-Argument passing through void-pointer
-Function execution happens outside of the interrupt due to stack frame 
manipulation

##Usage

The scheduler uses Timer/Counter0 to schedule tasks, so the program must be 
configured to work at your cpu-clock. To do so, please adjust 
`PRESCALE_SELECT` and `COUNT_TO` so the overflow-interrupt triggers every ms.
To initialize the scheduler, you have to call `init_timer` on startup once. 

Afterwards, there are two basic configurations to use the scheduler:

####1. Allocate tasks dynamically

This configuration uses malloc and free to dynamically allocate memory for the 
tasks and therefore needs more program memory, but can manage as many 
tasks as the RAM allows.
This configuration is recommended if you use malloc anyways, you don't care 
about program size or you have an unknown number of tasks.
You can activate it by setting TASKLISTSIZE to 0.

####2. Allocate tasks in static array

This configuration allocates a pre-defined number of tasks as an array 
at initialization. The scheduler will afterwards only use the space 
in this array and does not have to use malloc and free. Thus, it needs a predictable amount of memory and a 
smaller amount of program memory.
If you use malloc anyways, you have to use mode 1 as the program will 
not compile otherwise.
You can activate this mode by setting a value to TASKLISTSIZE.

###Scheduling tasks

After you have called `init_timer` once, you just have to call 
`schedule_function`. The arguments are

1. A pointer to your function, should have return type void and one 
void* argument.
2. A void-pointer to your arguments. Make sure the arguments are still 
available when the function is called, e.g. use malloc or allocate them 
statically!
3. The time in ms in which the function should be executed.

###Pausing program execution

You can call `delay` with the amount of time to halt your program in ms 
as the argument. This will use the scheduler for timing, but block 
execution with a while-loop until the given amount of time passed. Be 
aware the time until execution can be shorter if it is close to 1ms. 
This is due to the fact that the tasks are triggered by 
timer-interrupts.
