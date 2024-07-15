
/*
Usage:

	Just call draw_xxx procedures anywhere in the frame when you want something to be drawn that frame.


	// Examples:

	// Verbose
	Draw_Quad quad;
	quad.bottom_left  = v2(x, y);
	quad.top_left     = v2(x, y);
	quad.top_right    = v2(x, y);
	quad.bottom_right = v2(x, y);
	quad.color        = v4(r, g, b, a);
	quad.image        = my_image; // ZERO(Gfx_Image) To draw a plain color
	quad.uv           = v4(0, 0, 1, 1); 
	
	draw_quad(quad);
	
	
	// Basic rect. Bottom left at X=-0.25, Y=-0.5 with a size of W=0.5, H=0.5
	draw_rect(v2(-0.25, -0.5), v2(0.5, 0.5), COLOR_GREEN);
	
	// Rotated rect. Bottom left at X=-0.25, Y=-0.5 with a size of W=0.5, H=0.5
	// With a centered pivot (half size) and a rotation of 2.4 radians
	// If pivot is v2(0, 0), the rectangle will rotate around it's bottom left.
	draw_rect_rotated(v2(-0.25, -0.5), v2(0.5, 0.5), COLOR_GREEN, v2(0.25, 0.25), 2.4f);
	
	// Basic image. Bottom left at X=-0.25, Y=-0.5 with a size of W=0.5, H=0.5
	draw_image(v2(-0.25, -0.5), v2(0.5, 0.5), COLOR_GREEN);
	
	// Rotated image. Bottom left at X=-0.25, Y=-0.5 with a size of W=0.5, H=0.5
	// With a centered pivot (half size) and a rotation of 2.4 radians
	// If pivot is v2(0, 0), the rectangle will rotate around it's bottom left.
	draw_image_rotated(v2(-0.25, -0.5), v2(0.5, 0.5), COLOR_GREEN, v2(0.25, 0.25), 2.4f);
	
	// Loading an image (png only)
	Gfx_Image image = load_image_from_disk("my_image.png");
	if (!image.data) {
		// We failed loading the image.
	}
	
	// If you ever need to free the image:
	delete_image(image);
	
	
	API:
	
	// !! IMPORTANT
	// The Draw_Quad* returned from draw procedures is a temporary pointer and may be
	// invalid after the next draw_xxxx call. This is because quads are stored in a
	// resizing buffer (because that gave us a non-trivial performance boost).
	// So the purpose of returning them is to customize the quad right after the draw proc.
	
	Draw_Quad *draw_quad_projected(Draw_Quad quad, Matrix4 world_to_clip);
	Draw_Quad *draw_quad(Draw_Quad quad);
	Draw_Quad *draw_quad_xform(Draw_Quad quad, Matrix4 xform);
	Draw_Quad *draw_rect(Vector2 position, Vector2 size, Vector4 color);
	Draw_Quad *draw_rect_xform(Matrix4 xform, Vector2 size, Vector4 color);
	Draw_Quad *draw_image(Gfx_Image *image, Vector2 position, Vector2 size, Vector4 color);
	Draw_Quad *draw_image_xform(Gfx_Image *image, Matrix4 xform, Vector2 size, Vector4 color);
	// raster_height is the pixel height that the text will be rasterized at. If text is blurry,
	// you can try to increase raster_height and lower scale.
	void draw_text(Gfx_Font *font, string text, u32 raster_height, Vector2 position, Vector2 scale, Vector4 color);
	void draw_text_xform(Gfx_Font *font, string text, u32 raster_height, Matrix4 xform, Vector4 color);
	
*/

// We use radix sort so the exact bit count is of importance
#define MAX_Z_BITS 21
#define MAX_Z ((1 << MAX_Z_BITS)/2)
#define Z_STACK_MAX 4096

