

int entry(int argc, char **argv) {
	
	window.title = STR("OGB Text Rendering Example");
	window.scaled_width = 1280;
	window.scaled_height = 720;
	window.x = 200;
	window.y = 200;
	window.clear_color = hex_to_rgba(0x6495EDff);
	
	Gfx_Font *font = load_font_from_disk(STR("C:/windows/fonts/arial.ttf"), get_heap_allocator());
	assert(font, "Failed loading arial.ttf");
	
	const u32 font_height = 48;
	
	while (!window.should_close) tm_scope("Frame") {
		reset_temporary_storage();
		
		// Text is easiest to deal with if our projection matches window pixel size, because
		// then the rasterization height will match the screen pixel height (unless scaling).
		// The best way to make the text look good is to draw it at the exact same pixel height
		// as it was rasterized at with no down- or up-scaling.
		// It's fairly common in video games to render the UI with a separate projection for this
		// very reason.
		draw_frame.projection = m4_make_orthographic_projection(window.pixel_width * -0.5, window.pixel_width * 0.5, window.pixel_height * -0.5, window.pixel_height * 0.5, -1, 10);
		
		// Easy drop shadow: Just draw the same text underneath with a slight offset
		draw_text(font, STR("I am text"), font_height, v2(-2, 2), v2(1, 1), COLOR_BLACK);
		draw_text(font, STR("I am text"), font_height, v2(0, 0),  v2(1, 1), COLOR_WHITE);
		
		float now = (float)os_get_current_time_in_seconds();
		float animated_x = sin(now*0.1)*(window.width*0.5);
		
		// UTF-8 !
		draw_text(font, STR("Привет"), font_height, v2(animated_x-2, 2), v2(1, 1), COLOR_BLACK);
		draw_text(font, STR("Привет"), font_height, v2(animated_x, 0),  v2(1, 1), COLOR_WHITE);
		
		// New lines are handled when drawing text
		string hello_str = STR("Hello,\nTTTT New line\nAnother line");
		
		// To align/justify text we need to measure it.
		Gfx_Text_Metrics hello_metrics = measure_text(font, hello_str, font_height, v2(1, 1));
		
		// This is where we want the bottom left of the text to be...
		Vector2 bottom_left = v2(-window.width/2+20, -window.height/2+20);
		
		// ... So we have to justify that bottom_left according to text metrics
		Vector2 justified = v2_sub(bottom_left, hello_metrics.functional_pos_min);
		
		// If we wanted to center it:
		// justified = v2_sub(justified, v2_divf(hello_metrics.functional_size, 2));
		
		draw_text(font, hello_str, font_height, justified,  v2(1, 1), COLOR_WHITE);
		
		
		local_persist bool show_bounds = false;
		if (is_key_just_pressed('E')) show_bounds = !show_bounds;
		
		string long_text = STR("Jaunty jackrabbits juggle quaint TTT quilts and quirky quinces, \nquickly queuing up for a jubilant, jazzy jamboree in the jungle.\nLorem ipsilum ");
		if (show_bounds) {
			// Visualize the bounds we get from metrics
			Gfx_Text_Metrics m = measure_text(font, long_text, font_height, v2(1, 1));
			draw_rect(v2_add(v2(-600, -200), m.visual_pos_min), m.visual_size, v4(.1, .1, .1, .2));
			draw_rect(v2_add(v2(-600, -200), m.functional_pos_min), m.functional_size, v4(1, .1, .1, .2));		
		}
		draw_text(font, long_text, font_height, v2(-600, -200),  v2(1, 1), COLOR_WHITE);
		
		os_update();
		gfx_update();
		
	}
	
	return 0;
}