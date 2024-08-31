

int entry(int argc, char **argv) {
	
	window.title = STR("My epic game");
	window.point_width = 1280;
	window.point_height = 720;
	window.x = 200;
	window.y = 200;

	window.clear_color = hex_to_rgba(0x2a2d3aff);
	
	Gfx_Image *bush_image = load_image_from_disk(STR("oogabooga/examples/berry_bush.png"), get_heap_allocator());
	assert(bush_image, "Failed loading berry_bush.png");
	Gfx_Image *hammer_image = load_image_from_disk(STR("oogabooga/examples/hammer.png"), get_heap_allocator());
	assert(hammer_image, "Failed loading hammer.png");
	
	Custom_Mouse_Pointer hammer_pointer 
	   = os_make_custom_mouse_pointer_from_file(STR("oogabooga/examples/hammer.png"), 16, 16, get_heap_allocator());
	assert(hammer_pointer != 0, "Could not load hammer pointer");
	
	
	void *my_data = alloc(get_heap_allocator(), 32*32*4);
	memset(my_data, 0xffffffff, 32*32*4);
	Gfx_Image *my_image = make_image(32, 32, 4, my_data, get_heap_allocator());
	for (int *c = (int*)my_data; c < (int*)my_data+16*16; c += 1) {
		*c = 0xff0000ff;
	}
	gfx_set_image_data(my_image, 0, 0, 16, 16, my_data);
	
	Gfx_Font *font = load_font_from_disk(STR("C:/windows/fonts/arial.ttf"), get_heap_allocator());
	assert(font, "Failed loading arial.ttf, %d", GetLastError());
	
	// This makes sure atlas is rendered for ascii.
	// You might want to do this if your game lags the first time you render text because it renders
	// the atlas on the fly.
	render_atlas_if_not_yet_rendered(font, 32, 'A'); 
	
	seed_for_random = rdtsc();
	
	const float64 fps_limit = 69000;
	const float64 min_frametime = 1.0 / fps_limit;
	
	Matrix4 camera_xform = m4_scalar(1.0);
	
	float64 last_time = os_get_elapsed_seconds();
	while (!window.should_close) tm_scope("Frame") {
		reset_temporary_storage();
		
		float64 now = os_get_elapsed_seconds();
		float64 delta = now - last_time;
		if (delta < min_frametime) {
			os_high_precision_sleep((min_frametime-delta)*1000.0);
			now = os_get_elapsed_seconds();
			delta = now - last_time;
		}
		last_time = now;
		
		float32 aspect = (float32)window.width/(float32)window.height;
	
		draw_frame.projection = m4_make_orthographic_projection(-aspect, aspect, -1, 1, -1, 10);
		
		const float32 cam_move_speed = 4.0;
		Vector2 cam_move_axis = v2(0, 0);
		if (is_key_down('A')) {
			cam_move_axis.x -= 1.0;
		}
		if (is_key_down('D')) {
			cam_move_axis.x += 1.0;
		}
		if (is_key_down('S')) {
			cam_move_axis.y -= 1.0;
		}
		if (is_key_down('W')) {
			cam_move_axis.y += 1.0;
		}
		
		Vector2 cam_move = v2_mulf(cam_move_axis, delta * cam_move_speed);
		camera_xform = m4_translate(camera_xform, v3(v2_expand(cam_move), 0));
		draw_frame.camera_xform = camera_xform;

		seed_for_random = 69;
		for (u64 i = 0; i < 40000; i++) {
			float32 aspect = (float32)window.width/(float32)window.height;
			float min_x = -aspect;
			float max_x = aspect;
			float min_y = -1;
			float max_y = 1;
			
			float x = get_random_float32() * (max_x-min_x) + min_x;
			float y = get_random_float32() * (max_y-min_y) + min_y;

			draw_image(bush_image, v2(x, y), v2(0.1, 0.1), COLOR_WHITE);
		}
		
		if (is_key_just_released('E')) {
			log("FPS: %.2f", 1.0 / delta);
			log("ms: %.2f", delta*1000.0);
		}
		
		gfx_update();
		os_update();
	}

	return 0;
}

