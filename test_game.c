
Vector4 hex_to_rgba(s64 hex) {
	u8 r = (hex>>24) & 0x000000FF;
	u8 g = (hex>>16) & 0x000000FF;
	u8 b = (hex>>8) & 0x000000FF;
	u8 a = (hex>>0) & 0x000000FF;
	return (Vector4){r/255.0, g/255.0, b/255.0, a/255.0};
}


int start(int argc, char **argv) {
	
	window.title = fixed_string("My epic game");
	window.width = 1280;
	window.height = 720;
	window.x = 200;
	window.y = 200;

	window.clear_color = hex_to_rgba(0x2a2d3aff);
	
	Gfx_Image *bush_image = load_image_from_disk(fixed_string("berry_bush.png"), get_heap_allocator());
	assert(bush_image, "Failed loading berry_bush.png");
	Gfx_Image *hammer_image = load_image_from_disk(fixed_string("hammer.png"), get_heap_allocator());
	assert(hammer_image, "Failed loading hammer.png");
	
	seed_for_random = os_get_current_cycle_count();
	
	const float64 fps_limit = 69000;
	const float64 min_frametime = 1.0 / fps_limit;
	
	Matrix4 camera_view = m4_scalar(1.0);
	
	float64 last_time = os_get_current_time_in_seconds();
	while (!window.should_close) {
		reset_temporary_storage();
		
		float64 now = os_get_current_time_in_seconds();
		float64 delta = now - last_time;
		if (delta < min_frametime) {
			os_high_precision_sleep((min_frametime-delta)*1000.0);
			now = os_get_current_time_in_seconds();
			delta = now - last_time;
		}
		last_time = now;
		os_update(); 
		
		if (is_key_just_released(KEY_ESCAPE)) {
			window.should_close = true;
		}
		
		if (is_key_just_pressed(KEY_ARROW_LEFT)) {
			window.width += 10;
			window.x -= 10;
		}
		if (is_key_just_pressed(KEY_ARROW_RIGHT)) {
			window.width += 10;
		}
		
		if (is_key_just_released('Q')) {
			delete_image(bush_image);
			bush_image = load_image_from_disk(fixed_string("berry_bush.png"), get_heap_allocator());
			assert(bush_image, "Failed loading berry_bush.png");
		}
		
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
		
		camera_view = m4_translate(camera_view, v3(v2_expand(cam_move), 0));
		
		draw_frame.view = camera_view;
		
		Matrix4 hammer_xform = m4_scalar(1.0);
		hammer_xform         = m4_rotate_z(hammer_xform, (f32)now);
		hammer_xform         = m4_translate(hammer_xform, v3(-.25f, -.25f, 0));
		draw_image_xform(hammer_image, hammer_xform, v2(.5f, .5f), COLOR_RED);
		
		Vector2 hover_position = v2_rotate_point_around_pivot(v2(-.5, -.5), v2(0, 0), (f32)now);
		Vector2 local_pivot = v2(.125f, .125f);
		draw_rect(v2_sub(hover_position, local_pivot), v2(.25f, .25f), v4((sin(now)+1.0)/2.0, 1.0, 0.0, 1.0));
		
		draw_image(bush_image, v2(0.65, 0.65), v2(0.2*sin(now), 0.2*sin(now)), COLOR_WHITE);
		
		gfx_update();
		
		if (is_key_just_released('E')) {
			log("FPS: %.2f", 1.0 / delta);
			log("ms: %.2f", delta*1000.0);
		}
	}

	return 0;
}

