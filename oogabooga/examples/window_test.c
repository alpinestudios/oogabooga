
bool button(string label, Vector2 pos, Vector2 size, bool enabled);

Gfx_Font *font;
u32 font_height = 28;

int entry(int argc, char **argv) {

	font = load_font_from_disk(STR("C:/windows/fonts/arial.ttf"), get_heap_allocator());
	
	window.title = STR("Minimal Game Example");
	window.point_width = 900;
	window.point_height = 600;
	window.point_x = 100;
	window.point_y = 150;
	
	float64 last_time = os_get_elapsed_seconds();
	while (!window.should_close) {
		reset_temporary_storage();
		
		float64 now = os_get_elapsed_seconds();
		float64 delta_time = now - last_time;
		last_time = now;
		
		Matrix4 rect_xform = m4_scalar(1.0);
		rect_xform         = m4_rotate_z(rect_xform, (f32)now);
		rect_xform         = m4_translate(rect_xform, v3(-125, -125, 0));
		draw_rect_xform(rect_xform, v2(250, 250), COLOR_GREEN);
		
		draw_rect(v2(sin(now)*window.width*0.4-60, -60), v2(120, 120), COLOR_RED);
		
		float32 x = -window.width/2 + 40;
		float32 y =  window.height/2 - 40 - font_height;
		
		draw_text(font, tprint("FPS: %.2f", 1.0/delta_time), font_height, v2(x, y), v2(1, 1), v4(.9, .9, .9, 1.0));
		y -= font_height*1.2;
		draw_text(font, tprint("px width: %d", window.pixel_width), font_height, v2(x, y), v2(1, 1), v4(.9, .9, .9, 1.0));
		y -= font_height*1.2;
		draw_text(font, tprint("px height: %d", window.pixel_height), font_height, v2(x, y), v2(1, 1), v4(.9, .9, .9, 1.0));
		y -= font_height*1.2;
		draw_text(font, tprint("px x: %d", window.x), font_height, v2(x, y), v2(1, 1), v4(.9, .9, .9, 1.0));
		y -= font_height*1.2;
		draw_text(font, tprint("px y: %d", window.y), font_height, v2(x, y), v2(1, 1), v4(.9, .9, .9, 1.0));
		y -= font_height*1.2;
		draw_text(font, tprint("pt width: %d", window.point_width), font_height, v2(x, y), v2(1, 1), v4(.9, .9, .9, 1.0));
		y -= font_height*1.2;
		draw_text(font, tprint("pt height: %d", window.point_height), font_height, v2(x, y), v2(1, 1), v4(.9, .9, .9, 1.0));
		y -= font_height*1.2;
		draw_text(font, tprint("pt x: %d", window.point_x), font_height, v2(x, y), v2(1, 1), v4(.9, .9, .9, 1.0));
		y -= font_height*1.2;
		draw_text(font, tprint("pt y: %d", window.point_y), font_height, v2(x, y), v2(1, 1), v4(.9, .9, .9, 1.0));
		y -= font_height*1.5;
		draw_text(font, tprint("Fullscreen: %b", window.fullscreen), font_height, v2(x, y), v2(1, 1), v4(.9, .9, .9, 1.0));
		y -= font_height*1.2;
		draw_text(font, tprint("Mouse: %v2", v2(input_frame.mouse_x, input_frame.mouse_y)), font_height, v2(x, y), v2(1, 1), v4(.9, .9, .9, 1.0));
		y -= font_height*1.2;
		draw_text(font, tprint("DPI: %d", window.dpi), font_height, v2(x, y), v2(1, 1), v4(.9, .9, .9, 1.0));
		y -= font_height*1.2;
		draw_text(font, tprint("Point size in pixels: %.3f", window.point_size_in_pixels), font_height, v2(x, y), v2(1, 1), v4(.9, .9, .9, 1.0));
		y -= font_height*1.2;
		
		x = 0;
		y =  window.height/2 - 40 - font_height;
		
		float32 w = 200;
		float32 h = font_height*1.5;
		
		if (button(STR("Fullscreen"), v2(x-w/2, y-h/2), v2(w, h), window.fullscreen)) window.fullscreen = !window.fullscreen;
		y -= h*1.3;
		if (button(STR("Vsync"), v2(x-w/2, y-h/2), v2(w, h), window.enable_vsync)) window.enable_vsync = !window.enable_vsync;
		y -= h*1.3;
		if (button(STR("Allow resize"), v2(x-w/2, y-h/2), v2(w, h), window.allow_resize)) window.allow_resize = !window.allow_resize;
		y -= h*1.3;
		if (button(STR("Topmost"), v2(x-w/2, y-h/2), v2(w, h), window.force_topmost)) window.force_topmost = !window.force_topmost;
		y -= h*1.3;
		if (button(STR("Move right"), v2(x-w/2, y-h/2), v2(w, h), false)) window.point_x += 20;
		y -= h*1.3;
		if (button(STR("Move left"), v2(x-w/2, y-h/2), v2(w, h), false)) window.point_x -= 20;
		y -= h*1.3;
		if (button(STR("Move up"), v2(x-w/2, y-h/2), v2(w, h), false)) window.point_y += 20;
		y -= h*1.3;
		if (button(STR("Move down"), v2(x-w/2, y-h/2), v2(w, h), false)) window.point_y -= 20;
		y -= h*1.3;
		if (button(STR("+ width"), v2(x-w/2, y-h/2), v2(w, h), false)) window.point_width += 20;
		y -= h*1.3;
		if (button(STR("- width"), v2(x-w/2, y-h/2), v2(w, h), false)) window.point_width -= 20;
		y -= h*1.3;
		if (button(STR("+ height"), v2(x-w/2, y-h/2), v2(w, h), false)) window.point_height += 20;
		y -= h*1.3;
		if (button(STR("- height"), v2(x-w/2, y-h/2), v2(w, h), false)) window.point_height -= 20;
		y -= h*1.3;
		
		os_update(); 
		gfx_update();
	}

	return 0;
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