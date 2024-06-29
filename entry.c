

int start(int argc, char **argv) {
	
	window.title = fixed_string("My epic game");
	window.width = 400;
	window.height = 400;
	window.x = 200;
	window.y = 200;
	
	seed_for_random = os_get_current_cycle_count();
	
	float64 last_time = os_get_current_time_in_seconds();
	while (!window.should_close) {
		reset_temporary_storage();
		context.allocator = temp;
		
		os_update(); 
		
		float64 now = os_get_current_time_in_seconds();
		float64 delta = now - last_time;
		last_time = now;
		
		if (is_key_just_released(KEY_ESCAPE)) {
			window.should_close = true;
		}
		
		if (is_key_just_released('E')) {
			print("Mouse pos: %f, %f\n", input_frame.mouse_x, input_frame.mouse_y);
		}
		
		for (u64 i = 0; i < input_frame.number_of_events; i++) {
			Input_Event e = input_frame.events[i];
			
			switch (e.kind) {
				case INPUT_EVENT_KEY:    print("Key event key code %d\n", e.key_code); break;
				case INPUT_EVENT_SCROLL: print("Scroll event, x: %.3f, y: %.3f\n", e.xscroll, e.yscroll);  break;
				case INPUT_EVENT_TEXT:   print("Text event, utf32: %d, ascii: %c\n", e.utf32, e.ascii); break;
			}
		}
		
		
		draw_rect_rotated(v2(-.25f, -.25f), v2(.5f, .5f), COLOR_RED, v2(.25, .25), (f32)now);
		
		Vector2 hover_position = v2_rotate_point_around_pivot(v2(-.5, -.5), v2(0, 0), -(f32)now);
		Vector2 local_pivot = v2(.125f, .125f);
		draw_rect(v2_sub(hover_position, local_pivot), v2(.25f, .25f), COLOR_GREEN);
		
		gfx_update();
	}

	return 0;
}

