/*
    AVR-Scheduler created in ProLab
    Copyright (C) 2016 Jonathan Schostak, Sven Rotter, Andreas Liu, Patrick Artz, Karsten Eckhoff, Talha Kösker, Geronimo Bergk, Stephan Bauroth
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
    jonathan@schostak.org
*/

#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include "Scheduler.h"

/* TASKLISTSIZE 0 allocates memory dynamically with malloc (bigger binary) */
/* TASKLISTSIZE >0 uses an array to save tasks (rescricted number of tasks) */
#define TASKLISTSIZE 10

/* Must result in interrupt every ms - trimmed to 1MHz here */
#define PRESCALE_SELECT 0x02
#define COUNT_TO	124

/*////////////////////////////////////////////////////////////*/

#if TASKLISTSIZE == 0
	#include <stdlib.h>
#endif

typedef struct task
{
	void		(*functionPtr)(void*);
	void		*argPtr;
	uint16_t	timeUntilExec;
	struct task	*next;
} task;

#if TASKLISTSIZE > 0
	task timer_list_start[TASKLISTSIZE+1];
#else
	task *timer_list_start;
#endif

volatile uint8_t finished = 0;

#if TASKLISTSIZE > 0
	void* malloc ( uint8_t bytes_ignored )
	{
		for ( uint8_t i = 1; i <= TASKLISTSIZE; i++ )
		{
			if(timer_list_start[i].functionPtr == 0x00)
				return &timer_list_start[i];
		}

		return 0;
	}

	void free ( void* address )
	{
		task* the_task = (task*) address;
		the_task -> functionPtr = 0;
	}
#endif

void init_timer ( void )
{
	/* Setup so Timer0 Compare Interrupt will trigger every ms */
	OCR0 = COUNT_TO;
	/* Config Timer0: CTC-Mode, Prescale on defined value */
	TCCR0 = (1 << WGM01) | (PRESCALE_SELECT << CS00);
	TCCR0 &= ~ ((0x07 & ~PRESCALE_SELECT) << CS00);
	/* Activate Compare Interrupt */
	TIMSK |= (1 << OCIE0);

	timer_list_start -> next=0;

	sei();
}

void delay_finished ( void )
{
	finished = 1;
}

void delay( uint16_t ms )
{
	finished = 0;
	schedule_function((void*)delay_finished, 0, ms);
	while(!finished);
}

uint8_t schedule_function ( void (*functionPtr)(void*), void *args, uint16_t ms )
{
	/* search for place to insert */

	task *task_before = timer_list_start;
	uint16_t time	  = 0;

	while ( ( task_before -> next != 0 ) && (( (task_before -> next) -> timeUntilExec ) + time <= ms ) )
	{

		task_before 	= task_before -> next;
		time 		+= task_before -> timeUntilExec;

	}

	task *task_after = task_before -> next;

	/* create new task */
	task *task_new = malloc(sizeof(task));
	if(task_new == 0)
		return 0;

	task_new -> functionPtr		= functionPtr;
	task_new -> argPtr		= args;
	task_new -> timeUntilExec	= ms - time;
	task_new -> next		= task_after;

	/* correct time of next task */
	if ( task_after != 0 )
		task_after -> timeUntilExec -= task_new -> timeUntilExec;

	task_before -> next = task_new;

	return 1;
}

void return_from_timer_int ( void )
{
	/* restore registers */
	__asm__("pop r31");
	__asm__("pop r30");
	__asm__("pop r29");
	__asm__("pop r28");
	__asm__("pop r27");
	__asm__("pop r26");
	__asm__("pop r25");
	__asm__("pop r24");
	__asm__("pop r23");
	__asm__("pop r22");
	__asm__("pop r21");
	__asm__("pop r20");
	__asm__("pop r19");
	__asm__("pop r18");
	__asm__("pop r0");
	__asm__("out 0x3f, r0");
	__asm__("pop r0");
	__asm__("pop r1");
	__asm__("sei");
}

void next_timer_func ( void )
{
	__asm__( "pop r25" );
	__asm__( "pop r24" );
}

ISR ( TIMER0_COMP_vect, ISR_NAKED )
{

	/* save registers */
	__asm__("push r1");
	__asm__("push r0");
	__asm__("in r0, 0x3f");
	__asm__("push r0");
	__asm__("eor r1, r1");
	__asm__("push r18");
	__asm__("push r19");
	__asm__("push r20");
	__asm__("push r21");
	__asm__("push r22");
	__asm__("push r23");
	__asm__("push r24");
	__asm__("push r25");
	__asm__("push r26");
	__asm__("push r27");
	__asm__("push r28");
	__asm__("push r29");
	__asm__("push r30");
	__asm__("push r31");

	/* push interrupt return function */
	void (*end_func)(void) = return_from_timer_int;
	__asm__( "push %A0" : : "e" (end_func) );
	__asm__( "push %B0" : : "e" (end_func) );

	task *check_task = timer_list_start -> next;

	if ( check_task != 0 )
	check_task -> timeUntilExec--;

	while ( ( check_task != 0 ) && ( check_task -> timeUntilExec == 0 ) )
	{

		void (*function)(void*) = check_task -> functionPtr;
		void *arg = check_task -> argPtr;

		/* push function pointer */
		__asm__( "push %A0" : : "e" (function) );
		__asm__( "push %B0" : : "e" (function) );

		/* push arg pointer */
		__asm__( "push %A0" : : "e" (arg) );
		__asm__( "push %B0" : : "e" (arg) );

		/* push transition function pointer */
		__asm__( "push %A0" : : "e" (next_timer_func) );
		__asm__( "push %B0" : : "e" (next_timer_func) );

		/* free element */
		timer_list_start -> next = check_task -> next;
		free(check_task);
		check_task = timer_list_start -> next;

	}

	__asm__("reti");
}
