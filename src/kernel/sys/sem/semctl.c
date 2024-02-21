#include <nanvix/const.h>
#include <nanvix/sem.h>

PUBLIC int sys_semctl(int semid, int cmd, int val)
{
    switch (cmd)
    {
    case GETVAL:
        return semaphores[semid].counter;
    case SETVAL:
        semaphores[semid].counter = val;
        return 0;
    case IPC_RMID:
        if (semaphores[semid].waiting_processes == NULL)
        {
            semaphores[semid].valid = 0;
            return 0;
        }
        else
            return -1;
    default:
        return -1;
    }
}