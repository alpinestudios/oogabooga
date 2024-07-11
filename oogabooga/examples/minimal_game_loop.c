
int entry(int argc, char **argv) {
	
	window.title = STR("Minimal Game Example");
	window.scaled_width = 1280; // We need to set the scaled size if we want to handle system scaling (DPI)
	window.scaled_height = 720; 
	window.x = 200;
	window.y = 90;
	window.clear_color = hex_to_rgba(0x6495EDff);

	while (!window.should_close) {
		reset_temporary_storage();
				
		float64 now = os_get_current_time_in_seconds();
		Matrix4 rect_xform = m4_scalar(1.0);
		rect_xform         = m4_rotate_z(rect_xform, (f32)now);
		rect_xform         = m4_translate(rect_xform, v3(-.25f, -.25f, 0));
		draw_rect_xform(rect_xform, v2(.5f, .5f), COLOR_GREEN);
		
		draw_rect(v2(sin(now), -.8), v2(.5, .25), COLOR_RED);
		
		os_update(); 
		gfx_update();
	}

	return 0;
}