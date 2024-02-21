#include <nanvix/const.h>
#include <nanvix/sem.h>
#include <nanvix/pm.h>

PUBLIC int sys_semop(int semid, int op)
{
    if (op == 0)
        return -1;
    if (!semaphores[semid].valid)
        return -1;

    disable_interrupts();

    if (op > 0)
    {
        semaphores[semid].counter++;
        wakeup(&semaphores[semid].waiting_processes);
    }
    else
    {
        semaphores[semid].counter--;

        if (semaphores[semid].counter < 0)
        {
            sleep(&semaphores[semid].waiting_processes, curr_proc->priority);
        }
    }

    enable_interrupts();

    return 0;
}