
// BEWARE std140 packing:
// https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-packing-rules
typedef struct My_Cbuffer {
	Vector2 mouse_pos_screen; // We use this to make a light around the mouse cursor
	Vector2 window_size; // We only use this to revert the Y in the shader because for some reason d3d11 inverts it.
} My_Cbuffer;


// We implement these details which we implement in the shader
#define DETAIL_TYPE_ROUNDED_CORNERS 1
#define DETAIL_TYPE_OUTLINED 2
#define DETAIL_TYPE_OUTLINED_CIRCLE 3

// With custom shading we can extend the rendering library!
Draw_Quad *draw_rounded_rect(Vector2 p, Vector2 size, Vector4 color, float radius);
Draw_Quad *draw_rounded_rect_xform(Matrix4 xform, Vector2 size, Vector4 color, float radius);
Draw_Quad *draw_outlined_rect(Vector2 p, Vector2 size, Vector4 color, float line_width_pixels);
Draw_Quad *draw_outlined_rect_xform(Matrix4 xform, Vector2 size, Vector4 color, float line_width_pixels);
Draw_Quad *draw_outlined_circle(Vector2 p, Vector2 size, Vector4 color, float line_width_pixels);
Draw_Quad *draw_outlined_circle_xform(Matrix4 xform, Vector2 size, Vector4 color, float line_width_pixels);

