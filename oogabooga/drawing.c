
/*
	void push_z_layer(s32 z);
	void pop_z_layer();
	void push_window_scissor(Vector2 min, Vector2 max);
	void pop_window_scissor();
	
	Draw_Quad *draw_rect(Vector2 position, Vector2 size, Vector4 color);
	Draw_Quad *draw_rect_xform(Matrix4 xform, Vector2 size, Vector4 color);
	Draw_Quad *draw_circle(Vector2 position, Vector2 size, Vector4 color);
	Draw_Quad *draw_circle_xform(Matrix4 xform, Vector2 size, Vector4 color);
	Draw_Quad *draw_image(Gfx_Image *image, Vector2 position, Vector2 size, Vector4 color);
	Draw_Quad *draw_image_xform(Gfx_Image *image, Matrix4 xform, Vector2 size, Vector4 color);
	Draw_Quad *draw_quad_projected(Draw_Quad quad, Matrix4 world_to_clip);
	Draw_Quad *draw_quad(Draw_Quad quad);
	Draw_Quad *draw_quad_xform(Draw_Quad quad, Matrix4 xform);
	void draw_text_xform(Gfx_Font *font, string text, u32 raster_height, Matrix4 xform, Vector2 scale, Vector4 color);
	void draw_text(Gfx_Font *font, string text, u32 raster_height, Vector2 position, Vector2 scale, Vector4 color);
	Gfx_Text_Metrics draw_text_and_measure(Gfx_Font *font, string text, u32 raster_height, Vector2 position, Vector2 scale, Vector4 color);
	void draw_line(Vector2 p0, Vector2 p1, float line_width, Vector4 color);
*/

// We use radix sort so the exact bit count is of importance
#define MAX_Z_BITS 21
#define MAX_Z ((1 << MAX_Z_BITS)/2)
#define Z_STACK_MAX 4096
#define SCISSOR_STACK_MAX 4096

typedef struct Draw_Quad {
	// BEWARE !! These are in ndc
	Vector2 bottom_left, top_left, top_right, bottom_right;
	// r, g, b, a
	Vector4 color;
	Gfx_Image *image;
	Gfx_Filter_Mode image_min_filter;
	Gfx_Filter_Mode image_mag_filter;
	s32 z;
	u8 type;
	bool has_scissor;
	// x1, y1, x2, y2
	Vector4 uv;
	Vector4 scissor;
	
	Vector4 userdata[VERTEX_2D_USER_DATA_COUNT]; // #Volatile do NOT change this to a pointer
	
} Draw_Quad;



typedef struct Draw_Frame {
	Matrix4 projection;
	union {
		DEPRECATED(Matrix4 view, "Use draw_frame.camera_xform instead");
		Matrix4 camera_xform;
	};
	
	void *cbuffer;
	
	u64 scissor_count;
	Vector4 scissor_stack[SCISSOR_STACK_MAX];
	
	Draw_Quad *quad_buffer;
	
	u64 z_count;
	s32 z_stack[Z_STACK_MAX];
	bool enable_z_sorting;
	
} Draw_Frame;

// This frame is passed to the platform layer and rendered in os_update.
// Resets every frame.
ogb_instance Draw_Frame draw_frame;

#if !OOGABOOGA_LINK_EXTERNAL_INSTANCE
Draw_Frame draw_frame;
#endif // NOT OOGABOOGA_LINK_EXTERNAL_INSTANCE

void reset_draw_frame(Draw_Frame *frame) {

	// #Memory
	// I would like to try to have the quad buffer to be allocated in a growing arena
	// which is reset every frames, like temp allocator but large enough to fit the
	// highest number of quads the program submits in a frame.
	// For now, we just reset the count in the heap allocated buffer

	Draw_Quad *quad_buffer = frame->quad_buffer;
	if (quad_buffer) growing_array_clear((void**)&quad_buffer);

	*frame = (Draw_Frame){0};
	
	frame->quad_buffer = quad_buffer;
	
	frame->projection 
		= m4_make_orthographic_projection(-window.width/2, window.width/2, -window.height/2, window.height/2, -1, 10);
	frame->camera_xform = m4_scalar(1.0);
}

void push_z_layer(s32 z) {
	assert(draw_frame.z_count < Z_STACK_MAX, "Too many z layers pushed. You can pop with pop_z_layer() when you are done drawing to it.");
	
	draw_frame.z_stack[draw_frame.z_count] = z;
	draw_frame.z_count += 1;
}
void pop_z_layer() {
	assert(draw_frame.z_count > 0, "No Z layers to pop!");
	draw_frame.z_count -= 1;
}

void push_window_scissor(Vector2 min, Vector2 max) {
	assert(draw_frame.scissor_count < SCISSOR_STACK_MAX, "Too many scissors pushed. You can pop with pop_window_scissor() when you are done drawing to it.");
	
	draw_frame.scissor_stack[draw_frame.scissor_count] = v4(min.x, min.y, max.x, max.y);
	draw_frame.scissor_count += 1;
}
void pop_window_scissor() {
	assert(draw_frame.scissor_count > 0, "No scissors to pop!");
	draw_frame.scissor_count -= 1;
}

