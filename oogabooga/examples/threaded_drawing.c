
/*

	In this example we utilize separate draw frames and threading to split up the task of computing each
	quad.
	
	Note that the computed Draw_Frame's all need to be translated to vertices & copied to gpu on the main
	thread. 
	
	So what we do is that we split the total work (draw X sprites) up for a certain amount of thread, each
	which has it's own Draw_Frame. 

	We use Binary_Semaphore's per thread to notify 1. When draw thread can start drawing, after main thread
	has finished rendering the result draw_frames and 2. When draw thread is done, which the main thread needs
	to wait for before using the potentially unfinished result Draw_Frame's for rendering.
	
	If your computer has at lest 5-6 logical processors, that seems to split the time it takes to draw in
	about 1/3 (at least on my computer).
	
	Unfortunately, since the backend is using d3d11, the copying of vertices to gpu is very slow and can't
	really be mutlithreaded so that's really where the bottleneck is in this case. But offloading the Draw_Frame
	computations to separate threads definitely proved non-trivial.
	
*/

// Context per thread
typedef struct Draw_Context {
	Draw_Frame frame;
	u64 index;
	Gfx_Image *sprite;
	Binary_Semaphore draw_thread_start_sem;
	Binary_Semaphore draw_thread_done_sem;
	u64 number_of_sprites;
	Vector4 color;
	
	u64 frame_count;
	float64 accum_seconds;
} Draw_Context;

void draw_thread(Thread *t);

int entry(int argc, char **argv) {
	window.title = STR("Threaded Drawing Example");
	
	Gfx_Font *font = load_font_from_disk(STR("C:/windows/fonts/arial.ttf"), get_heap_allocator());
	
	Gfx_Image *sprite = load_image_from_disk(STR("oogabooga/examples/berry_bush.png"), get_heap_allocator());
	assert(sprite, "Could not load 'oogabooga/examples/berry_bush.png'");
	
	// This is overkill af on my computer with 32 logical processors, in fact 5-6 seems to peek in
	// performance and after that there's no difference. 
	// You could however imagine the threads doing a lot more work.
	u64 number_of_threads = os_get_number_of_logical_processors();
	
	u64 total_number_of_sprites = 150000;
		
	Thread *threads = (Thread*)alloc(get_heap_allocator(), number_of_threads*sizeof(Thread));
	Draw_Context *draw_contexts = (Draw_Context*)alloc(get_heap_allocator(), number_of_threads*sizeof(Draw_Context));
	
	// Initialize each thread and the respective draw context, and start the threads
	for (u64 i = 0; i < number_of_threads; i += 1) {
		Thread *t = threads + i;
		Draw_Context *draw_context = draw_contexts + i;
		
		os_thread_init(t, draw_thread);
		t->data = draw_context;
		
		draw_frame_init(&draw_context->frame);
		os_binary_semaphore_init(&draw_context->draw_thread_start_sem, false);
		os_binary_semaphore_init(&draw_context->draw_thread_done_sem, false);
		draw_context->index = i;
		draw_context->sprite = sprite;
		draw_context->number_of_sprites = total_number_of_sprites/number_of_threads;
		// Draw threads can start right away. Also if we don't do this, we will deadlock since draw thread will wait
		// for this signal, but we will wait for draw thread to signal being done.
		os_binary_semaphore_signal(&draw_context->draw_thread_start_sem);
		draw_context->color = v4(
			get_random_float32_in_range(0, 1),
			get_random_float32_in_range(0, 1),
			get_random_float32_in_range(0, 1),
			1
		);
		
		os_thread_start(t);
	}
	
	int tick = 0;
	
	float64 last_time = os_get_elapsed_seconds();
	while (!window.should_close) tm_scope("Update") {
		reset_temporary_storage();
		
		float64 now = os_get_elapsed_seconds();
		if ((int)now != (int)last_time) log("%.2f FPS\n%.2fms", 1.0/(now-last_time), (now-last_time)*1000);
		last_time = now;
		
		for (u64 i = 0; i < number_of_threads; i += 1) {
			Draw_Context *draw_context = draw_contexts + i;
			
			// Wait for draw thread to be done
			os_binary_semaphore_wait(&draw_context->draw_thread_done_sem);
			
			// Render the result Draw_Frame
			gfx_render_draw_frame_to_window(&draw_context->frame); 
			
			// Signal the draw thread that it can start drawing the next Draw_Frame
			os_binary_semaphore_signal(&draw_context->draw_thread_start_sem);
		}
		
		os_update(); 
		gfx_update();
		
	}

	return 0;
}

void draw_thread(Thread *t) {
	Draw_Context *draw_context = (Draw_Context*)t->data;
	
	u64 my_seed = rdtsc() + t->id;
	
	float32 sprite_width = 8;
	float32 sprite_height = 8;
	
	while (!window.should_close) tm_scope("Thread frame") {
		reset_temporary_storage();
		
		float64 now = os_get_elapsed_seconds();
		
		os_binary_semaphore_wait(&draw_context->draw_thread_start_sem);
		
		tm_scope("Thread draw") {
			draw_frame_reset(&draw_context->frame);
	
			// Remember, seed_for_random is thread_local
			seed_for_random = my_seed;
			
			for (u64 i = 0; i < draw_context->number_of_sprites; i += 1) {
				draw_image_in_frame(
					draw_context->sprite,
					v2(
						get_random_float32_in_range(-window.width/2, window.width/2) - sprite_width/2,
						get_random_float32_in_range(-window.height/2, window.height/2) - sprite_height/2
					),
					v2(sprite_width, sprite_height),
					draw_context->color,
					&draw_context->frame
				);
			}
			
			float64 duration = os_get_elapsed_seconds() - now;
			
			draw_context->accum_seconds += duration;
			draw_context->frame_count += 1;
		}
		
		os_binary_semaphore_signal(&draw_context->draw_thread_done_sem);
	}
}