int entry(int argc, char **argv) {
	
	window.title = STR("Custom Shader Example");
	window.point_width = 1280;
	window.point_height = 720; 
	window.x = 200;
	window.y = 90;
	window.clear_color = hex_to_rgba(0x6495EDff);

	string source;
	bool ok = os_read_entire_file("oogabooga/examples/custom_shader.hlsl", &source, get_heap_allocator());
	assert(ok, "Could not read oogabooga/examples/custom_shader.hlsl");
	
	// This is slow and needs to recompile the shader. However, it should probably only happen once (or each hot reload)
	// If it fails, it will return false and return to whatever shader it was before.
	gfx_shader_recompile_with_extension(source, sizeof(My_Cbuffer));
	
	dealloc_string(get_heap_allocator(), source);
	
	// This memory needs to stay alive throughout the frame because we pass the pointer to it in draw_frame.cbuffer.
	// If this memory is invalidated before gfx_update after setting draw_frame.cbuffer, then gfx_update will copy
	// memory from an invalid address.
	My_Cbuffer cbuffer;

	float64 last_time = os_get_elapsed_seconds();
	while (!window.should_close) {
		
		float64 now = os_get_elapsed_seconds();
		if ((int)now != (int)last_time) {
			log("%.2f FPS\n%.2fms", 1.0/(now-last_time), (now-last_time)*1000);
		}
		last_time = now;
	
		reset_temporary_storage();
		
		float32 aspect = (float32)window.width/(float32)window.height;
	
		draw_frame.projection = m4_make_orthographic_projection(-aspect, aspect, -1, 1, -1, 10);
		
		cbuffer.mouse_pos_screen = v2(input_frame.mouse_x, input_frame.mouse_y);
		cbuffer.window_size = v2(window.width, window.height);
		draw_frame.cbuffer = &cbuffer;
		
		// Just draw a big rect to cover background, so our lighting shader will apply to background
		draw_rect(v2(-5, -5), v2(10, 10), v4(.4, .4, .4, 1.0));
		
		Matrix4 rect_xform = m4_scalar(1.0);
		rect_xform         = m4_rotate_z(rect_xform, (f32)now);
		rect_xform         = m4_translate(rect_xform, v3(-.25f, -.25f, 0));
		Draw_Quad *q = draw_rounded_rect_xform(rect_xform, v2(.5f, .5f), COLOR_GREEN, 0.1);
		
		draw_outlined_rect(v2(sin(now), -.8), v2(.5, .25), COLOR_RED, 2);
		
		draw_outlined_circle(v2(-sin(now), -.8), v2(.6, .6), COLOR_BLUE, 2);
		
	
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

Vector2 world_to_screen(Vector2 p) {
    Vector4 in_cam_space  = m4_transform(draw_frame.camera_xform, v4(p.x, p.y, 0.0, 1.0));
    Vector4 in_clip_space = m4_transform(draw_frame.projection, in_cam_space);
    
    Vector4 ndc = {
        .x = in_clip_space.x / in_clip_space.w,
        .y = in_clip_space.y / in_clip_space.w,
        .z = in_clip_space.z / in_clip_space.w,
        .w = in_clip_space.w
    };
    
    return v2(
        (ndc.x + 1.0f) * 0.5f * (f32)window.width,
        (ndc.y + 1.0f) * 0.5f * (f32)window.height
    );
}
Vector2 world_size_to_screen_size(Vector2 s) {
    Vector2 origin = v2(0, 0);
    
    Vector2 screen_origin = world_to_screen(origin);
    Vector2 screen_size_point = world_to_screen(s);
    
    return v2(
        screen_size_point.x - screen_origin.x,
        screen_size_point.y - screen_origin.y
    );
}

Draw_Quad *draw_rounded_rect(Vector2 p, Vector2 size, Vector4 color, float radius) {
	Draw_Quad *q = draw_rect(p, size, color);
	// detail_type
	q->userdata[0].x = DETAIL_TYPE_ROUNDED_CORNERS;
	// corner_radius
	q->userdata[0].y = radius;
	return q;
}
Draw_Quad *draw_rounded_rect_xform(Matrix4 xform, Vector2 size, Vector4 color, float radius) {
	Draw_Quad *q = draw_rect_xform(xform, size, color);
	// detail_type
	q->userdata[0].x = DETAIL_TYPE_ROUNDED_CORNERS;
	// corner_radius
	q->userdata[0].y = radius;
	return q;
}
Draw_Quad *draw_outlined_rect(Vector2 p, Vector2 size, Vector4 color, float line_width_pixels) {
	Draw_Quad *q = draw_rect(p, size, color);
	// detail_type
	q->userdata[0].x = DETAIL_TYPE_OUTLINED;
	// line_width_pixels
	q->userdata[0].y = line_width_pixels;
	// rect_size
	q->userdata[0].zw = world_size_to_screen_size(size);
	return q;
}
Draw_Quad *draw_outlined_rect_xform(Matrix4 xform, Vector2 size, Vector4 color, float line_width_pixels) {
	Draw_Quad *q = draw_rect_xform(xform, size, color);
	// detail_type
	q->userdata[0].x = DETAIL_TYPE_OUTLINED;
	// line_width_pixels
	q->userdata[0].y = line_width_pixels;
	// rect_size
	q->userdata[0].zw = world_size_to_screen_size(size);
	return q;
}
Draw_Quad *draw_outlined_circle(Vector2 p, Vector2 size, Vector4 color, float line_width_pixels) {
	Draw_Quad *q = draw_rect(p, size, color);
	// detail_type
	q->userdata[0].x = DETAIL_TYPE_OUTLINED_CIRCLE;
	// line_width_pixels
	q->userdata[0].y = line_width_pixels;
	// rect_size_pixels
	q->userdata[0].zw = world_size_to_screen_size(size); // Transform world space to screen space
	return q;
}
Draw_Quad *draw_outlined_circle_xform(Matrix4 xform, Vector2 size, Vector4 color, float line_width_pixels) {
	Draw_Quad *q = draw_rect_xform(xform, size, color);
	// detail_type
	q->userdata[0].x = DETAIL_TYPE_OUTLINED_CIRCLE;
	// line_width_pixels
	q->userdata[0].y = line_width_pixels;
	// rect_size_pixels
	q->userdata[0].zw = world_size_to_screen_size(size); // Transform world space to screen space
	
	return q;
}