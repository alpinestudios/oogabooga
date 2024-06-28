

void test_allocator(bool do_log_heap) {
	// Basic allocation and free
    int* a = (int*)alloc(sizeof(int));
    int* b = (int*)alloc(sizeof(int));
    int* c = (int*)alloc(sizeof(int));
    
    *a = 69;
    *b = 420;
    *c = 1337;
    
    assert(*a == 69, "Test failed: Memory corrupted");
    assert(*b == 420, "Test failed: Memory corrupted");
    assert(*c == 1337, "Test failed: Memory corrupted");

    // Test growing memory
    os_grow_program_memory(1024 * 1024 * 1000);
    
    assert(*a == 69, "Test failed: Memory corrupted");
    assert(*b == 420, "Test failed: Memory corrupted");
    assert(*c == 1337, "Test failed: Memory corrupted");

    // Allocate and free large block
    void* large_block = alloc(1024 * 1024 * 100);
    dealloc(large_block);

    // Allocate multiple small blocks
    void* blocks[100];
    for (int i = 0; i < 100; ++i) {
        blocks[i] = alloc(128);
        assert(blocks[i] != NULL, "Failed to allocate small block");
    }
    
    for (int i = 0; i < 100; ++i) {
        dealloc(blocks[i]);
    }

    // Stress test with various sizes
    for (int i = 1; i <= 1000; ++i) {
        void* p = alloc(i * 64);
        assert(p != NULL, "Failed to allocate varying size block");
        dealloc(p);
    }

    // Free in reverse order
    for (int i = 0; i < 100; ++i) {
        blocks[i] = alloc(128);
        assert(blocks[i] != NULL, "Failed to allocate small block");
    }

    for (int i = 99; i >= 0; --i) {
        dealloc(blocks[i]);
    }

    // Test memory integrity with various allocation patterns
    int* nums[10];
    for (int i = 0; i < 10; ++i) {
        nums[i] = (int*)alloc(sizeof(int) * 10);
        for (int j = 0; j < 10; ++j) {
            nums[i][j] = i * 10 + j;
        }
    }

    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 10; ++j) {
            assert(nums[i][j] == i * 10 + j, "Memory corruption detected");
        }
         dealloc(nums[i]);
    }
    
    
    push_allocator(temp);
    
    int* foo = (int*)alloc(72);
    *foo = 1337;
    void* bar = alloc(69);
    void* baz = alloc(420);
    
    assert(*foo == 1337, "Temp memory corruptada");
    
    int* old_foo = foo;
    
    reset_temporary_storage();
    
    foo = (int*)alloc(72);
    
    assert(old_foo == foo, "Temp allocator goof");
    
    pop_allocator();
    
    // Repeated Allocation and Free
    for (int i = 0; i < 10000; ++i) {
        void* temp = alloc(128);
        assert(temp != NULL && "Repeated allocation failed");
        dealloc(temp);
    }

    // Mixed Size Allocations
    void* mixed_blocks[200];
    for (int i = 0; i < 200; ++i) {
        if (i % 2 == 0) {
            mixed_blocks[i] = alloc(128);
        } else {
            mixed_blocks[i] = alloc(1024 * 1024); // 1MB blocks
        }
        assert(mixed_blocks[i] != NULL && "Mixed size allocation failed");
    }

    for (int i = 0; i < 200; ++i) {
        if (i % 2 == 0) {
            dealloc(mixed_blocks[i]);
        }
    }

    for (int i = 0; i < 200; ++i) {
        if (i % 2 != 0) {
            dealloc(mixed_blocks[i]);
        }
    }

    // Fragmentation Stress Test
    for (int i = 0; i < 50; ++i) {
        blocks[i] = alloc(256);
        assert(blocks[i] != NULL && "Failed to allocate small block for fragmentation test");
    }

    for (int i = 0; i < 50; i += 2) {
        dealloc(blocks[i]);
    }

    for (int i = 50; i < 100; ++i) {
        blocks[i] = alloc(128);
        assert(blocks[i] != NULL && "Failed to allocate small block in fragmented heap");
    }

    for (int i = 50; i < 100; ++i) {
        dealloc(blocks[i]);
    }

    for (int i = 1; i < 50; i += 2) {
        dealloc(blocks[i]);
    }
    
    if (do_log_heap) log_heap();
}

void test_thread_proc1(Thread* t) {
	os_sleep(5);
	printf("Hello from thread %llu\n", t->id);
	os_sleep(5);
	printf("Hello from thread %llu\n", t->id);
	os_sleep(5);
	printf("Hello from thread %llu\n", t->id);
	os_sleep(5);
	printf("Hello from thread %llu\n", t->id);
	os_sleep(5);
	printf("Hello from thread %llu\n", t->id);
}
Mutex_Handle test_mutex;
void test_thread_proc2(Thread* t) {
	os_lock_mutex(test_mutex);
	printf("Thread %llu part 1\n", t->id);
	os_sleep(1);
	printf("Thread %llu part 2\n", t->id);
	os_sleep(1);
	printf("Thread %llu part 3\n", t->id);
	os_unlock_mutex(test_mutex);
}
void test_threads() {
	Thread* t = os_make_thread(test_thread_proc1);
	os_start_thread(t);
	os_sleep(10);
	printf("This should be printed in middle of thread execution\n");
	os_join_thread(t);
	printf("Thread is joined\n");
	
	Mutex_Handle m = os_make_mutex();
	os_lock_mutex(m);
	os_unlock_mutex(m);
	
	
	test_mutex = os_make_mutex();
	Thread *threads[100];
	for (int i = 0; i < 100; i++) {
		threads[i] = os_make_thread(test_thread_proc2);
		os_start_thread(threads[i]);
	}
	for (int i = 0; i < 100; i++) {
		os_join_thread(threads[i]);
	}
}

void test_allocator_threaded(Thread *t) {
    for (int i = 0; i < 1000; ++i) {
        void* temp = alloc(128);
        assert(temp != NULL && "Repeated allocation failed");
        dealloc(temp);
    }

    void* mixed_blocks[40];
    for (int i = 0; i < 40; ++i) {
        if (i % 2 == 0) {
            mixed_blocks[i] = alloc(128);
        } else {
            mixed_blocks[i] = alloc(1024 * 1024); // 1MB blocks
        }
        assert(mixed_blocks[i] != NULL && "Mixed size allocation failed");
    }

    for (int i = 0; i < 40; ++i) {
        if (i % 2 == 0) {
            dealloc(mixed_blocks[i]);
        }
    }

    for (int i = 0; i < 40; ++i) {
        if (i % 2 != 0) {
            dealloc(mixed_blocks[i]);
        }
    }
}

void oogabooga_run_tests() {
	printf("Testing allocator...\n");
	test_allocator(true);
	printf("OK!\n");
	
	printf("Testing threads...\n");
	test_threads();
	printf("OK!\n");

	printf("Thread bombing allocator...\n");
	Thread* threads[100];
	for (int i = 0; i < 100; i++) {
		threads[i] = os_make_thread(test_allocator_threaded);
		os_start_thread(threads[i]);
	}
	for (int i = 0; i < 100; i++) {
		os_join_thread(threads[i]);
	}
	printf("OK!\n");
}