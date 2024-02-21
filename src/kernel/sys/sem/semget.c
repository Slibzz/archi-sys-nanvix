#include <nanvix/const.h>
#include <nanvix/sem.h>

PUBLIC int sys_semget(unsigned key)
{
    
    for (int i = 0; i < MAX_SEM; i++)
    {
        if (semaphores[i].key == key)
            return i;
    }
    for (int i = 0; i < MAX_SEM; i++)
    {
        if (semaphores[i].valid)
        {
            semaphores[i].valid = 1;
            semaphores[i].counter = 0;
            semaphores[i].key = key;
            semaphores[i].waiting_processes = NULL;
            return i;
        }
    }
    return -1;
}