Draw_Quad _nil_quad = {0};
Draw_Quad *draw_quad_projected(Draw_Quad quad, Matrix4 world_to_clip) {
	quad.bottom_left  = m4_transform(world_to_clip, v4(v2_expand(quad.bottom_left), 0, 1)).xy;
	quad.top_left     = m4_transform(world_to_clip, v4(v2_expand(quad.top_left), 0, 1)).xy;
	quad.top_right    = m4_transform(world_to_clip, v4(v2_expand(quad.top_right), 0, 1)).xy;
	quad.bottom_right = m4_transform(world_to_clip, v4(v2_expand(quad.bottom_right), 0, 1)).xy;
	
	bool should_cull = 
	    (quad.bottom_left.x < -1 && quad.top_left.x < -1 && quad.top_right.x < -1 && quad.bottom_right.x < -1) ||
	    (quad.bottom_left.x > 1 && quad.top_left.x > 1 && quad.top_right.x > 1 && quad.bottom_right.x > 1) ||
	    (quad.bottom_left.y < -1 && quad.top_left.y < -1 && quad.top_right.y < -1 && quad.bottom_right.y < -1) ||
	    (quad.bottom_left.y > 1 && quad.top_left.y > 1 && quad.top_right.y > 1 && quad.bottom_right.y > 1);

	if (should_cull) {
		return &_nil_quad;
	}
	
	quad.image_min_filter = GFX_FILTER_MODE_NEAREST;
	quad.image_mag_filter = GFX_FILTER_MODE_NEAREST;
	
	
	quad.z = 0;
	if (draw_frame.z_count > 0)  quad.z = draw_frame.z_stack[draw_frame.z_count-1];
	
	quad.has_scissor = false;
	if (draw_frame.scissor_count > 0) {
		quad.scissor = draw_frame.scissor_stack[draw_frame.scissor_count-1];
		quad.has_scissor = true;
	}
	
	memset(quad.userdata, 0, sizeof(quad.userdata));
	
	if (!draw_frame.quad_buffer) {
		// #Memory
		// Use an arena
		growing_array_init((void**)&draw_frame.quad_buffer, sizeof(Draw_Quad), get_heap_allocator());
	}
	
	Draw_Quad **target_buffer = &draw_frame.quad_buffer;
	
	growing_array_add((void**)target_buffer, &quad);
	
	return &(*target_buffer)[growing_array_get_valid_count(*target_buffer)-1];
}
Draw_Quad *draw_quad(Draw_Quad quad) {
	return draw_quad_projected(quad, m4_mul(draw_frame.projection, m4_inverse(draw_frame.camera_xform)));
}

Draw_Quad *draw_quad_xform(Draw_Quad quad, Matrix4 xform) {
	Matrix4 world_to_clip = m4_scalar(1.0);
	world_to_clip         = m4_mul(world_to_clip, draw_frame.projection);
	world_to_clip         = m4_mul(world_to_clip, m4_inverse(draw_frame.camera_xform));
	world_to_clip         = m4_mul(world_to_clip, xform);
	return draw_quad_projected(quad, world_to_clip);
}

Draw_Quad *draw_rect(Vector2 position, Vector2 size, Vector4 color) {
	// #Copypaste #Volatile	
	const float32 left   = position.x;
	const float32 right  = position.x + size.x;
	const float32 bottom = position.y;
	const float32 top    = position.y+size.y;
	
	Draw_Quad q;
	q.bottom_left  = v2(left,  bottom);
	q.top_left     = v2(left,  top);
	q.top_right    = v2(right, top);
	q.bottom_right = v2(right, bottom);
	q.color = color;
	q.image = 0;
	q.type = QUAD_TYPE_REGULAR;
	
	return draw_quad(q);
}
Draw_Quad *draw_rect_xform(Matrix4 xform, Vector2 size, Vector4 color) {
	// #Copypaste #Volatile	
	Draw_Quad q = ZERO(Draw_Quad);
	q.bottom_left  = v2(0,  0);
	q.top_left     = v2(0,  size.y);
	q.top_right    = v2(size.x, size.y);
	q.bottom_right = v2(size.x, 0);
	q.color = color;
	q.image = 0;
	q.type = QUAD_TYPE_REGULAR;
	
	return draw_quad_xform(q, xform);
}
Draw_Quad *draw_circle(Vector2 position, Vector2 size, Vector4 color) {
	// #Copypaste #Volatile	
	const float32 left   = position.x;
	const float32 right  = position.x + size.x;
	const float32 bottom = position.y;
	const float32 top    = position.y+size.y;
	
	Draw_Quad q;
	q.bottom_left  = v2(left,  bottom);
	q.top_left     = v2(left,  top);
	q.top_right    = v2(right, top);
	q.bottom_right = v2(right, bottom);
	q.color = color;
	q.image = 0;
	q.type = QUAD_TYPE_CIRCLE;
	
	return draw_quad(q);
}
Draw_Quad *draw_circle_xform(Matrix4 xform, Vector2 size, Vector4 color) {
	// #Copypaste #Volatile	
	Draw_Quad q = ZERO(Draw_Quad);
	q.bottom_left  = v2(0,  0);
	q.top_left     = v2(0,  size.y);
	q.top_right    = v2(size.x, size.y);
	q.bottom_right = v2(size.x, 0);
	q.color = color;
	q.image = 0;
	q.type = QUAD_TYPE_CIRCLE;
	
	return draw_quad_xform(q, xform);
}
Draw_Quad *draw_image(Gfx_Image *image, Vector2 position, Vector2 size, Vector4 color) {
	Draw_Quad *q = draw_rect(position, size, color);
	
	q->image = image;
	q->uv = v4(0, 0, 1, 1);
	
	return q;
}
Draw_Quad *draw_image_xform(Gfx_Image *image, Matrix4 xform, Vector2 size, Vector4 color) {
	Draw_Quad *q = draw_rect_xform(xform, size, color);
	
	q->image = image;
	q->uv = v4(0, 0, 1, 1);
	
	return q;
}

