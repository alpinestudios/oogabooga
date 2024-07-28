
typedef struct Spinlock Spinlock;
typedef struct Mutex Mutex;
typedef struct Binary_Semaphore Binary_Semaphore;

// These are probably your best friend for sync-free multi-processing.
inline bool compare_and_swap_8(volatile uint8_t *a, uint8_t b, uint8_t old);
inline bool compare_and_swap_16(volatile uint16_t *a, uint16_t b, uint16_t old);
inline bool compare_and_swap_32(volatile uint32_t *a, uint32_t b, uint32_t old);
inline bool compare_and_swap_64(volatile uint64_t *a, uint64_t b, uint64_t old);
inline bool compare_and_swap_bool(volatile bool *a, bool b, bool old);

///
// Spinlock "primitive"
// Like a mutex but it eats up the entire core while waiting.
// Beneficial if contention is low or sync speed is important
typedef struct Spinlock {
	volatile bool locked;
} Spinlock;

void ogb_instance
spinlock_init(Spinlock *l);

void ogb_instance
spinlock_acquire_or_wait(Spinlock* l);

// This returns true if successfully acquired or false if timeout reached.
bool ogb_instance
spinlock_acquire_or_wait_timeout(Spinlock* l, f64 timeout_seconds);

void ogb_instance
spinlock_release(Spinlock* l);


///
// High-level mutex primitive (short spinlock then OS mutex lock)
// Just spins for a few (configurable) microseconds with a spinlock,
// and if acquiring fails it falls back to a OS mutex.
#define MUTEX_DEFAULT_SPIN_TIME_MICROSECONDS 100
typedef struct Mutex {
	Spinlock spinlock;
	f64 spin_time_microseconds;
	Mutex_Handle os_handle;
	volatile bool spinlock_acquired;
	volatile u64 acquiring_thread;
} Mutex;

void ogb_instance
mutex_init(Mutex *m);

void ogb_instance
mutex_destroy(Mutex *m);

void ogb_instance
mutex_acquire_or_wait(Mutex *m);

void ogb_instance
mutex_release(Mutex *m);


///
// Binary semaphore
typedef struct Binary_Semaphore {
    volatile bool signaled;
    Mutex mutex;
} Binary_Semaphore;

void ogb_instance
binary_semaphore_init(Binary_Semaphore *sem, bool initial_state);

void ogb_instance
binary_semaphore_destroy(Binary_Semaphore *sem);

void ogb_instance
binary_semaphore_wait(Binary_Semaphore *sem);

void ogb_instance
binary_semaphore_signal(Binary_Semaphore *sem);


#if !OOGABOOGA_LINK_EXTERNAL_INSTANCE

void spinlock_init(Spinlock *l) {
	memset(l, 0, sizeof(*l));
}
void spinlock_acquire_or_wait(Spinlock* l) {
	while (true) {
        bool expected = false;
        if (compare_and_swap_bool(&l->locked, true, expected)) {
            return;
        }
        while (l->locked) {
            // spinny boi
        }
    }
}
// Returns true on aquired, false if timeout seconds reached
bool spinlock_acquire_or_wait_timeout(Spinlock* l, f64 timeout_seconds) {
    f64 start = os_get_current_time_in_seconds();
	while (true) {
        bool expected = false;
        if (compare_and_swap_bool(&l->locked, true, expected)) {
            return true;
        }
        while (l->locked) {
            // spinny boi
            if ((os_get_current_time_in_seconds()-start) >= timeout_seconds) return false;
        }
    }
    return true;
}
void spinlock_release(Spinlock* l) {
	bool expected = true;
    bool success = compare_and_swap_bool(&l->locked, false, expected);
    assert(success, "This thread should have acquired the spinlock but compare_and_swap failed");
}


///
// High-level mutex primitive (short spinlock then OS mutex lock)

void mutex_init(Mutex *m) {
	spinlock_init(&m->spinlock);
	m->spin_time_microseconds = MUTEX_DEFAULT_SPIN_TIME_MICROSECONDS;
	m->os_handle = os_make_mutex();
	m->spinlock_acquired = false;
	m->acquiring_thread = 0;
}
void mutex_destroy(Mutex *m) {
	os_destroy_mutex(m->os_handle);
}
void mutex_acquire_or_wait(Mutex *m) {
	if (spinlock_acquire_or_wait_timeout(&m->spinlock, m->spin_time_microseconds / 1000000.0)) {
        assert(!m->spinlock_acquired, "Internal sync error in Mutex");
    	m->spinlock_acquired = true;
    }
    os_lock_mutex(m->os_handle);
    
    assert(!m->acquiring_thread, "Internal sync error in Mutex: Multiple threads acquired");
    m->acquiring_thread = context.thread_id;
}
void mutex_release(Mutex *m) {
	assert(m->acquiring_thread != 0, "Tried to release a mutex which is not acquired");
	assert(m->acquiring_thread == context.thread_id, "Non-owning thread tried to release mutex");
	m->acquiring_thread = 0;
	bool was_spinlock_acquired = m->spinlock_acquired;
	m->spinlock_acquired = false;
	os_unlock_mutex(m->os_handle);
	if (was_spinlock_acquired) {
		spinlock_release(&m->spinlock);
	}
}



void binary_semaphore_init(Binary_Semaphore *sem, bool initial_state) {
    sem->signaled = initial_state;
    mutex_init(&sem->mutex);
}

void binary_semaphore_destroy(Binary_Semaphore *sem) {
    mutex_destroy(&sem->mutex);
}

void binary_semaphore_wait(Binary_Semaphore *sem) {
    mutex_acquire_or_wait(&sem->mutex);
    while (!sem->signaled) {
        mutex_release(&sem->mutex);
        os_yield_thread();
        mutex_acquire_or_wait(&sem->mutex);
    }
    sem->signaled = false;
    mutex_release(&sem->mutex);
}

void binary_semaphore_signal(Binary_Semaphore *sem) {
    mutex_acquire_or_wait(&sem->mutex);
    sem->signaled = true;
    mutex_release(&sem->mutex);
}

#endif