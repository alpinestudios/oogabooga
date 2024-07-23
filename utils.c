#define SPRITE_PIXEL_SIZE 16

Vector2 screen_to_world()
{
    float mouse_x = input_frame.mouse_x;
    float mouse_y = input_frame.mouse_y;
    Matrix4 proj = draw_frame.projection;
    Matrix4 view = draw_frame.view;

    // Normalize the mouse coordinates
    float ndc_x = (mouse_x / (window.width * 0.5f)) - 1.0f;
    float ndc_y = (mouse_y / (window.height * 0.5f)) - 1.0f;

    // Transform to world coordinates
    Vector4 world_pos = v4(ndc_x, ndc_y, 0, 1);
    world_pos = m4_transform(m4_inverse(proj), world_pos);
    world_pos = m4_transform(view, world_pos);
    // log("%f, %f", world_pos.x, world_pos.y);

    // Return as 2D vector
    return (Vector2){world_pos.x, world_pos.y};
}

Vector2 get_image_size(Gfx_Image *image)
{
    return (Vector2){image->width, image->height};
}

Vector4 getUvCoords(Vector2 imageSize, Vector2 cellIndex, float cellSize, int numberOfCellsOnX, int numberOfCellsOnY)
{
    // Calculate the width and height of each cell in UV space
    f32 cellWidth = cellSize / imageSize.x;
    f32 cellHeight = cellSize / imageSize.y;

    float u = fmod(cellIndex.x, (float)numberOfCellsOnX) * cellWidth;
    float v = fmod(cellIndex.y, (float)numberOfCellsOnY) * cellHeight;

    return (Vector4){
        u,
        v,
        u + cellWidth,
        v + cellHeight};
}

int world_pos_to_tile_pos(float world_pos)
{
    return floorf(world_pos / (float)SPRITE_PIXEL_SIZE);
}

float tile_pos_to_world_pos(int tile_pos)
{
    return ((float)tile_pos * (float)SPRITE_PIXEL_SIZE);
}

Vector2 round_v2_to_tile(Vector2 world_pos)
{
    world_pos.x = tile_pos_to_world_pos(world_pos_to_tile_pos(world_pos.x));
    world_pos.y = tile_pos_to_world_pos(world_pos_to_tile_pos(world_pos.y));
    return world_pos;
}