

// !!!!!!!! BUILD CONFIG SHOULD BE DONE IN build_engine.c

#define OOGABOOGA_LINK_EXTERNAL_INSTANCE 1
#include "oogabooga/oogabooga.c"

///
///
// This is the game module which is what can be recompiled in the engine runtime

// For the engine to be able to detect a symbol, it needs to be marked with SHARED_EXPORT
void SHARED_EXPORT
game_update(f64 delta_time) {

    float64 now = os_get_elapsed_seconds();

	Matrix4 rect_xform = m4_scalar(1.0);
	rect_xform         = m4_rotate_z(rect_xform, (f32)now);
	rect_xform         = m4_translate(rect_xform, v3(-.25f, -.25f, 0));
	draw_rect_xform(rect_xform, v2(.5f, .5f), COLOR_GREEN);
	
	draw_rect(v2(sin(now), -.8), v2(.5, .25), COLOR_RED);
	
	float aspect = (f32)window.width/(f32)window.height;
	float mx = (input_frame.mouse_x/(f32)window.width  * 2.0 - 1.0)*aspect;
	float my = input_frame.mouse_y/(f32)window.height * 2.0 - 1.0;
	
	draw_line(v2(-.75, -.75), v2(mx, my), 0.005, COLOR_WHITE);
}