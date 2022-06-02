/*
 * Copyright (c) 1998-2007 Apple Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 *
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */

//#include <IOKit/system.h>

#include <IOKit/IOReturn.h>
#include <IOKit/IOKitLib.h>
#include <assert.h>
#include <string.h>

#include <IOKit/IOLocksPrivate.h>
#include <IOKit/IOLocks.h>

extern "C" {
#include <pthread.h>

#if defined(__x86_64__)
/* Synthetic event if none is specified, for backwards compatibility only. */
static bool IOLockSleep_NO_EVENT __attribute__((used)) = 0;
#endif

typedef struct _IOLock {
    pthread_mutex_t _lock;
    pthread_cond_t _cond;
} IOLock;

void
IOLockInitWithState( IOLock * lock, IOLockState state)
{
	if (state == kIOLockStateLocked) {
        IOLockLock(lock);
	}
}

IOLock *
IOLockAlloc( void )
{
    IOLock *lock = (IOLock *)malloc(sizeof(IOLock));
    memset(lock, 0x0, sizeof(*lock));
    return lock;
}

void
IOLockFree( IOLock * lock)
{
	free(lock);
}

void *
IOLockGetMachLock( IOLock * lock)
{
	return &lock->_lock;
}

void IOLockLock( IOLock * lock)
{
    pthread_mutex_lock(&lock->_lock);;
}

boolean_t IOLockTryLock( IOLock * lock)
{
    return pthread_mutex_trylock(&lock->_lock);
}

void IOLockUnlock( IOLock * lock)
{
    pthread_mutex_unlock(&lock->_lock);;
}

int
IOLockSleep( IOLock * lock, void *event, UInt32 interType __unused)
{
    return  pthread_cond_wait(&lock->_cond, &lock->_lock);
//    (lock, LCK_SLEEP_PROMOTED_PRI, (event_t) event, (wait_interrupt_t) interType);
}

int
IOLockSleepDeadline( IOLock * lock, void *event,
    AbsoluteTime deadline, UInt32 interType __unused)
{
    time_t *time = (time_t *) &deadline;
    struct timespec tm = { *time/1000000000 , *time };
    return pthread_cond_timedwait_relative_np(&lock->_cond, &lock->_lock, &tm);
//    (lock, LCK_SLEEP_PROMOTED_PRI, (event_t) event, (wait_interrupt_t) interType, __OSAbsoluteTime(deadline));
}

void
IOLockWakeup(IOLock * lock, void *event, bool oneThread)
{
//	thread_wakeup_prim((event_t) event, oneThread, THREAD_AWAKENED);
    pthread_cond_signal(&lock->_cond);
}


#if defined(__x86_64__)
/*
 * For backwards compatibility, kexts built against pre-Darwin 14 headers will bind at runtime to this function,
 * which supports a NULL event,
 */
int     IOLockSleep_legacy_x86_64( IOLock * lock, void *event, UInt32 interType) __asm("_IOLockSleep");
int     IOLockSleepDeadline_legacy_x86_64( IOLock * lock, void *event,
    AbsoluteTime deadline, UInt32 interType) __asm("_IOLockSleepDeadline");
void    IOLockWakeup_legacy_x86_64(IOLock * lock, void *event, bool oneThread) __asm("_IOLockWakeup");

int
IOLockSleep_legacy_x86_64( IOLock * lock, void *event, UInt32 interType)
{
	if (event == NULL) {
		event = (void *)&IOLockSleep_NO_EVENT;
	}

	return IOLockSleep(lock, event, interType);
}

int
IOLockSleepDeadline_legacy_x86_64( IOLock * lock, void *event,
    AbsoluteTime deadline, UInt32 interType)
{
	if (event == NULL) {
		event = (void *)&IOLockSleep_NO_EVENT;
	}

	return IOLockSleepDeadline(lock, event, deadline, interType);
}

void
IOLockWakeup_legacy_x86_64(IOLock * lock, void *event, bool oneThread)
{
	if (event == NULL) {
		event = (void *)&IOLockSleep_NO_EVENT;
	}

	IOLockWakeup(lock, event, oneThread);
}
#endif /* defined(__x86_64__) */


struct _IORecursiveLock {
	pthread_mutex_t  _lock;
    pthread_cond_t  _cond;
	lck_grp_t       *group;
	pthread_t        thread;
	UInt32          count;
};

IORecursiveLock *
IORecursiveLockAllocWithLockGroup( lck_grp_t * lockGroup )
{
	_IORecursiveLock * lock;

	if (lockGroup == 0) {
		return 0;
	}

	lock = IONew( _IORecursiveLock, 1 );
	if (!lock) {
		return 0;
	}

	pthread_mutex_init(&lock->_lock, nullptr);
	lock->group = lockGroup;
	lock->thread = 0;
	lock->count  = 0;

	return (IORecursiveLock *) lock;
}


IORecursiveLock *
IORecursiveLockAlloc( void )
{
	return IORecursiveLockAllocWithLockGroup( IOLockGroup );
}

void
IORecursiveLockFree( IORecursiveLock * _lock )
{
	_IORecursiveLock * lock = (_IORecursiveLock *)_lock;

	pthread_mutex_destroy(&lock->_lock);
	IODelete( lock, _IORecursiveLock, 1 );
}

pthread_mutex_t *
IORecursiveLockGetMachLock( IORecursiveLock * lock )
{
	return &lock->_lock;
}

void
IORecursiveLockLock( IORecursiveLock * _lock)
{
	_IORecursiveLock * lock = (_IORecursiveLock *)_lock;

	if (lock->thread == pthread_self()) {
		lock->count++;
	} else {
		pthread_mutex_lock( &lock->_lock );
		assert( lock->thread == 0 );
		assert( lock->count == 0 );
		lock->thread = pthread_self();
		lock->count = 1;
	}
}

boolean_t
IORecursiveLockTryLock( IORecursiveLock * _lock)
{
	_IORecursiveLock * lock = (_IORecursiveLock *)_lock;

	if (lock->thread == pthread_self()) {
		lock->count++;
		return true;
	} else {
		if (pthread_mutex_trylock( &lock->_lock )) {
			assert( lock->thread == 0 );
			assert( lock->count == 0 );
			lock->thread = pthread_self();
			lock->count = 1;
			return true;
		}
	}
	return false;
}

void
IORecursiveLockUnlock( IORecursiveLock * _lock)
{
	_IORecursiveLock * lock = (_IORecursiveLock *)_lock;

	assert( lock->thread == pthread_self());

	if (0 == (--lock->count)) {
		lock->thread = 0;
		pthread_mutex_unlock( &lock->_lock );
	}
}

boolean_t
IORecursiveLockHaveLock( const IORecursiveLock * _lock)
{
	_IORecursiveLock * lock = (_IORecursiveLock *)_lock;

	return lock->thread == pthread_self();
}

int
IORecursiveLockSleep(IORecursiveLock *_lock, void *event, UInt32 interType)
{
	_IORecursiveLock * lock = (_IORecursiveLock *)_lock;
	UInt32 count = lock->count;
	int res;

	assert(lock->thread == pthread_self());

	lock->count = 0;
	lock->thread = 0;
//	res = lck_mtx_sleep(&lock->_lock, LCK_SLEEP_PROMOTED_PRI, (event_t) event, (wait_interrupt_t) interType);
    
    res = pthread_cond_wait(&lock->_cond, &lock->_lock);
    
	// Must re-establish the recursive lock no matter why we woke up
	// otherwise we would potentially leave the return path corrupted.
	assert(lock->thread == 0);
	assert(lock->count == 0);
	lock->thread = pthread_self();
	lock->count = count;
	return res;
}

int
IORecursiveLockSleepDeadline( IORecursiveLock * _lock, void *event,
    AbsoluteTime deadline, UInt32 interType)
{
	_IORecursiveLock * lock = (_IORecursiveLock *)_lock;
	UInt32 count = lock->count;
	int res;

	assert(lock->thread == pthread_self());

	lock->count = 0;
	lock->thread = 0;
//	res = lck_mtx_sleep_deadline(&lock->_lock, LCK_SLEEP_PROMOTED_PRI, (event_t) event,
//	    (wait_interrupt_t) interType, __OSAbsoluteTime(deadline));

    time_t *time = (time_t *) &deadline;
    struct timespec tm = { *time/1000000000 , *time };
    return pthread_cond_timedwait_relative_np(&lock->_cond, &lock->_lock, &tm);
    
	// Must re-establish the recursive lock no matter why we woke up
	// otherwise we would potentially leave the return path corrupted.
	assert(lock->thread == 0);
	assert(lock->count == 0);
	lock->thread = pthread_self();
	lock->count = count;
	return res;
}

void
IORecursiveLockWakeup(IORecursiveLock *lock, void *event, bool oneThread)
{
//	thread_wakeup_prim((event_t) event, oneThread, THREAD_AWAKENED);
    pthread_cond_signal(&lock->_cond);
}

/*
 * Complex (read/write) lock operations
 */

struct _IORWLock  {
    pthread_rwlock_t _lock;
};


IORWLock *
IORWLockAlloc( void )
{
    IORWLock *lock = (IORWLock *)malloc(sizeof(IORWLock));
    memset(lock, 0x0, sizeof(*lock));
    return lock;
}

void
IORWLockFree( IORWLock * lock)
{
    pthread_rwlock_destroy(&lock->_lock);
}

pthread_rwlock_t *
IORWLockGetMachLock( IORWLock * lock)
{
	return &lock->_lock;
}


void IORWLockRead(IORWLock * lock)
{
    pthread_rwlock_rdlock(&lock->_lock);
}


void IORWLockWrite( IORWLock * lock)
{
    pthread_rwlock_wrlock(&lock->_lock);
}


void IORWLockUnlock( IORWLock * lock)
{
    pthread_rwlock_unlock(&lock->_lock);
}


/*
 * Spin locks
 */

typedef struct _IOSimpleLock {
    pthread_spinlock_t _lock;
} IOSimpleLock;

IOSimpleLock *
IOSimpleLockAlloc( void )
{
    IOSimpleLock *lock = (IOSimpleLock *)malloc(sizeof(IOSimpleLock));
    pthread_spinlock_init(&lock->_lock, 0);
    return lock;
//	return lck_spin_alloc_init( IOLockGroup, LCK_ATTR_NULL);
}

void
IOSimpleLockInit( IOSimpleLock * lock)
{
	pthread_spinlock_init(&lock->_lock, 0);
}

void
IOSimpleLockFree( IOSimpleLock * lock )
{
	pthread_spinlock_destroy(&lock->_lock);
    free(lock);
}

pthread_spinlock_t *
IOSimpleLockGetMachLock( IOSimpleLock * lock)
{
	return &lock->_lock;
}

void IOSimpleLockLock( IOSimpleLock * lock )
{
    pthread_spinlock_lock(&lock->_lock);
}

boolean_t IOSimpleLockTryLock( IOSimpleLock * lock )
{
    return pthread_spinlock_trylock(&lock->_lock);
}

void IOSimpleLockUnlock( IOSimpleLock * lock )
{
    pthread_spinlock_unlock(&lock->_lock);
}

#ifndef IOLOCKS_INLINE
/*
 * Lock assertions
 */

typedef enum {
    kIOLockAssertOwned,
    kIOLockAssertNotOwned
} IOLockAssertState;

typedef enum {
    kIORWLockAssertRead,
    kIORWLockAssertWrite,
    kIORWLockAssertHeld,
    kIORWLockAssertNotHeld,
} IORWLockAssertState;

void
IOLockAssert(IOLock * lock, IOLockAssertState type)
{
//	LCK_MTX_ASSERT(lock, type);
}

void
IORWLockAssert(IORWLock * lock, IORWLockAssertState type)
{
//	LCK_RW_ASSERT(lock, type);
}

typedef enum {
    kIOSimpleLockAssertOwned,
    kIOSimpleLockAssertNotOwned,
} IOSimpleLockAssertState;

void
IOSimpleLockAssert(IOSimpleLock *lock, IOSimpleLockAssertState type)
{
//	LCK_SPIN_ASSERT(l, type);
}
#endif /* !IOLOCKS_INLINE */
} /* extern "C" */
