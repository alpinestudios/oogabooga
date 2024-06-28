
#define QUADS_PER_BLOCK 1024

typedef struct Draw_Quad {
	float left;
	float right;
	float bottom;
	float top;
	
	Vector4 color;
	
} Draw_Quad;

typedef struct Draw_Quad_Block {
	Draw_Quad quad_buffer[QUADS_PER_BLOCK];
	u64 num_quads;
	
	struct Draw_Quad_Block *next;
} Draw_Quad_Block;

typedef struct Draw_Frame {
	Draw_Quad_Block first_quad_block;
} Draw_Frame;

