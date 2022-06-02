//
//  pthread_spin_lock.c
//  libkern
//
//  Created by John Othwolo on 6/1/22.
//  Copyright © 2022 oth. All rights reserved.
//

#include <stdio.h>
#include <errno.h>
#include <pthread_spinlock.h>

int pthread_spinlock_init(pthread_spinlock_t *lock, int pshared) {
    __asm__ __volatile__ ("" ::: "memory");
    *lock = 0;
    return 0;
}

int pthread_spinlock_destroy(pthread_spinlock_t *lock) {
    return 0;
}

int pthread_spinlock_lock(pthread_spinlock_t *lock) {
    while (1) {
        int i;
        for (i=0; i < 10000; i++) {
            if (__sync_bool_compare_and_swap(lock, 0, 1)) {
                return 0;
            }
        }
        sched_yield();
    }
}

int pthread_spinlock_trylock(pthread_spinlock_t *lock) {
    if (__sync_bool_compare_and_swap(lock, 0, 1)) {
        return 0;
    }
    return EBUSY;
}

int pthread_spinlock_unlock(pthread_spinlock_t *lock) {
    __asm__ __volatile__ ("" ::: "memory");
    *lock = 0;
    return 0;
}
