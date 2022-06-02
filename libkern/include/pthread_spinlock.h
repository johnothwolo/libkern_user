//
//  pthread_spin_lock.h
//  testobj
//
//  Created by John Othwolo on 6/1/22.
//  Copyright © 2022 oth. All rights reserved.
//

#ifndef pthread_spin_lock_h
#define pthread_spin_lock_h

#include <pthread.h>

typedef int pthread_spinlock_t;

#ifndef PTHREAD_PROCESS_SHARED
# define PTHREAD_PROCESS_SHARED 1
#endif
#ifndef PTHREAD_PROCESS_PRIVATE
# define PTHREAD_PROCESS_PRIVATE 2
#endif

int pthread_spinlock_init(pthread_spinlock_t *lock, int pshared);
int pthread_spinlock_destroy(pthread_spinlock_t *lock);
int pthread_spinlock_lock(pthread_spinlock_t *lock);
int pthread_spinlock_trylock(pthread_spinlock_t *lock);
int pthread_spinlock_unlock(pthread_spinlock_t *lock);

#endif /* pthread_spin_lock_h */
