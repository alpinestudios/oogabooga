
// BEWARE std140 packing:
// https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-packing-rules
typedef struct My_Cbuffer {
	Vector2 mouse_pos_screen;
	Vector2 window_size;
} My_Cbuffer;

int entry(int argc, char **argv) {
	
	window.title = STR("Custom Shader Example");
	window.scaled_width = 1280;
	window.scaled_height = 720; 
	window.x = 200;
	window.y = 90;
	window.clear_color = hex_to_rgba(0x6495EDff);

	string source;
	bool ok = os_read_entire_file("oogabooga/examples/custom_shader.hlsl", &source, get_heap_allocator());
	assert(ok, "Could not read oogabooga/examples/custom_shader.hlsl");
	
	// This is slow and needs to recompile the shader. However, it should probably only happen once (or each hot reload)
	// If it fails, it will return false and return to whatever shader it was before.
	shader_recompile_with_extension(source, sizeof(My_Cbuffer));
	
	dealloc_string(get_heap_allocator(), source);
	
	// This memory needs to stay alive throughout the frame because we pass the pointer to it in draw_frame.cbuffer.
	// If this memory is invalidated before gfx_update after setting draw_frame.cbuffer, then gfx_update will copy
	// memory from an invalid address.
	My_Cbuffer cbuffer;

	float64 last_time = os_get_current_time_in_seconds();
	while (!window.should_close) {
		
		float64 now = os_get_current_time_in_seconds();
		if ((int)now != (int)last_time) {
			log("%.2f FPS\n%.2fms", 1.0/(now-last_time), (now-last_time)*1000);
		}
		last_time = now;
	
		reset_temporary_storage();
		
		cbuffer.mouse_pos_screen = v2(input_frame.mouse_x, input_frame.mouse_y);
		cbuffer.window_size = v2(window.width, window.height);
		draw_frame.cbuffer = &cbuffer;
		
		// Just draw a big rect to cover background, so our lighting shader will apply to background
		draw_rect(v2(-5, -5), v2(10, 10), v4(.4, .4, .4, 1.0));
		
		Matrix4 rect_xform = m4_scalar(1.0);
		rect_xform         = m4_rotate_z(rect_xform, (f32)now);
		rect_xform         = m4_translate(rect_xform, v3(-.25f, -.25f, 0));
		draw_rect_xform(rect_xform, v2(.5f, .5f), COLOR_GREEN);
		
		draw_rect(v2(sin(now), -.8), v2(.5, .25), COLOR_RED);
		
	
		// Shader hot reloading
		if (is_key_just_pressed('R')) {
			ok = os_read_entire_file("oogabooga/examples/custom_shader.hlsl", &source, get_heap_allocator());
			assert(ok, "Could not read oogabooga/examples/custom_shader.hlsl");
			shader_recompile_with_extension(source, sizeof(My_Cbuffer));
			dealloc_string(get_heap_allocator(), source);
		}
		
		os_update(); 
		gfx_update();
	}

	return 0;
}