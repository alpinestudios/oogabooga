
int entry(int argc, char **argv) {
	
	window.title = fixed_string("Minimal Game Example");
	window.width = 1280;
	window.height = 720;
	window.x = 200;
	window.y = 200;
	window.clear_color = v4(1, 1, 1, 1);

	while (!window.should_close) {
		reset_temporary_storage();
		
		os_update(); 
		
		float64 now = os_get_current_time_in_seconds();
		Matrix4 hammer_xform = m4_scalar(1.0);
		hammer_xform         = m4_rotate_z(hammer_xform, (f32)now);
		hammer_xform         = m4_translate(hammer_xform, v3(-.25f, -.25f, 0));
		draw_rect_xform(hammer_xform, v2(.5f, .5f), COLOR_RED);
		
		gfx_update();
	}

	return 0;
}