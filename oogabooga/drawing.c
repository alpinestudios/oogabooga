
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
	
	quad.color = v4(r, g, b, a);

	draw_quad(quad);
	
	
	// Basic rect. Bottom left at X=-0.25, Y=-0.5 with a size of W=0.5, H=0.5
	draw_rect(v2(-0.25, -0.5), v2(0.5, 0.5), COLOR_GREEN);
	
	// Rotated rect. Bottom left at X=-0.25, Y=-0.5 with a size of W=0.5, H=0.5
	// With a centered pivot (half size) and a rotation of 2.4 radians
	// If pivot is v2(0, 0), the rectangle will rotate around it's bottom left.
	draw_rect_rotated(v2(-0.25, -0.5), v2(0.5, 0.5), COLOR_GREEN, v2(0.25, 0.25), 2.4f);
*/

#define QUADS_PER_BLOCK 1024

typedef struct Draw_Quad {
	Vector2 bottom_left, top_left, top_right, bottom_right;
	// r, g, b, a
	Vector4 color;
	
} Draw_Quad;

typedef struct Draw_Quad_Block {
	Draw_Quad quad_buffer[QUADS_PER_BLOCK];
	u64 num_quads;
	
	struct Draw_Quad_Block *next;
} Draw_Quad_Block;

typedef struct Draw_Frame {
	Draw_Quad_Block first_block;
	Draw_Quad_Block *current;
} Draw_Frame;

// This frame is passed to the platform layer and rendered in os_update.
// Resets every frame.
Draw_Frame draw_frame = ZERO(Draw_Frame);

void draw_quad(Draw_Quad quad) {
	if (!draw_frame.current) draw_frame.current = &draw_frame.first_block;
	
	assert(draw_frame.current->num_quads <= QUADS_PER_BLOCK);
	
	if (draw_frame.current->num_quads == QUADS_PER_BLOCK) {
		draw_frame.current->next = cast(Draw_Quad_Block*)talloc(sizeof(Draw_Quad_Block));
		draw_frame.current = draw_frame.current->next;
		
		draw_frame.current->next = 0;
		draw_frame.current->num_quads = 0;
	}
	
	draw_frame.current->quad_buffer[draw_frame.current->num_quads] = quad;
	draw_frame.current->num_quads += 1;
}

void draw_rect(Vector2 position, Vector2 size, Vector4 color) {
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
	
	draw_quad(q);
}
void draw_rect_rotated(Vector2 position, Vector2 size, Vector4 color, Vector2 local_pivot, float32 rotation) {

	const float32 left   = position.x;
	const float32 right  = position.x + size.x;
	const float32 bottom = position.y;
	const float32 top    = position.y+size.y;
	
	Vector2 bottom_left  = v2(left,  bottom);
	Vector2 top_left     = v2(left,  top);
	Vector2 top_right    = v2(right, top);
	Vector2 bottom_right = v2(right, bottom);

	Vector2 pivot = v2(position.x + local_pivot.x, position.y + local_pivot.y);

    Draw_Quad q;
    q.bottom_left  = v2_rotate_point_around_pivot(bottom_left,  pivot, rotation);
    q.top_left     = v2_rotate_point_around_pivot(top_left,     pivot, rotation);
    q.top_right    = v2_rotate_point_around_pivot(top_right,    pivot, rotation);
    q.bottom_right = v2_rotate_point_around_pivot(bottom_right, pivot, rotation);
    q.color = color;

    draw_quad(q);
}


#define COLOR_RED   ((Vector4){1.0, 0.0, 0.0, 1.0})
#define COLOR_GREEN ((Vector4){0.0, 1.0, 0.0, 1.0})
#define COLOR_BLUE  ((Vector4){0.0, 0.0, 1.0, 1.0})