typedef struct {
	Gfx_Font *font;
	string text;
	u32 raster_height;
	Matrix4 xform;
	Vector2 scale;
	Vector4 color;
} Draw_Text_Callback_Params;
bool draw_text_callback(Gfx_Glyph glyph, Gfx_Font_Atlas *atlas, float glyph_x, float glyph_y, void *ud) {

	u32 codepoint = glyph.codepoint;

	Draw_Text_Callback_Params *params = (Draw_Text_Callback_Params*)ud;
	
	Vector2 size = v2(glyph.width*params->scale.x, glyph.height*params->scale.y);
	
	Matrix4 glyph_xform = m4_translate(params->xform, v3(glyph_x, glyph_y, 0));
	
	Draw_Quad *q = draw_image_xform(atlas->image, glyph_xform, size, params->color);
	q->uv = glyph.uv;
	q->type = QUAD_TYPE_TEXT;
	q->image_min_filter = GFX_FILTER_MODE_LINEAR;
	q->image_mag_filter = GFX_FILTER_MODE_LINEAR;
	
	return true;
}

void draw_text_xform(Gfx_Font *font, string text, u32 raster_height, Matrix4 xform, Vector2 scale, Vector4 color) {
	
	Draw_Text_Callback_Params p;
	p.font = font;
	p.text = text;
	p.raster_height = raster_height;
	p.xform = xform;
	p.scale = scale;
	p.color = color;
	
	walk_glyphs((Walk_Glyphs_Spec){font, text, raster_height, scale, true, &p}, draw_text_callback);
}
void draw_text(Gfx_Font *font, string text, u32 raster_height, Vector2 position, Vector2 scale, Vector4 color) {
	Matrix4 xform = m4_scalar(1.0);
	xform         = m4_translate(xform, v3(position.x, position.y, 0));
	
	draw_text_xform(font, text, raster_height, xform, scale, color);
}
Gfx_Text_Metrics draw_text_and_measure(Gfx_Font *font, string text, u32 raster_height, Vector2 position, Vector2 scale, Vector4 color) {
	Matrix4 xform = m4_scalar(1.0);
	xform         = m4_translate(xform, v3(position.x, position.y, 0));
	
	draw_text_xform(font, text, raster_height, xform, scale, color);
	
	return measure_text(font, text, raster_height, scale);
}

void draw_line(Vector2 p0, Vector2 p1, float line_width, Vector4 color) {
	Vector2 dir = v2(p1.x - p0.x, p1.y - p0.y);
	float length = sqrt(dir.x * dir.x + dir.y * dir.y);
	float r = atan2(-dir.y, dir.x);
	Matrix4 line_xform = m4_scalar(1);
	line_xform = m4_translate(line_xform, v3(p0.x, p0.y, 0));
	line_xform = m4_rotate_z(line_xform, r);
	line_xform = m4_translate(line_xform, v3(0, -line_width/2, 0));
	draw_rect_xform(line_xform, v2(length, line_width), color);
}

#define COLOR_RED   ((Vector4){1.0, 0.0, 0.0, 1.0})
#define COLOR_GREEN ((Vector4){0.0, 1.0, 0.0, 1.0})
#define COLOR_BLUE  ((Vector4){0.0, 0.0, 1.0, 1.0})
#define COLOR_WHITE ((Vector4){1.0, 1.0, 1.0, 1.0})
#define COLOR_BLACK ((Vector4){0.0, 0.0, 0.0, 1.0})

