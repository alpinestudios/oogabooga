/*
	Screen-space bloom example.
	
	How it works:
	
	1. Draw game to an image (game_image)
	
	2. Draw game to another image, but with a shader that only lets through color values above 1.0 (bloom_map)
	
	3. Draw game_image to a third image (final_image), binding bloom_map to a shader which samples surrounding pixels from the bloom map to create the bloom effect.
	
	4. Draw final_image to the window
*/



Gfx_Shader_Extension load_shader(string file, int cbuffer_size);
void draw_game(Draw_Frame *frame);
bool button(string label, Vector2 pos, Vector2 size, bool enabled);

typedef enum View_Mode {
	VIEW_GAME_AFTER_POSTPROCESS,
	VIEW_GAME_BEFORE_POSTPROCESS,
	VIEW_BLOOM_MAP,
	
	VIEW_MODE_MAX
} View_Mode; 

string view_mode_stringify(View_Mode vm) {
	switch (vm) {
		case VIEW_GAME_AFTER_POSTPROCESS:
			return STR("VIEW_GAME_AFTER_POSTPROCESS");
		case VIEW_GAME_BEFORE_POSTPROCESS:
			return STR("VIEW_GAME_BEFORE_POSTPROCESS");
		case VIEW_BLOOM_MAP:
			return STR("VIEW_BLOOM_MAP");
		default: return STR("");
	}
}

// BEWARE std140 packing:
// https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-packing-rules
typedef struct Scene_Cbuffer {
	Vector2 mouse_pos_screen; // We use this to make a light around the mouse cursor
	Vector2 window_size; // We only use this to revert the Y in the shader because for some reason d3d11 inverts it.
} Scene_Cbuffer;

Gfx_Font *font;
u32 font_height = 28;
int entry(int argc, char **argv) {
	
	window.title = STR("Bloom example");
	
	font = load_font_from_disk(STR("C:/windows/fonts/arial.ttf"), get_heap_allocator());
	assert(font != 0, "Failed loading arial.ttf");
	
	// regular shader + point light which makes things extra bright
	Gfx_Shader_Extension light_shader = load_shader(STR("oogabooga/examples/bloom_light.hlsl"), sizeof(Scene_Cbuffer));
	// shader used to generate bloom map. Very simple: It takes the output color -1 on all channels 
	// so all we have left is how much bloom there should be
	Gfx_Shader_Extension bloom_map_shader = load_shader(STR("oogabooga/examples/bloom_map.hlsl"), sizeof(Scene_Cbuffer));
	// postprocess shader where the bloom happens. It samples from the generated bloom_map.
	Gfx_Shader_Extension postprocess_bloom_shader = load_shader(STR("oogabooga/examples/bloom.hlsl"), sizeof(Scene_Cbuffer));
	
	Gfx_Image *bloom_map = 0;
	Gfx_Image *game_image = 0;
	Gfx_Image *final_image = 0;
	
	View_Mode view = VIEW_GAME_AFTER_POSTPROCESS;
	
	Draw_Frame offscreen_draw_frame;
	draw_frame_init(&offscreen_draw_frame);
	
	Scene_Cbuffer scene_cbuffer;
	
	// Window width and height may be 0 before first call to os_update(), and we base render target sizes of window size.
	// This is an Oogabooga quirk which might get fixed at some point.
	os_update();
	
	while (!window.should_close) {
		reset_temporary_storage();
		
		
		///
		// Create bloom map and game image when window size changes (or first time)
		local_persist Os_Window last_window;
		if ((last_window.width != window.width || last_window.height != window.height || !game_image) && window.width > 0 && window.height > 0) {
			if (bloom_map)   delete_image(bloom_map);
			if (game_image)  delete_image(game_image);
			if (final_image) delete_image(final_image);
			
			bloom_map  = make_image_render_target(window.width, window.height, 4, 0, get_heap_allocator());
			game_image = make_image_render_target(window.width, window.height, 4, 0, get_heap_allocator());
			final_image = make_image_render_target(window.width, window.height, 4, 0, get_heap_allocator());
		}
		last_window = window;
		
		// Set stuff in cbuffer which we need to pass to shaders
		scene_cbuffer.mouse_pos_screen = v2(input_frame.mouse_x, window.height-input_frame.mouse_y);
		scene_cbuffer.window_size = v2(window.width, window.height);
		
		///
		// Draw game with light shader to game_image
		
		// Reset draw frame & clear the image with a clear color
		draw_frame_reset(&offscreen_draw_frame);
		gfx_clear_render_target(game_image, v4(.7, .7, .7, 1.0));
		
		// Draw game things to offscreen Draw_Frame
		draw_game(&offscreen_draw_frame);
		
		// Set the shader & cbuffer before the render call
		offscreen_draw_frame.shader_extension = light_shader;
		offscreen_draw_frame.cbuffer = &scene_cbuffer;
		
		// Render Draw_Frame to the image
		///// NOTE: Drawing to one frame like this will wait for the gpu to finish the last draw call. If this becomes
		// a performance bottleneck, you would have more frames "in flight" which you cycle through.
		gfx_render_draw_frame(&offscreen_draw_frame, game_image);
		
		
		
		///
		// Draw game with bloom map shader to the bloom map
		
		// Reset draw frame & clear the image
		draw_frame_reset(&offscreen_draw_frame);
		gfx_clear_render_target(bloom_map, COLOR_BLACK);

		
		// Draw game things to offscreen Draw_Frame
		draw_game(&offscreen_draw_frame);
		
		// Set the shader & cbuffer before the render call
		offscreen_draw_frame.shader_extension = bloom_map_shader;
		offscreen_draw_frame.cbuffer = &scene_cbuffer;
		
		// Render Draw_Frame to the image
		///// NOTE: Drawing to one frame like this will wait for the gpu to finish the last draw call. If this becomes
		// a performance bottleneck, you would have more frames "in flight" which you cycle through.
		gfx_render_draw_frame(&offscreen_draw_frame, bloom_map);
		
		///
		// Draw game image into final image, using the bloom shader which samples from the bloom_map
		
		draw_frame_reset(&offscreen_draw_frame);
		gfx_clear_render_target(final_image, COLOR_BLACK);
		
		// To sample from another image in the shader, we must bind it to a specific slot.
		draw_frame_bind_image_to_shader(&offscreen_draw_frame, bloom_map, 0);
		
		// Draw the game the final image, but now with the post process shader
		draw_image_in_frame(game_image, v2(-window.width/2, -window.height/2), v2(window.width, window.height), COLOR_WHITE, &offscreen_draw_frame);
		
		offscreen_draw_frame.shader_extension = postprocess_bloom_shader;
		offscreen_draw_frame.cbuffer = &scene_cbuffer;
		
		gfx_render_draw_frame(&offscreen_draw_frame, final_image);
		
		
		switch (view) {
			case VIEW_GAME_AFTER_POSTPROCESS:
				Draw_Quad *q = draw_image(final_image, v2(-window.width/2, -window.height/2), v2(window.width, window.height), COLOR_WHITE);
				// The draw image will be flipped on y, so we want to draw it "upside down"
				swap(q->uv.y, q->uv.w, float);
				break;
			case VIEW_GAME_BEFORE_POSTPROCESS:
				draw_image(game_image, v2(-window.width/2, -window.height/2), v2(window.width, window.height), COLOR_WHITE);
				break;
			case VIEW_BLOOM_MAP:
				draw_image(bloom_map, v2(-window.width/2, -window.height/2), v2(window.width, window.height), COLOR_WHITE);
				break;
			default: break;
		}
		
		for (int i = 0; i < VIEW_MODE_MAX; i += 1) {
			if (button(view_mode_stringify(i), v2(-window.width/2+40, window.height/2-100-i*60), v2(500, 50), i == view)) {
				view = i;
			}
		}
		
		os_update(); 
		gfx_update();
	}

	return 0;
}

