
int entry(int argc, char **argv) {
	
	// This is how we (optionally) configure the window.
	// To see all the settable window properties, ctrl+f "struct Os_Window" in os_interface.c
	window.title = STR("Minimal Game Example");
	
	float64 last_time = os_get_elapsed_seconds();
	while (!window.should_close) {
		reset_temporary_storage();
		
		float64 now = os_get_elapsed_seconds();
		if ((int)now != (int)last_time) log("%.2f FPS\n%.2fms", 1.0/(now-last_time), (now-last_time)*1000);
		last_time = now;
		
		Matrix4 rect_xform = m4_scalar(1.0);
		rect_xform         = m4_rotate_z(rect_xform, (f32)now);
		rect_xform         = m4_translate(rect_xform, v3(-.25f, -.25f, 0));
		draw_rect_xform(rect_xform, v2(.5f, .5f), COLOR_GREEN);
		
		draw_rect(v2(sin(now), -.8), v2(.5, .25), COLOR_RED);
		
		if (is_key_just_pressed('F')) {
			window.fullscreen = !window.fullscreen;
		}
		
		os_update(); 
		gfx_update();
	}

	return 0;
}