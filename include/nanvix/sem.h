#ifndef NANVIX_SEM_H_

#define NANVIX_SEM_H_
#include <nanvix/const.h>
#include <nanvix/config.h>

#define MAX_SEM 100
#define GETVAL 0
#define SETVAL 1
#define IPC_RMID 3

#endif
// On considère que la liste chainée d'un processus est utilisé uniquement
// pour les sémaphores et qu'aucune autre fonction kernel ne l'utilise.
typedef struct
{
    unsigned int valid; // 0 = false, 1 = true
    int counter;
    unsigned int key;
    struct process *waiting_processes;
} semaphore;


EXTERN semaphore semaphores[MAX_SEM];
EXTERN int sys_semget(unsigned key);
EXTERN int sys_semop(int semid, int op);
EXTERN int sys_semctl(int semid, int cmd, int val);
