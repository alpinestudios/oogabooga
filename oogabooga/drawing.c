
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
*/








#define QUADS_PER_BLOCK 1024
typedef struct Draw_Quad {
	Vector2 bottom_left, top_left, top_right, bottom_right;
	// r, g, b, a
	Vector4 color;
	Gfx_Image *image;
	
	// x1, y1, x2, y2
	Vector4 uv;
} Draw_Quad;


typedef struct Draw_Quad_Block {
	Draw_Quad quad_buffer[QUADS_PER_BLOCK];
	u64 num_quads;
	
	struct Draw_Quad_Block *next;
} Draw_Quad_Block;

// I made these blocks part of the frame at first so they were temp allocated BUT I think 
// that was a mistake because these blocks  are accessed a lot so we want it to just be
// persistent memory that's super hot all the time.
Draw_Quad_Block first_block = {0};

typedef struct Draw_Frame {
	Draw_Quad_Block *current;
	u64 num_blocks;
	
	Matrix4 projection;
	Matrix4 view;
	
	Gfx_Handle garbage_stack[4096];
	u64 garbage_stack_count;
} Draw_Frame;
// This frame is passed to the platform layer and rendered in os_update.
// Resets every frame.
Draw_Frame draw_frame = ZERO(Draw_Frame);

void reset_draw_frame(Draw_Frame *frame) {
	*frame = (Draw_Frame){0};
	
	frame->current = &first_block;
	frame->current->num_quads = 0;
	
	float32 aspect = (float32)window.width/(float32)window.height;
	
	frame->projection = m4_make_orthographic_projection(-aspect, aspect, -1, 1, -1, 10);
	frame->view = m4_scalar(1.0);
}

Draw_Quad *draw_quad_projected(Draw_Quad quad, Matrix4 world_to_clip) {
	quad.bottom_left  = m4_transform(world_to_clip, v4(v2_expand(quad.bottom_left), 0, 1)).xy;
	quad.top_left     = m4_transform(world_to_clip, v4(v2_expand(quad.top_left), 0, 1)).xy;
	quad.top_right    = m4_transform(world_to_clip, v4(v2_expand(quad.top_right), 0, 1)).xy;
	quad.bottom_right = m4_transform(world_to_clip, v4(v2_expand(quad.bottom_right), 0, 1)).xy;

	if (!draw_frame.current) draw_frame.current = &first_block;
	
	if (draw_frame.current == &first_block)  draw_frame.num_blocks = 1;
	
	assert(draw_frame.current->num_quads <= QUADS_PER_BLOCK);
	
	if (draw_frame.current->num_quads == QUADS_PER_BLOCK) {
		
		if (!draw_frame.current->next) {
			draw_frame.current->next = cast(Draw_Quad_Block*)alloc(get_heap_allocator(), sizeof(Draw_Quad_Block));
			*draw_frame.current->next = ZERO(Draw_Quad_Block);
		}
		
		draw_frame.current = draw_frame.current->next;
		draw_frame.current->num_quads = 0;
		
		draw_frame.num_blocks += 1;
	}
	
	draw_frame.current->quad_buffer[draw_frame.current->num_quads] = quad;
	draw_frame.current->num_quads += 1;
	
	return &draw_frame.current->quad_buffer[draw_frame.current->num_quads-1];
}
Draw_Quad *draw_quad(Draw_Quad quad) {
	return draw_quad_projected(quad, m4_multiply(draw_frame.projection, m4_inverse(draw_frame.view)));
}

Draw_Quad *draw_quad_xform(Draw_Quad quad, Matrix4 xform) {
	Matrix4 world_to_clip = m4_scalar(1.0);
	world_to_clip         = m4_multiply(world_to_clip, draw_frame.projection);
	world_to_clip         = m4_multiply(world_to_clip, m4_inverse(draw_frame.view));
	world_to_clip         = m4_multiply(world_to_clip, xform);
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


#define COLOR_RED   ((Vector4){1.0, 0.0, 0.0, 1.0})
#define COLOR_GREEN ((Vector4){0.0, 1.0, 0.0, 1.0})
#define COLOR_BLUE  ((Vector4){0.0, 0.0, 1.0, 1.0})
#define COLOR_WHITE ((Vector4){1.0, 1.0, 1.0, 1.0})
#define COLOR_BLACK ((Vector4){0.0, 0.0, 0.0, 1.0})

Gfx_Image *load_image_from_disk(string path, Allocator allocator) {
	string png;
	bool ok = os_read_entire_file(path, &png, allocator);
	if (!ok) return 0;

	Gfx_Image *image = alloc(allocator, sizeof(Gfx_Image));
	
	// This is fucking terrible I gotta write my own decoder

	lodepng_allocator = allocator;

	LodePNGState state;
	lodepng_state_init(&state);
	u32 error = lodepng_inspect(&image->width, &image->height, &state, png.data, png.count);
	if (error) {
		return 0;
	}
	
	// 5 lines of code to say "ignore_adler32 = true" (because it's broken and gives me an error)
	// I JUST WANT TO LOAD A PNG 
	LodePNGDecoderSettings decoder;
	lodepng_decoder_settings_init(&decoder);
	lodepng_decompress_settings_init(&decoder.zlibsettings);
	decoder.zlibsettings.ignore_adler32 = true;
	state.decoder = decoder;
	
	error = lodepng_decode(&image->data, &image->width, &image->height, &state, png.data, png.count);
	
	lodepng_state_cleanup(&state);
	
	dealloc_string(allocator, png);
	
	if (error) {
		return 0;
	}
	
	// We need to flip the image
	u32 row_bytes = image->width * 4;  // #Magicvalue assuming 4 bytes
    u8* temp_row = (u8*)alloc(temp, row_bytes);
    for (u32 i = 0; i < image->height / 2; i++) {
        u8* top_row = image->data + i * row_bytes;
        u8* bottom_row = image->data + (image->height - i - 1) * row_bytes;

        // Swap the top row with the bottom row
        memcpy(temp_row, top_row, row_bytes);
        memcpy(top_row, bottom_row, row_bytes);
        memcpy(bottom_row, temp_row, row_bytes);
    }
	
	image->gfx_handle = GFX_INVALID_HANDLE; // This is handled in gfx
	
	image->allocator = allocator;
	
	return image;
}
void delete_image(Gfx_Image *image) {
	dealloc(image->allocator, image->data);
	image->width = 0;
	image->height = 0;
	draw_frame.garbage_stack[draw_frame.garbage_stack_count] = image->gfx_handle;
	draw_frame.garbage_stack_count += 1;
	dealloc(image->allocator, image);
}