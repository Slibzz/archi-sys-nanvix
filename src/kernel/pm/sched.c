/*
 * Copyright(C) 2011-2016 Pedro H. Penna   <pedrohenriquepenna@gmail.com>
 *              2015-2016 Davidson Francis <davidsondfgl@hotmail.com>
 *
 * This file is part of Nanvix.
 *
 * Nanvix is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Nanvix is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Nanvix. If not, see <http://www.gnu.org/licenses/>.
 */

#include <nanvix/clock.h>
#include <nanvix/const.h>
#include <nanvix/hal.h>
#include <nanvix/pm.h>
#include <signal.h>
#include <nanvix/klib.h>

PRIVATE inline struct process *default_yield(void);
PRIVATE inline struct process *round_robin(void);
PRIVATE inline struct process *random(void);
PRIVATE inline struct process *lottery(void);
PRIVATE inline struct process *priority(void);

/**
 * @brief Schedules a process to execution.
 *
 * @param proc Process to be scheduled.
 */
PUBLIC void sched(struct process *proc)
{
	proc->state = PROC_READY;
	proc->counter = 0;
}

/**
 * @brief Stops the current running process.
 */
PUBLIC void stop(void)
{
	curr_proc->state = PROC_STOPPED;
	sndsig(curr_proc->father, SIGCHLD);
	yield();
}

/**
 * @brief Resumes a process.
 *
 * @param proc Process to be resumed.
 *
 * @note The process must stopped to be resumed.
 */
PUBLIC void resume(struct process *proc)
{
	/* Resume only if process has stopped. */
	if (proc->state == PROC_STOPPED)
		sched(proc);
}

/**
 * @brief Yields the processor.
 */
PUBLIC void yield(void)
{
	struct process *p;	  /* Working process.     */
	struct process *next; /* Next process to run. */

	/* Re-schedule process for execution. */
	if (curr_proc->state == PROC_RUNNING)
		sched(curr_proc);

	/* Remember this process. */
	last_proc = curr_proc;

	/* Check alarm. */
	for (p = FIRST_PROC; p <= LAST_PROC; p++)
	{
		/* Skip invalid processes. */
		if (!IS_VALID(p))
			continue;

		/* Alarm has expired. */
		if ((p->alarm) && (p->alarm < ticks))
			p->alarm = 0, sndsig(p, SIGALRM);
	}
	next = lottery();

	/* Switch to next process. */
	next->priority = PRIO_USER;
	next->state = PROC_RUNNING;
	next->counter = PROC_QUANTUM;
	if (curr_proc != next)
		switch_to(next);
}

/*Test PASSED*/
PRIVATE inline struct process *default_yield(void)
{
	struct process *p;	  /* Working process.     */
	struct process *next; /* Next process to run. */
	/* Choose a process to run next. */
	next = IDLE;
	for (p = FIRST_PROC; p <= LAST_PROC; p++)
	{
		/* Skip non-ready process. */
		if (p->state != PROC_READY)
			continue;
		/*
		 * Process with higher
		 * waiting time found.
		 */
		if (p->counter > next->counter)
		{
			next->counter++;
			next = p;
		}

		/*
		 * Increment waiting
		 * time of process.
		 */
		else
			p->counter++;
	}
	return next;
}

/*Test PASSED*/
PRIVATE inline struct process *round_robin(void)
{
	struct process *p;	  /* Working process.     */
	struct process *next; /* Next process to run. */
	next = IDLE;
	for (p = curr_proc + 1; p <= LAST_PROC; p++)
	{
		if (p->state == PROC_READY)
		{
			next = p;
			break;
		}
	}
	if (next == IDLE)
	{
		for (p = FIRST_PROC; p < curr_proc; p++)
		{
			if (p->state == PROC_READY)
			{
				next = p;
				break;
			}
		}
	}
	for (p = FIRST_PROC; p < next; p++)
	{
		if (p->state == PROC_READY)
		{
			p->counter++;
		}
	}
	for (p = next + 1; p <= LAST_PROC; p++)
	{
		if (p->state == PROC_READY)
		{
			p->counter++;
		}
	}
	return next;
}

/*Test PASSED*/
PRIVATE inline struct process *random(void)
{
	struct process *next = IDLE;
	struct process *p;

	int nprocsReady = 0;
	for (p = FIRST_PROC; p <= LAST_PROC; p++)
	{
		if (p->state == PROC_READY)
		{
			p->counter++;
			nprocsReady++;
		}
	}

	if (nprocsReady == 0)
	{
		next = IDLE;
	}
	else
	{
		int random = (krand() % (nprocsReady)) + 1;
		int i = 0;
		for (p = FIRST_PROC; p <= LAST_PROC; p++)
		{
			if (p->state == PROC_READY)
			{
				i++;
				if (i == random)
				{
					next = p;
					break;
				}
			}
		}
	}

	return next;
}

/*
-100 = 8
...
0 = 3
20 = 2
40 = 1
*/
PRIVATE inline int weight(struct process *p)
{
	return (-p->priority + 60) / 20;
}

/*Test PASSED*/
PRIVATE inline struct process *lottery(void)
{
	struct process *p;	  /* Working process.     */
	struct process *next; /* Next process to run. */
	int tickets = 0;
	for (p = FIRST_PROC; p <= LAST_PROC; p++)
	{
		if (p->state == PROC_READY)
		{
			tickets += weight(p);
			p->counter++;
		}
	}
	if (tickets == 0)
	{
		next = IDLE;
	}
	else
	{
		int random = (krand() % tickets) + 1;
		int i = 0;
		for (p = FIRST_PROC; p <= LAST_PROC; p++)
		{
			if (p->state == PROC_READY)
			{
				i += weight(p);
				if (i >= random)
				{
					next = p;
					break;
				}
			}
		}
	}
	return next;
}

/*Test PASSED*/
PRIVATE inline struct process *priority(void)
{
	struct process *p;	  /* Working process.     */
	struct process *next; /* Next process to run. */
	/* Choose a process to run next. */
	next = IDLE;
	for (p = FIRST_PROC; p <= LAST_PROC; p++)
	{
		/* Skip non-ready process. */
		if (p->state != PROC_READY)
			continue;

		/*
		 * Process with higher
		 * waiting time found.
		 */
		if (p->priority < next->priority)
		{
			next->counter++;
			next = p;
		}
		else if (p->priority == next->priority)
		{
			if (p->utime + p->ktime <= next->utime + next->ktime)
			{
				next->counter++;
				next = p;
			}
			else
			{
				p->counter++;
			}
		}

		/*
		 * Increment waiting
		 * time of process.
		 */
		else
			p->counter++;
	}
	return next;
}