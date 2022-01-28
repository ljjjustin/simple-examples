#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>

#define CHILDREN 4

void *alloc_shared_mem(int size) {
    void *ptr = mmap(0, size,
                    PROT_READ|PROT_WRITE,
                    MAP_ANON|MAP_SHARED, -1,0);
    if (ptr == MAP_FAILED) {
        return NULL;
    }
    return memset(ptr, 0, size);
}

int atomic_cmpset(int *var, int old, int set) {
    char rc;

    __asm__ volatile (
            " lock;            "
            " cmpxchgl  %3, %1;"
            " sete      %0;    "
            : "=a" (rc) : "m" (*var), "a" (old), "r" (set) : "cc", "memory"
    );

    return rc;
}

int shared_spinlock(int *lock) {
    for(;;) {
        if (atomic_cmpset(lock, 0, 1)) {
            /* gain lock */
            return 1;
        }
        /* pause */
	__asm__ volatile ("pause; pause; pause;" :::);
    }
}

void shared_unlock(int *lock) {
    *lock = 0;
}


void countabit(int *lock, int *count) {

    for (int i=0; i < 5; i++) {
        shared_spinlock(lock);
        *count += 1;
        printf("counter is: %d\n", *count);
        shared_unlock(lock);
    }
}

int main(int argc, char **argv)
{
    int *lock = alloc_shared_mem(sizeof(int));
    if (!lock) {
        perror("alloc shared memory failed\n");
        exit(-1);
    }
    int *count = alloc_shared_mem(sizeof(int));
    if (!count) {
        perror("alloc shared memory failed\n");
        exit(-1);
    }

    int i, children[CHILDREN];

    for (i = 0; i < CHILDREN; i++) {
        children[i] = 0;
        int pid=fork();
        if (pid < 0) {
            perror("create child process failed\n");
            continue;
        }

        if (pid > 0) { /* parent */
            children[i] = pid;
        } else { /* child */
            countabit(lock, count);
            exit(0);
        }
    }
    countabit(lock, count);

    /* wait for all children quit */
    for (i = 0; i < CHILDREN; i++) {
        int status=0;
        if (children[i] > 0) {
            waitpid(children[i], &status, 0);
        }
    }
    exit(0);
}