typedef struct Draw_Quad {
	Vector2 bottom_left, top_left, top_right, bottom_right;
	// r, g, b, a
	Vector4 color;
	Gfx_Image *image;
	
	// x1, y1, x2, y2
	Vector4 uv;
	u8 type;
	
	Gfx_Filter_Mode image_min_filter;
	Gfx_Filter_Mode image_mag_filter;
	
	s32 z;
	
} Draw_Quad;


Draw_Quad *quad_buffer;
u64 allocated_quads;
typedef struct Draw_Frame {
	u64 num_quads;
	
	Matrix4 projection;
	Matrix4 view;
	
	bool enable_z_sorting;
	s32 z_stack[Z_STACK_MAX];
	u64 z_count;
} Draw_Frame;
// This frame is passed to the platform layer and rendered in os_update.
// Resets every frame.
Draw_Frame draw_frame = ZERO(Draw_Frame);

void reset_draw_frame(Draw_Frame *frame) {
	*frame = (Draw_Frame){0};
	
	float32 aspect = (float32)window.width/(float32)window.height;
	
	frame->projection = m4_make_orthographic_projection(-aspect, aspect, -1, 1, -1, 10);
	frame->view = m4_scalar(1.0);	
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

Draw_Quad *draw_quad_projected(Draw_Quad quad, Matrix4 world_to_clip) {
	quad.bottom_left  = m4_transform(world_to_clip, v4(v2_expand(quad.bottom_left), 0, 1)).xy;
	quad.top_left     = m4_transform(world_to_clip, v4(v2_expand(quad.top_left), 0, 1)).xy;
	quad.top_right    = m4_transform(world_to_clip, v4(v2_expand(quad.top_right), 0, 1)).xy;
	quad.bottom_right = m4_transform(world_to_clip, v4(v2_expand(quad.bottom_right), 0, 1)).xy;
	
	quad.image_min_filter = GFX_FILTER_MODE_NEAREST;
	quad.image_mag_filter = GFX_FILTER_MODE_NEAREST;
	
	quad.z = 0;
	if (draw_frame.z_count > 0)  quad.z = draw_frame.z_stack[draw_frame.z_count-1];
	
	if (draw_frame.num_quads >= allocated_quads) {
		// #Memory
		
		u64 new_count = max(get_next_power_of_two(draw_frame.num_quads+1), 128);
		
		Draw_Quad *new_buffer = alloc(get_heap_allocator(), new_count*sizeof(Draw_Quad));
		
		if (quad_buffer) {
			memcpy(new_buffer, quad_buffer, draw_frame.num_quads*sizeof(Draw_Quad));
			dealloc(get_heap_allocator(), quad_buffer);
		}
		
		quad_buffer = new_buffer;
		allocated_quads = new_count;
	}
	
	quad_buffer[draw_frame.num_quads] = quad;
	draw_frame.num_quads += 1;
	
	return &quad_buffer[draw_frame.num_quads-1];
}
Draw_Quad *draw_quad(Draw_Quad quad) {
	return draw_quad_projected(quad, m4_mul(draw_frame.projection, m4_inverse(draw_frame.view)));
}

Draw_Quad *draw_quad_xform(Draw_Quad quad, Matrix4 xform) {
	Matrix4 world_to_clip = m4_scalar(1.0);
	world_to_clip         = m4_mul(world_to_clip, draw_frame.projection);
	world_to_clip         = m4_mul(world_to_clip, m4_inverse(draw_frame.view));
	world_to_clip         = m4_mul(world_to_clip, xform);
	return draw_quad_projected(quad, world_to_clip);
}

Draw_Quad *draw_rect(Vector2 position, Vector2 size, Vector4 color) {
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


#define COLOR_RED   ((Vector4){1.0, 0.0, 0.0, 1.0})
#define COLOR_GREEN ((Vector4){0.0, 1.0, 0.0, 1.0})
#define COLOR_BLUE  ((Vector4){0.0, 0.0, 1.0, 1.0})
#define COLOR_WHITE ((Vector4){1.0, 1.0, 1.0, 1.0})
#define COLOR_BLACK ((Vector4){0.0, 0.0, 0.0, 1.0})

