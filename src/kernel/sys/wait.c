/*
 * 
 */

#include <nanvix/const.h>
#include <nanvix/mm.h>
#include <nanvix/pm.h>
#include <sys/types.h>
#include <errno.h>

/* Waiting chain. */
PRIVATE struct process *chain = NULL;

/*
 * Waits for a child process to terminate.
 */
PUBLIC pid_t sys_wait(int *stat_loc)
{
	struct process *p;
	
	/* Has no permissions to write at stat_loc. */
	if ((stat_loc != NULL) && (!chkmem(stat_loc, sizeof(int), 1)))
		return (-EINVAL);

repeat:
	/* Nobody to wait for. */
	if (curr_proc->nchildren == 0)
		return (-ECHILD);

	/* Look for child processes. */
	for (p = FIRST_PROC; p <= LAST_PROC; p++)
	{
		 /* Found. */
		if (p->father == curr_proc)
		{
			/* Task has already terminated. */
			if (p->state == PROC_ZOMBIE)
			{
				/* Get exit code. */
				if (stat_loc != NULL)
					*stat_loc = p->status;

				/* Kill child task. */
				bury(p);

				/* Return task pid. */
				return (p->pid);
			}
		}
	}
	
	sleep(&chain, PRIO_USER);
		
	if (issig() == SIGCHLD)
		goto repeat;
		
	return (-EINTR);
}