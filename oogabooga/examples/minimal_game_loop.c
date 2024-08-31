
int entry(int argc, char **argv) {
	
	// This is how we (optionally) configure the window.
	// To see all the settable window properties, ctrl+f "struct Os_Window" in os_interface.c
	window.title = STR("Minimal Game Example");
	
	while (!window.should_close) {
		reset_temporary_storage();
		
		float64 now = os_get_elapsed_seconds();
		
		Matrix4 rect_xform = m4_scalar(1.0);
		rect_xform         = m4_rotate_z(rect_xform, (f32)now);
		rect_xform         = m4_translate(rect_xform, v3(-125, -125, 0));
		draw_rect_xform(rect_xform, v2(250, 250), COLOR_GREEN);
		
		draw_rect(v2(sin(now)*window.width*0.4-60, -60), v2(120, 120), COLOR_RED);
		
		os_update(); 
		gfx_update();
	}

	return 0;
}