Gfx_Shader_Extension load_shader(string file_path, int cbuffer_size) {
	string source;
	
	bool ok = os_read_entire_file(file_path, &source, get_heap_allocator());
	assert(ok, "Could not read %s", file_path);
	
	Gfx_Shader_Extension shader;
	ok = gfx_compile_shader_extension(source, cbuffer_size, &shader);
	assert(ok, "Failed compiling shader extension");
	
	return shader;
}

void draw_game(Draw_Frame *frame) {
	// Draw a background
	draw_rect_in_frame(v2(-window.width/2, -window.height/2), v2(window.width, window.height), v4(.2, .2, .2, 1), frame);

	// Draw some random things, with same seed each time so it looks like persistent things
	seed_for_random = 69;
	for (int i = 0; i < 1000; i += 1) {
	
		// Bias towards dark colors, so the bright bloom will stand out more
		bool dark = get_random_float32_in_range(0, 1) < 0.9;
		
		int bright_channels = 0;
		
		if (!dark) {
			if (get_random_int_in_range(0, 2))  bright_channels |= (1 << 1);
			if (get_random_int_in_range(0, 2))  bright_channels |= (1 << 2);
			if (get_random_int_in_range(0, 2))  bright_channels |= (1 << 3);
		}
	
		draw_rect_in_frame(v2( // Random pos
			get_random_float32_in_range(-window.width/2, window.width/2),
			get_random_float32_in_range(-window.height/2, window.height/2)
		), v2( // Random size
			get_random_float32_in_range(40, 100),
			get_random_float32_in_range(40, 100)
		), v4( // Random color
			(bright_channels & (1 << 1)) ? get_random_float32_in_range(0.95, 1.0) : get_random_float32_in_range(0.0, 0.3),
			(bright_channels & (1 << 2)) ? get_random_float32_in_range(0.95, 1.0) : get_random_float32_in_range(0.0, 0.3),
			(bright_channels & (1 << 3)) ? get_random_float32_in_range(0.95, 1.0) : get_random_float32_in_range(0.0, 0.3),
			1
		), frame);
		
	}
}

bool button(string label, Vector2 pos, Vector2 size, bool enabled) {

	Vector4 color = v4(.45, .45, .45, 1);
	
	float L = pos.x;
	float R = L + size.x;
	float B = pos.y;
	float T = B + size.y;
	
	float mx = input_frame.mouse_x - window.width/2;
	float my = input_frame.mouse_y - window.height/2;

	bool pressed = false;

	if (mx >= L && mx < R && my >= B && my < T) {
		color = v4(.15, .15, .15, 1);
		if (is_key_down(MOUSE_BUTTON_LEFT)) {
			color = v4(.05, .05, .05, 1);
		}
		
		pressed = is_key_just_released(MOUSE_BUTTON_LEFT);
	}
	
	if (enabled) {
		color = v4_sub(color, v4(.2, .2, .2, 0));
	}

	draw_rect(pos, size, color);
	
	Gfx_Text_Metrics m = measure_text(font, label, font_height, v2(1, 1));
	
	Vector2 bottom_left = v2_sub(pos, m.functional_pos_min);
	bottom_left.x += size.x/2;
	bottom_left.x -= m.functional_size.x/2;
	
	bottom_left.y += size.y/2;
	bottom_left.y -= m.functional_size.y/2;
	
	draw_text(font, label, font_height, bottom_left, v2(1, 1), COLOR_WHITE);
	
	return pressed;
}