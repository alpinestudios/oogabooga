
typedef struct Circle {
    Vector2 pos;
    float radius;
} Circle;

int entry(int argc, char **argv) {
	
	window.title = STR("Minimal Game Example");
	window.point_width = 1280; // We need to set the scaled size if we want to handle system scaling (DPI)
	window.point_height = 720; 
	window.x = 200;
	window.y = 90;
	window.clear_color = hex_to_rgba(0x6495EDff);

    Circle *circles;
    growing_array_init((void**)&circles, sizeof(Circle), get_heap_allocator());
    
    const int num_circles = 10000;
    const float radius_min = 8.0;
    const float radius_max = 32.0;
    
    const float hover_radius = 200;
    
    os_update(); // We set scaled window size, os_update updates the pixel window size values for us
    
    for (int i = 0; i < num_circles; i++) {
    	float32 r = get_random_float32_in_range(radius_min, radius_max);
    	Vector2 p = v2(
    		get_random_float32_in_range(-(f32)window.width/2.0+r, (f32)window.width/2.0-r),
    		get_random_float32_in_range(-(f32)window.height/2.0+r, (f32)window.height/2.0-r)
    	);
    	// &(Circle){p, r} will only compile in true C, not in a C++ compiler
        growing_array_add((void**)&circles, &(Circle){p, r});
    }

	float64 last_time = os_get_elapsed_seconds();
	while (!window.should_close) {
		float64 now = os_get_elapsed_seconds();
		if ((int)now != (int)last_time) log("%.2f FPS\n%.2fms", 1.0/(now-last_time), (now-last_time)*1000);
		last_time = now;
		
		reset_temporary_storage();
		
		draw_frame.projection = m4_make_orthographic_projection(window.pixel_width * -0.5, window.pixel_width * 0.5, window.pixel_height * -0.5, window.pixel_height * 0.5, -1, 10);
		
		
        float mx = input_frame.mouse_x - window.width/2;
        float my = input_frame.mouse_y - window.height/2;
        
        // We build an array on the fly each frame for all the hovered circles
        Circle *circles_hovered;
        growing_array_init_reserve((void**)&circles_hovered, sizeof(Circle), num_circles, get_temporary_allocator());
		for (int i = 0; i < num_circles; i++) {
            float distance = v2_length(v2_sub(v2(mx, my), circles[i].pos));
            if (distance <= hover_radius) {
                growing_array_add((void**)&circles_hovered, &circles[i]);
            }
		}
        
        
	
		for (int i = 0; i < growing_array_get_valid_count(circles_hovered); i++) {
            Circle c = circles_hovered[i];
            draw_circle(v2_sub(c.pos, v2(c.radius, c.radius)), v2(c.radius, c.radius), COLOR_GREEN);
		}
		
		os_update(); 
		gfx_update();
	}

	return 0;
}