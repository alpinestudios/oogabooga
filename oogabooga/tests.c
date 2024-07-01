
void log_heap() {
	os_spinlock_lock(heap_lock);
	printf("\nHEAP:\n");
	
	Heap_Block *block = heap_head;
	
	while (block != 0) {
		
		printf("\tBLOCK @ 0x%I64x, %llu bytes\n", (u64)block, block->size);
		
		Heap_Free_Node *node = block->free_head;

		u64 total_free = 0;
		
		while (node != 0) {
		
			printf("\t\tFREE NODE @ 0x%I64x, %llu bytes\n", (u64)node, node->size);
			
			total_free += node->size;
		
			node = node->next;
		}
		
		printf("\t TOTAL FREE: %llu\n\n", total_free);
		
		block = block->next;
	}
	os_spinlock_unlock(heap_lock);
}

void test_allocator(bool do_log_heap) {

	Allocator heap = get_heap_allocator();

	// Basic allocation and free
    int* a = (int*)alloc(heap, sizeof(int));
    int* b = (int*)alloc(heap, sizeof(int));
    int* c = (int*)alloc(heap, sizeof(int));
    
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
    void* large_block = alloc(heap, 1024 * 1024 * 100);
    dealloc(heap, large_block);

    // Allocate multiple small blocks
    void* blocks[100];
    for (int i = 0; i < 100; ++i) {
        blocks[i] = alloc(heap, 128);
        assert(blocks[i] != NULL, "Failed to allocate small block");
    }
    
    for (int i = 0; i < 100; ++i) {
        dealloc(heap, blocks[i]);
    }

    // Stress test with various sizes
    for (int i = 1; i <= 1000; ++i) {
        void* p = alloc(heap, i * 64);
        assert(p != NULL, "Failed to allocate varying size block");
        dealloc(heap, p);
    }

    // Free in reverse order
    for (int i = 0; i < 100; ++i) {
        blocks[i] = alloc(heap, 128);
        assert(blocks[i] != NULL, "Failed to allocate small block");
    }

    for (int i = 99; i >= 0; --i) {
        dealloc(heap, blocks[i]);
    }

    // Test memory integrity with various allocation patterns
    int* nums[10];
    for (int i = 0; i < 10; ++i) {
        nums[i] = (int*)alloc(heap, sizeof(int) * 10);
        for (int j = 0; j < 10; ++j) {
            nums[i][j] = i * 10 + j;
        }
    }

    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 10; ++j) {
            assert(nums[i][j] == i * 10 + j, "Memory corruption detected");
        }
         dealloc(heap, nums[i]);
    }
    
    
    reset_temporary_storage();
    
    int* foo = (int*)alloc(temp, 72);
    *foo = 1337;
    void* bar = alloc(temp, 69);
    (void)bar;
    void* baz = alloc(temp, 420);
    (void)baz;
    
    assert(*foo == 1337, "Temp memory corruptada");
    
    int* old_foo = foo;
    
    reset_temporary_storage();
    
    foo = (int*)alloc(temp, 72);
    
    assert(old_foo == foo, "Temp allocator goof");
    
    // Repeated Allocation and Free
    for (int i = 0; i < 10000; ++i) {
        void* temp = alloc(heap, 128);
        assert(temp != NULL && "Repeated allocation failed");
        dealloc(heap, temp);
    }

    // Mixed Size Allocations
    void* mixed_blocks[200];
    for (int i = 0; i < 200; ++i) {
        if (i % 2 == 0) {
            mixed_blocks[i] = alloc(heap, 128);
        } else {
            mixed_blocks[i] = alloc(heap, 1024 * 1024); // 1MB blocks
        }
        assert(mixed_blocks[i] != NULL && "Mixed size allocation failed");
    }

    for (int i = 0; i < 200; ++i) {
        if (i % 2 == 0) {
            dealloc(heap, mixed_blocks[i]);
        }
    }

    for (int i = 0; i < 200; ++i) {
        if (i % 2 != 0) {
            dealloc(heap, mixed_blocks[i]);
        }
    }

    // Fragmentation Stress Test
    for (int i = 0; i < 50; ++i) {
        blocks[i] = alloc(heap, 256);
        assert(blocks[i] != NULL && "Failed to allocate small block for fragmentation test");
    }

    for (int i = 0; i < 50; i += 2) {
        dealloc(heap, blocks[i]);
    }

    for (int i = 50; i < 100; ++i) {
        blocks[i] = alloc(heap, 128);
        assert(blocks[i] != NULL && "Failed to allocate small block in fragmented heap");
    }

    for (int i = 50; i < 100; ++i) {
        dealloc(heap, blocks[i]);
    }

    for (int i = 1; i < 50; i += 2) {
        dealloc(heap, blocks[i]);
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

void test_threads() {
	
	Thread* t = os_make_thread(test_thread_proc1, get_heap_allocator());
	os_start_thread(t);
	os_sleep(20);
	printf("This should be printed in middle of thread execution\n");
	os_join_thread(t);
	printf("Thread is joined\n");
	
	Mutex_Handle m = os_make_mutex();
	os_lock_mutex(m);
	os_unlock_mutex(m);
}

void test_allocator_threaded(Thread *t) {

	Allocator heap = get_heap_allocator();

    for (int i = 0; i < 1000; ++i) {
        void* temp = alloc(heap, 128);
        assert(temp != NULL && "Repeated allocation failed");
        dealloc(heap, temp);
    }

    void* mixed_blocks[40];
    for (int i = 0; i < 40; ++i) {
        if (i % 2 == 0) {
            mixed_blocks[i] = alloc(heap, 128);
        } else {
            mixed_blocks[i] = alloc(heap, 1024 * 1024); // 1MB blocks
        }
        assert(mixed_blocks[i] != NULL && "Mixed size allocation failed");
    }

    for (int i = 0; i < 40; ++i) {
        if (i % 2 == 0) {
            dealloc(heap, mixed_blocks[i]);
        }
    }

    for (int i = 0; i < 40; ++i) {
        if (i % 2 != 0) {
            dealloc(heap, mixed_blocks[i]);
        }
    }
}

void test_strings() {
	// Test length_of_null_terminated_string
    assert(length_of_null_terminated_string("Test") == 4, "Failed: length_of_null_terminated_string");
    assert(length_of_null_terminated_string("") == 0, "Failed: length_of_null_terminated_string");

	Allocator heap = get_heap_allocator();

    // Test alloc_string and dealloc_string
    string alloc_str = alloc_string(heap, 10);
    assert(alloc_str.data != NULL, "Failed: alloc_string");
    assert(alloc_str.count == 10, "Failed: alloc_string");
    dealloc_string(heap, alloc_str);

    // Test string_concat
    string str1 = fixed_string("Hello, ");
    string str2 = fixed_string("World!");
    string concat_str = string_concat(str1, str2, heap);
    assert(concat_str.count == str1.count + str2.count, "Failed: string_concat");
    assert(memcmp(concat_str.data, "Hello, World!", concat_str.count) == 0, "Failed: string_concat");
    dealloc_string(heap, concat_str);

    // Test convert_to_null_terminated_string
    char* cstr = convert_to_null_terminated_string(str1, heap);
    assert(strcmp(cstr, "Hello, ") == 0, "Failed: convert_to_null_terminated_string");
    dealloc(heap, cstr);

    // Test temp_convert_to_null_terminated_string
    cstr = temp_convert_to_null_terminated_string(str2);
    assert(strcmp(cstr, "World!") == 0, "Failed: temp_convert_to_null_terminated_string");

    // Test sprint
    string format_str = fixed_string("Number: %d");
    string formatted_str = sprint(heap, format_str, 42);
    char* formatted_cstr = convert_to_null_terminated_string(formatted_str, heap);
    assert(strcmp(formatted_cstr, "Number: 42") == 0, "Failed: sprint");
    dealloc(heap, formatted_str.data);
    dealloc(heap, formatted_cstr);

    // Test tprint
    string temp_formatted_str = tprint(format_str, 100);
    formatted_cstr = temp_convert_to_null_terminated_string(temp_formatted_str);
    assert(strcmp(formatted_cstr, "Number: 100") == 0, "Failed: tprint");

    // Test print and printf (visual inspection)
    printf("Expected output: Hello, World!\n");
    print("Hello, %s!\n", fixed_string("World"));

    printf("Expected output: Number: 1234\n");
    print(fixed_string("Number: %d\n"), 1234);
    
    printf("Expected output: Number: 1234\n");
    print(fixed_string("Number: %d\n"), 1234);

    printf("Expected output: Mixed values: 42 and 3.14\n");
    print(fixed_string("Mixed values: %d and %.2f\n"), 42, 3.14);

	// This should fail assert and print descriptive error
    //printf("Expected output (printf): Hello, World!\n");
    //printf("Hello, %cs!\n", 5);
	// This should fail assert and print descriptive error
    // printf("Expected output (printf): Hello, World!\n");
    // printf("Hello, %s!\n", "World");
    
    printf("Expected output (printf): Hello, World!\n");
    printf("Hello, %s!\n", cstr("World"));

    printf("Expected output (printf): Number: 5678\n");
    printf("Number: %d\n", 5678);

    printf("Expected output (printf): Mixed values: 99 and 2.71\n");
    printf("Mixed values: %d and %.2f\n", 99, 2.71);

    // Test handling of empty strings
    string empty_str = fixed_string("");
    string concat_empty_str = string_concat(empty_str, empty_str, heap);
    assert(concat_empty_str.count == 0, "Failed: string_concat with empty strings");
    dealloc_string(heap, concat_empty_str);

    // Test very large strings (performance test)
    string large_str1 = alloc_string(heap, 1024 * 1024);
    string large_str2 = alloc_string(heap, 1024 * 1024);
    string large_concat_str = string_concat(large_str1, large_str2, heap);
    assert(large_concat_str.count == 2 * 1024 * 1024, "Failed: large string_concat");
    dealloc_string(heap, large_str1);
    dealloc_string(heap, large_str2);
    dealloc_string(heap, large_concat_str);

    // Test string with special characters
    string special_char_str = fixed_string("Special chars: \n\t\r");
    cstr = convert_to_null_terminated_string(special_char_str, heap);
    assert(strcmp(cstr, "Special chars: \n\t\r") == 0, "Failed: special character string");
    dealloc(heap, cstr);
    
    string a = tprintf("Hello, %cs!\n", "balls");
    string balls1 = string_view(a, 7, 5);
    string balls2 = fixed_string("balls");
    
    assert(strings_match(balls1, balls2), "String match failed");
    assert(!strings_match(balls1, a), "String match failed");
}

void test_file_io() {

#if TARGET_OS == WINDOWS
    // Test win32_fixed_utf8_to_null_terminated_wide
    string utf8_str = fixed_string("Test");
    u16 *wide_str = win32_fixed_utf8_to_null_terminated_wide(utf8_str, get_heap_allocator());
    assert(wide_str != NULL, "Failed: win32_fixed_utf8_to_null_terminated_wide");
    assert(wide_str[4] == 0, "Failed: win32_fixed_utf8_to_null_terminated_wide");
    dealloc(get_heap_allocator(), wide_str);

    // Test temp_win32_fixed_utf8_to_null_terminated_wide
    wide_str = temp_win32_fixed_utf8_to_null_terminated_wide(utf8_str);
    assert(wide_str != NULL, "Failed: temp_win32_fixed_utf8_to_null_terminated_wide");
    assert(wide_str[4] == 0, "Failed: temp_win32_fixed_utf8_to_null_terminated_wide");
#endif

	File file = OS_INVALID_FILE;

    os_file_close(file);
    // Test os_file_open and os_file_close
    file = os_file_open(fixed_string("test.txt"), O_WRITE | O_CREATE);
    assert(file != OS_INVALID_FILE, "Failed: os_file_open (write/create)");
    os_file_close(file);
    

    // Test os_file_write_string and os_file_read
    string hello_world_write = fixed_string("Hello, World!");
    file = os_file_open(fixed_string("test.txt"), O_WRITE | O_CREATE);
    assert(file != OS_INVALID_FILE, "Failed: os_file_open (write/create)");
    bool write_result = os_file_write_string(file, hello_world_write);
    assert(write_result, "Failed: os_file_write_string");
    os_file_close(file);

    file = os_file_open(fixed_string("test.txt"), O_READ);
    assert(file != OS_INVALID_FILE, "Failed: os_file_open (read)");
    string hello_world_read = talloc_string(hello_world_write.count);
    bool read_result = os_file_read(file, hello_world_read.data, hello_world_read.count, &hello_world_read.count);
    assert(read_result, "Failed: os_file_read %d", GetLastError());
    assert(strings_match(hello_world_read, hello_world_write), "Failed: os_file_read write/read mismatch");
    os_file_close(file);

    // Test os_file_write_bytes
    file = os_file_open(fixed_string("test_bytes.txt"), O_WRITE | O_CREATE);
    assert(file != OS_INVALID_FILE, "Failed: os_file_open (write/create)");
    int int_data = 42;
    write_result = os_file_write_bytes(file, &int_data, sizeof(int));
    assert(write_result, "Failed: os_file_write_bytes");
    os_file_close(file);

    // Test os_read_entire_file and os_write_entire_file
    string write_data = fixed_string("Entire file test");
    bool write_entire_result = os_write_entire_file(fixed_string("entire_test.txt"), write_data);
    assert(write_entire_result, "Failed: os_write_entire_file");

	Allocator heap = get_heap_allocator();

    string read_data;
    bool read_entire_result = os_read_entire_file(fixed_string("entire_test.txt"), &read_data, heap);
    assert(read_entire_result, "Failed: os_read_entire_file");
    assert(strings_match(read_data, write_data), "Failed: os_read_entire_file write/read mismatch");
    assert(memcmp(read_data.data, write_data.data, write_data.count) == 0, "Failed: os_read_entire_file (content mismatch)");
    dealloc(heap, read_data.data);
    
    // Test fprint
    File balls = os_file_open(fixed_string("balls.txt"), O_WRITE | O_CREATE);
    assert(balls != OS_INVALID_FILE, "Failed: Could not create balls.txt");
	fprint(balls, "Hello, %cs!", "Balls");    
    os_file_close(balls);
    string hello_balls;
    read_entire_result = os_read_entire_file(fixed_string("balls.txt"), &hello_balls, heap);
    assert(read_entire_result, "Failed: could not read balls.txt");
    assert(strings_match(hello_balls, fixed_string("Hello, Balls!")), "Failed: balls read/write mismatch. Expected 'Hello, Balls!', got '%s'", hello_balls);
    
    u64 integers[4096];
    for (u64 i = 0; i < 4096; i++) {
    	integers[i] = get_random();
    }
    string integers_data;
    integers_data.data = (u8*)integers;
    integers_data.count = 4096*sizeof(u64);
    bool ok = os_write_entire_file(fxstr("integers"), integers_data);
    assert(ok, "write integers fail");
    
    string integers_read;
    ok = os_read_entire_file(fxstr("integers"), &integers_read, heap);
    assert(ok, "read integers fail");
    u64 *new_integers = (u64*)integers_data.data;
    assert(integers_read.count == integers_data.count, "Failed: big file read/write mismatch. Read was %d and written was %d", integers_read.count, integers_data.count);
    assert(strings_match(integers_data, integers_read), "Failed: big file read/write mismatch");

    // Clean up test files
    bool delete_ok = false;
    delete_ok = os_file_delete(fixed_string("test.txt"));
    assert(delete_ok, "Failed: could not delete test.txt");
    delete_ok = os_file_delete(fixed_string("test_bytes.txt"));
    assert(delete_ok, "Failed: could not delete test_bytes.txt");
    delete_ok = os_file_delete(fixed_string("entire_test.txt"));
    assert(delete_ok, "Failed: could not delete entire_test.txt");
    delete_ok = os_file_delete(fixed_string("balls.txt"));
    assert(delete_ok, "Failed: could not delete balls.txt");
    delete_ok = os_file_delete(fixed_string("integers.txt"));
    assert(delete_ok, "Failed: could not delete integers.txt");
}

void oogabooga_run_tests() {
	printf("Testing allocator... ");
	test_allocator(true);
	printf("OK!\n");
	
	printf("Testing threads... ");
	test_threads();
	printf("OK!\n");
	
	printf("Testing strings... ");
	test_strings();
	printf("OK!\n");

	printf("Thread bombing allocator... ");
	Thread* threads[100];
	for (int i = 0; i < 100; i++) {
		threads[i] = os_make_thread(test_allocator_threaded, get_heap_allocator());
		os_start_thread(threads[i]);
	}
	for (int i = 0; i < 100; i++) {
		os_join_thread(threads[i]);
	}
	printf("OK!\n");
	
	printf("Testing file IO... ");
	test_file_io();
	printf("OK!\n");
}