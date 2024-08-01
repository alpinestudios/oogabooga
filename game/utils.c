#define SPRITE_PIXEL_SIZE 16


#define V2_ZERO ((Vector2){0.0, 0.0})
#define V2_ONE ((Vector2){1.0, 1.0})
#define V2_RIGHT ((Vector2){1.0, 0.0})
#define V2_LEFT ((Vector2){-1.0, 0.0})
#define V2_UP ((Vector2){0.0, 1.0})
#define V2_DOWN ((Vector2){0.0, -1.0})

#define V3_ZERO ((Vector3){0.0, 0.0, 0.0})

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

Vector2 get_xform_position(Matrix4 xform)
{
    return v2(xform.m[0][3], xform.m[1][3]);
}

// 0 -> 1
float sin_breathe(float time, float rate)
{
    return (sin(time * rate) + 1.0) / 2.0;
}

bool almost_equals(float a, float b, float epsilon)
{
    return fabs(a - b) <= epsilon;
}

bool animate_f32_to_target(float *value, float target, float delta_t, float rate)
{
    *value += (target - *value) * (1.0 - pow(2.0f, -rate * delta_t));
    if (almost_equals(*value, target, 0.001f))
    {
        *value = target;
        return true; // reached
    }
    return false;
}

bool animate_v2_to_target(Vector2 *value, Vector2 target, float delta_t, float rate)
{
    animate_f32_to_target(&(value->x), target.x, delta_t, rate);
    animate_f32_to_target(&(value->y), target.y, delta_t, rate);
}

Range2f quad_to_range(Draw_Quad quad)
{
    return (Range2f){quad.bottom_left, quad.top_right};
}

int is_char_a_digit(char c)
{
    return c >= '0' && c <= '9';
}

float slerp(float start, float end, float t) {
    // Normalize the angles
    float difference = fmod(end - start + M_PI, 2.0 * M_PI) - M_PI;
    if (difference < -M_PI) {
        difference += 2.0 * M_PI;
    }

    return start + t * difference;
}