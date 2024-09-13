
/*

	NOTES: 
		- For creating, loading, managing Gfx_Image's see gfx_interface.c.
		- For clearing and rendering a draw_frame to offscreen render target or to
			the window, see gfx_render_draw_frame and gfx_render_draw_frame_to_window
			in gfx_interface.c.
		- A practical example for offscreen drawing can be found in examples/offscreen_drawing.c
		- A practical example for using Draw_Frame's can be found in examples/threaded_drawing.c


	The drawing API has two modes: EZ mode and advanced mode.
	
	EZ mode:
	
		EZ mode let's you just draw your things right away without caring about anything else.
		
		These functions will draw to the global draw_frame, which is always rendered, reset, and cleared in each
		call to gfx_update().
		
		- To draw a rectangle:
		
			draw_rect(v2(x, y), v2(width, height), v4(r, g, b, a));
			
		- Or, for more advanced transformation:
	
			Matrix4 xform = ....;
			draw_rect_xform(xform, v2(x, y), v2(width, height), v4(r, g, b, a));
	
		This is the full API of EZ mode:
			
		- Drawing basic shapes & images:
		
			See "- Retroactively modifying quads" for more info about the Draw_Quad*
		
			Draw_Quad *draw_rect(Vector2 position, Vector2 size, Vector4 color);
			Draw_Quad *draw_rect_xform(Matrix4 xform, Vector2 size, Vector4 color);
			
			Draw_Quad *draw_circle(Vector2 position, Vector2 size, Vector4 color);
			Draw_Quad *draw_circle_xform(Matrix4 xform, Vector2 size, Vector4 color);
			
			Draw_Quad *draw_image(Gfx_Image *image, Vector2 position, Vector2 size, Vector4 color);
			Draw_Quad *draw_image_xform(Gfx_Image *image, Matrix4 xform, Vector2 size, Vector4 color);
			
			void draw_line(Vector2 p0, Vector2 p1, float line_width, Vector4 color);
		
		- Drawing text:
			
			void draw_text_xform(Gfx_Font *font, string text, u32 raster_height, Matrix4 xform, Vector2 scale, Vector4 color);
			void draw_text(Gfx_Font *font, string text, u32 raster_height, Vector2 position, Vector2 scale, Vector4 color);
			Gfx_Text_Metrics draw_text_and_measure(Gfx_Font *font, string text, u32 raster_height, Vector2 position, Vector2 scale, Vector4 color);	
	
			- For loading and dealing with fonts see font.c, or for a practical example see examples/text_rendering.c
			
		- Lower-level quad drawing:
		
			Draw_Quad *draw_quad_projected(Draw_Quad quad, Matrix4 world_to_clip);
			Draw_Quad *draw_quad(Draw_Quad quad);
			Draw_Quad *draw_quad_xform(Draw_Quad quad, Matrix4 xform);
			
			- You'll need to create a Draw_Quad struct to pass here. 
			- You mostly shouldn't need to use this as it's quite verbose.
			- See struct Draw_Quad. 
			- If you need to customize a quad more, such as setting the UV or image filtering, then most other 
				draw_xxx functions will return a Draw_Quad* which you can modify Retroactively. Keep in mind 
				that the returned pointer is only guaranteed to be valid BEFORE you call other draw functions.
				See "- Retroactively modifying quads" for more info about Draw_Quad
				
		- Layer sorting, scissor boxing/cropping:
		
			void push_z_layer(s32 z);
			void pop_z_layer(void);
			void push_window_scissor(Vector2 min, Vector2 max);
			void pop_window_scissor(void);
			
		- Draw_Frame context stuff:
			
			Matrix4 draw_frame.projection
			Matrix4 draw_frame.camera_xform
			void   *draw_frame.cbuffer
			
			Since draw_frame is completely reset each gfx_update, the projection and camera_xform
			needs to be set each frame (immediate mode).
			
			The draw_frame.projection is a typical graphics programming projection, which we usually set to
			m4_make_orthographic_projecttion(...), and the draw_frame.camera_xform is the inverse of the
			typical "view". So; when transforming camera_xform for example to x += 40, then it's
			conceptually the camera that moves to the right, rather than the whole world moving to
			the right. Same for scaling; we change the size of the camera lens, which means that the
			world will appear smaller with a larger camera.
			
			The projection and xform gets applied directly in each draw_xxx call. So, you need to set
			the camera stuff just before drawing stuff to a specific camera.
			
			The cbuffer is for passing a constant buffer to the custom shader. For more info on custom
			shading, see examples/custom_shader.c.
				
	Advanced mode:
	
		Advanced mode is very similar to EZ mode, but we add _in_frame to all of the draw functions and
		add an additional parameter to each: Draw_Frame *frame.
		
		
		
		This means, of course, there is also an api to init and manage Draw_Frame's manually:
		
			void draw_frame_init(Draw_Frame *frame);
			void draw_frame_init_reserve(Draw_Frame *frame, u64 number_of_quads_to_reserve);
			void draw_frame_reset(Draw_Frame *frame);
			
			- draw_frame_init needs to be called once to set up some initial stuff. I don't like this so it
				might change.
			- draw_frame_init_reserve does the same as draw_frame_init, but you can pre-allocate for a certain
				amount of quads.
			- draw_frame_reset will, in short, clear the array of computed Draw_Quad's and zero everything
				out.	
				
			- A practical example for using Draw_Frame's can be found in examples/threaded_drawing.c	
		
		- The rest of the advanced API, similar to EZ mode:
		
			Draw_Quad *draw_quad_projected_in_frame(Draw_Quad quad, Matrix4 world_to_clip, Draw_Frame *frame);
			Draw_Quad *draw_quad_in_frame(Draw_Quad quad, Draw_Frame *frame);
			Draw_Quad *draw_quad_xform_in_frame(Draw_Quad quad, Matrix4 xform, Draw_Frame *frame);
			
			Draw_Quad *draw_rect_in_frame(Vector2 position, Vector2 size, Vector4 color, Draw_Frame *frame);
			Draw_Quad *draw_rect_xform_in_frame(Matrix4 xform, Vector2 size, Vector4 color, Draw_Frame *frame);
			
			Draw_Quad *draw_circle_in_frame(Vector2 position, Vector2 size, Vector4 color, Draw_Frame *frame);
			Draw_Quad *draw_circle_xform_in_frame(Matrix4 xform, Vector2 size, Vector4 color, Draw_Frame *frame);
			
			Draw_Quad *draw_image_in_frame(Gfx_Image *image, Vector2 position, Vector2 size, Vector4 color, Draw_Frame *frame);
			Draw_Quad *draw_image_xform_in_frame(Gfx_Image *image, Matrix4 xform, Vector2 size, Vector4 color, Draw_Frame *frame);
				
			void draw_line_in_frame(Vector2 p0, Vector2 p1, float line_width, Vector4 color, Draw_Frame *frame);
			
			void draw_text_xform_in_frame(Gfx_Font *font, string text, u32 raster_height, Matrix4 xform, Vector2 scale, Vector4 color, Draw_Frame *frame);
			void draw_text_in_frame(Gfx_Font *font, string text, u32 raster_height, Vector2 position, Vector2 scale, Vector4 color, Draw_Frame *frame);
			Gfx_Text_Metrics draw_text_and_measure_in_frame(Gfx_Font *font, string text, u32 raster_height, Vector2 position, Vector2 scale, Vector4 color, Draw_Frame *frame);
			
			void push_z_layer_in_frame(s32 z, Draw_Frame *frame);
			void pop_z_layer_in_frame(Draw_Frame *frame);
			void push_window_scissor_in_frame(Vector2 min, Vector2 max, Draw_Frame *frame);
			void pop_window_scissor_in_frame(Draw_Frame *frame);
			
			
	- Retroactively modifying quads
		
		All draw_xxx functions (except text) returns a Draw_Quad*. This can be used to either slightly modify
		the quad you just drew OR set some useful things like:
			- Vector4         Draw_Quad.uv: A normalized x0, y0, x1, y1 box describing the uv/texture coords
											to specify which part of the image to be sampled. By default this
											will be v4(0.0, 0.0, 1.0, 1.0) which means the whole image will be
											sampled.
			- s32             Draw_Quad.z: A value used for sorting. To enable this you must set 
										   draw_frame.enable_z_sorting to true each frame.
			- Gfx_Filter_Mode Draw_Quad.image_min_filter
			- Gfx_Filter_Mode Draw_Quad.image_mag_filter
				
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
	// #Cleanup
	union {
		DEPRECATED(Matrix4 view, "Use Draw_Frame.camera_xform instead");
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

void draw_frame_init(Draw_Frame *frame) {
	*frame = ZERO(Draw_Frame);
	
	growing_array_init((void**)&frame->quad_buffer, sizeof(Draw_Quad), get_heap_allocator());
}
void draw_frame_init_reserve(Draw_Frame *frame, u64 number_of_quads_to_reserve) {
	*frame = ZERO(Draw_Frame);
	
	growing_array_init_reserve((void**)&frame->quad_buffer, sizeof(Draw_Quad), number_of_quads_to_reserve, get_heap_allocator());
}

void draw_frame_reset(Draw_Frame *frame) {

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

// This is the global draw frame which is rendered and reset each time you call gfx_update();
ogb_instance Draw_Frame draw_frame;

#if !OOGABOOGA_LINK_EXTERNAL_INSTANCE
// #Global
Draw_Frame draw_frame;
#endif // NOT OOGABOOGA_LINK_EXTERNAL_INSTANCE

Draw_Quad _nil_quad = {0};
Draw_Quad *draw_quad_projected_in_frame(Draw_Quad quad, Matrix4 world_to_clip, Draw_Frame *frame) {
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
	if (frame->z_count > 0)  quad.z = frame->z_stack[frame->z_count-1];
	
	quad.has_scissor = false;
	if (frame->scissor_count > 0) {
		quad.scissor = frame->scissor_stack[frame->scissor_count-1];
		quad.has_scissor = true;
	}
	
	memset(quad.userdata, 0, sizeof(quad.userdata));
	
	Draw_Quad **target_buffer = &frame->quad_buffer;
	
	growing_array_add((void**)target_buffer, &quad);
	
	Draw_Quad *q = &(*target_buffer)[growing_array_get_valid_count(*target_buffer)-1];
	
	// This is meant to fix the annoying artifacts that shows up when sampling from a large atlas
    // presumably for floating point precision issues or something.

    // #Incomplete
    // If we want to animate text with small movements then it will look wonky.
    // This should be optional probably.

	float pixel_width = 2.0/(float)window.width;
	float pixel_height = 2.0/(float)window.height;
	
	q->bottom_left.x  = round(q->bottom_left.x  / pixel_width)  * pixel_width;
    q->bottom_left.y  = round(q->bottom_left.y  / pixel_height) * pixel_height;
    q->top_left.x     = round(q->top_left.x     / pixel_width)  * pixel_width;
    q->top_left.y     = round(q->top_left.y     / pixel_height) * pixel_height;
    q->top_right.x    = round(q->top_right.x    / pixel_width)  * pixel_width;
    q->top_right.y    = round(q->top_right.y    / pixel_height) * pixel_height;
    q->bottom_right.x = round(q->bottom_right.x / pixel_width)  * pixel_width;
    q->bottom_right.y = round(q->bottom_right.y / pixel_height) * pixel_height;
	
	return q;
}
Draw_Quad *draw_quad_in_frame(Draw_Quad quad, Draw_Frame *frame) {
	return draw_quad_projected_in_frame(quad, m4_mul(frame->projection, m4_inverse(frame->camera_xform)), frame);
}

Draw_Quad *draw_quad_xform_in_frame(Draw_Quad quad, Matrix4 xform, Draw_Frame *frame) {
	Matrix4 world_to_clip = m4_scalar(1.0);
	world_to_clip         = m4_mul(world_to_clip, frame->projection);
	world_to_clip         = m4_mul(world_to_clip, m4_inverse(frame->camera_xform));
	world_to_clip         = m4_mul(world_to_clip, xform);
	return draw_quad_projected_in_frame(quad, world_to_clip, frame);
}

Draw_Quad *draw_rect_in_frame(Vector2 position, Vector2 size, Vector4 color, Draw_Frame *frame) {
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
	
	return draw_quad_in_frame(q, frame);
}
Draw_Quad *draw_rect_xform_in_frame(Matrix4 xform, Vector2 size, Vector4 color, Draw_Frame *frame) {
	// #Copypaste #Volatile	
	Draw_Quad q = ZERO(Draw_Quad);
	q.bottom_left  = v2(0,  0);
	q.top_left     = v2(0,  size.y);
	q.top_right    = v2(size.x, size.y);
	q.bottom_right = v2(size.x, 0);
	q.color = color;
	q.image = 0;
	q.type = QUAD_TYPE_REGULAR;
	
	return draw_quad_xform_in_frame(q, xform, frame);
}
Draw_Quad *draw_circle_in_frame(Vector2 position, Vector2 size, Vector4 color, Draw_Frame *frame) {
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
	
	return draw_quad_in_frame(q, frame);
}
Draw_Quad *draw_circle_xform_in_frame(Matrix4 xform, Vector2 size, Vector4 color, Draw_Frame *frame) {
	// #Copypaste #Volatile	
	Draw_Quad q = ZERO(Draw_Quad);
	q.bottom_left  = v2(0,  0);
	q.top_left     = v2(0,  size.y);
	q.top_right    = v2(size.x, size.y);
	q.bottom_right = v2(size.x, 0);
	q.color = color;
	q.image = 0;
	q.type = QUAD_TYPE_CIRCLE;
	
	return draw_quad_xform_in_frame(q, xform, frame);
}
Draw_Quad *draw_image_in_frame(Gfx_Image *image, Vector2 position, Vector2 size, Vector4 color, Draw_Frame *frame) {
	Draw_Quad *q = draw_rect_in_frame(position, size, color, frame);
	
	q->image = image;
	q->uv = v4(0, 0, 1, 1);
	
	return q;
}
Draw_Quad *draw_image_xform_in_frame(Gfx_Image *image, Matrix4 xform, Vector2 size, Vector4 color, Draw_Frame *frame) {
	Draw_Quad *q = draw_rect_xform_in_frame(xform, size, color, frame);
	
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
	Draw_Frame *frame;
} Draw_Text_Callback_Params;
bool draw_text_callback(Gfx_Glyph glyph, Gfx_Font_Atlas *atlas, float glyph_x, float glyph_y, void *ud) {

	u32 codepoint = glyph.codepoint;

	Draw_Text_Callback_Params *params = (Draw_Text_Callback_Params*)ud;
	
	Vector2 size = v2(glyph.width*params->scale.x, glyph.height*params->scale.y);
	
	Matrix4 glyph_xform = m4_translate(params->xform, v3(glyph_x, glyph_y, 0));
	
	Draw_Quad *q = draw_image_xform_in_frame(atlas->image, glyph_xform, size, params->color, params->frame);
	q->uv = glyph.uv;
	q->type = QUAD_TYPE_TEXT;
	q->image_min_filter = GFX_FILTER_MODE_LINEAR;
	q->image_mag_filter = GFX_FILTER_MODE_LINEAR;
	
	return true;
}

void draw_text_xform_in_frame(Gfx_Font *font, string text, u32 raster_height, Matrix4 xform, Vector2 scale, Vector4 color, Draw_Frame *frame) {
	
	Draw_Text_Callback_Params p;
	p.font = font;
	p.text = text;
	p.raster_height = raster_height;
	p.xform = xform;
	p.scale = scale;
	p.color = color;
	p.frame = frame;
	
	walk_glyphs((Walk_Glyphs_Spec){font, text, raster_height, scale, true, &p}, draw_text_callback);
}
void draw_text_in_frame(Gfx_Font *font, string text, u32 raster_height, Vector2 position, Vector2 scale, Vector4 color, Draw_Frame *frame) {
	Matrix4 xform = m4_scalar(1.0);
	xform         = m4_translate(xform, v3(position.x, position.y, 0));
	
	draw_text_xform_in_frame(font, text, raster_height, xform, scale, color, frame);
}
Gfx_Text_Metrics draw_text_and_measure_in_frame(Gfx_Font *font, string text, u32 raster_height, Vector2 position, Vector2 scale, Vector4 color, Draw_Frame *frame) {
	Matrix4 xform = m4_scalar(1.0);
	xform         = m4_translate(xform, v3(position.x, position.y, 0));
	
	draw_text_xform_in_frame(font, text, raster_height, xform, scale, color, frame);
	
	return measure_text(font, text, raster_height, scale);
}

void draw_line_in_frame(Vector2 p0, Vector2 p1, float line_width, Vector4 color, Draw_Frame *frame) {
	Vector2 dir = v2(p1.x - p0.x, p1.y - p0.y);
	float length = sqrt(dir.x * dir.x + dir.y * dir.y);
	float r = atan2(-dir.y, dir.x);
	Matrix4 line_xform = m4_scalar(1);
	line_xform = m4_translate(line_xform, v3(p0.x, p0.y, 0));
	line_xform = m4_rotate_z(line_xform, r);
	line_xform = m4_translate(line_xform, v3(0, -line_width/2, 0));
	draw_rect_xform_in_frame(line_xform, v2(length, line_width), color, frame);
}

void push_z_layer_in_frame(s32 z, Draw_Frame *frame) {
	assert(frame->z_count < Z_STACK_MAX, "Too many z layers pushed. You can pop with pop_z_layer() when you are done drawing to it.");
	
	frame->z_stack[frame->z_count] = z;
	frame->z_count += 1;
}
void pop_z_layer_in_frame(Draw_Frame *frame) {
	assert(frame->z_count > 0, "No Z layers to pop!");
	frame->z_count -= 1;
}

void push_window_scissor_in_frame(Vector2 min, Vector2 max, Draw_Frame *frame) {
	assert(frame->scissor_count < SCISSOR_STACK_MAX, "Too many scissors pushed. You can pop with pop_window_scissor() when you are done drawing to it.");
	
	frame->scissor_stack[frame->scissor_count] = v4(min.x, min.y, max.x, max.y);
	frame->scissor_count += 1;
}
void pop_window_scissor_in_frame(Draw_Frame *frame) {
	assert(frame->scissor_count > 0, "No scissors to pop!");
	frame->scissor_count -= 1;
}


///
// Global draw api (draw to global draw_frame)
//

inline
Draw_Quad *draw_quad_projected(Draw_Quad quad, Matrix4 world_to_clip) { 
	return draw_quad_projected_in_frame(quad, world_to_clip, &draw_frame); 
}
inline
Draw_Quad *draw_quad(Draw_Quad quad) {
	return draw_quad_in_frame(quad, &draw_frame);
}

inline
Draw_Quad *draw_quad_xform(Draw_Quad quad, Matrix4 xform) {
	return draw_quad_xform_in_frame(quad, xform, &draw_frame);
}

inline
Draw_Quad *draw_rect(Vector2 position, Vector2 size, Vector4 color) {
	return draw_rect_in_frame(position, size, color, &draw_frame);
}
inline
Draw_Quad *draw_rect_xform(Matrix4 xform, Vector2 size, Vector4 color) {
	return draw_rect_xform_in_frame(xform, size, color, &draw_frame);
}
inline
Draw_Quad *draw_circle(Vector2 position, Vector2 size, Vector4 color) {
	return draw_circle_in_frame(position, size, color, &draw_frame);
}
inline
Draw_Quad *draw_circle_xform(Matrix4 xform, Vector2 size, Vector4 color) {
	return draw_circle_xform_in_frame(xform, size, color, &draw_frame);
}
inline
Draw_Quad *draw_image(Gfx_Image *image, Vector2 position, Vector2 size, Vector4 color) {
	return draw_image_in_frame(image, position, size, color, &draw_frame);
}
inline
Draw_Quad *draw_image_xform(Gfx_Image *image, Matrix4 xform, Vector2 size, Vector4 color) {
	return draw_image_xform_in_frame(image, xform, size, color, &draw_frame);
}

inline
void draw_text_xform(Gfx_Font *font, string text, u32 raster_height, Matrix4 xform, Vector2 scale, Vector4 color) {
	draw_text_xform_in_frame(font, text, raster_height, xform, scale, color, &draw_frame);
}
inline
void draw_text(Gfx_Font *font, string text, u32 raster_height, Vector2 position, Vector2 scale, Vector4 color) {
	draw_text_in_frame(font, text, raster_height, position, scale, color, &draw_frame);
}
Gfx_Text_Metrics draw_text_and_measure(Gfx_Font *font, string text, u32 raster_height, Vector2 position, Vector2 scale, Vector4 color) {
	return draw_text_and_measure_in_frame(font, text, raster_height, position, scale, color, &draw_frame);
}

inline
void draw_line(Vector2 p0, Vector2 p1, float line_width, Vector4 color) {
	draw_line_in_frame(p0, p1, line_width, color, &draw_frame);
}

inline
void push_z_layer(s32 z) { push_z_layer_in_frame(z, &draw_frame); }
inline
void pop_z_layer() { pop_z_layer_in_frame(&draw_frame); }

inline
void push_window_scissor(Vector2 min, Vector2 max) { push_window_scissor_in_frame(min, max, &draw_frame); }
inline
void pop_window_scissor() { pop_window_scissor_in_frame(&draw_frame); }


#define COLOR_RED   ((Vector4){1.0, 0.0, 0.0, 1.0})
#define COLOR_GREEN ((Vector4){0.0, 1.0, 0.0, 1.0})
#define COLOR_BLUE  ((Vector4){0.0, 0.0, 1.0, 1.0})
#define COLOR_WHITE ((Vector4){1.0, 1.0, 1.0, 1.0})
#define COLOR_BLACK ((Vector4){0.0, 0.0, 0.0, 1.0})

