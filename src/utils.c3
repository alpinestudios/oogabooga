module utils;
import engine;
import std::math;



def Vector2 = engine::Vector2;
def Vector3 = engine::Vector3;
def Vector4 = engine::Vector4;
def Matrix4 = engine::Matrix4;
def m4_translate = engine::m4_translate;
def m4_transform = engine::m4_transform;

fn bool almost_equals(float a, float b, float epsilon)
{
    return engine::fabs((double)a - (double)b) <= (double)epsilon;
}

fn bool animate_f32_to_target(float *value, float target, float delta_t, float rate)
{
    *value += (target - *value) * (1 - $$pow(2.0f, -rate * delta_t));
    if (almost_equals(*value, target, 0.001f))
    {
        *value = target;
        return true; // reached
    }
    return false;
}

fn void animate_v2_to_target(Vector2 *value, Vector2 target, double delta_t, float rate)
{
    animate_f32_to_target(&(value.x), target.x, (float)delta_t, rate);
    animate_f32_to_target(&(value.y), target.y, (float)delta_t, rate);
}

fn Vector2 get_image_size(GfxImage *image)
{
    return Vector2 {{image.width, image.height}};
}

fn Vector4 get_uv_coords(Vector2 imageSize, Vector2 cellIndex, float cellSize, int numberOfCellsOnX, int numberOfCellsOnY)
{
    // Calculate the width and height of each cell in UV space
    float cellWidth = cellSize / imageSize.x;
    float cellHeight = cellSize / imageSize.y;

    float u = (float)engine::fmod(cellIndex.x, numberOfCellsOnX) * cellWidth;
    float v = (float)engine::fmod(cellIndex.y, numberOfCellsOnY) * cellHeight;

    return {{
        u,
        v,
        u + cellWidth,
        v + cellHeight}};
}

fn float calc_rotation_to_target(Vector2 a, Vector2 b) {
	float delta_x = a.x - b.x;
	float delta_y = a.y - b.y;
	float angle = math::atan2(delta_y, delta_x);
	return angle;
}


def window = engine::window;
def draw_frame = engine::draw_frame;

fn Vector2 screen_to_world()
{
    float mouse_x = engine::input_frame.mouse_x;
    float mouse_y = engine::input_frame.mouse_y;
    Matrix4 proj = draw_frame.projection;
    Matrix4 view = draw_frame.view;

    // Normalize the mouse coordinates
    float ndc_x = (mouse_x / (window.width * 0.5f)) - 1.0f;
    float ndc_y = (mouse_y / (window.height * 0.5f)) - 1.0f;

    // Transform to world coordinates
    Vector4 world_pos = {{ ndc_x, ndc_y, 0, 1 }};
    world_pos = engine::m4_transform(engine::m4_inverse(proj), world_pos);
    world_pos = engine::m4_transform(view, world_pos);
    // log("%f, %f", world_pos.x, world_pos.y);

    // Return as 2D vector
    return {{ world_pos.x, world_pos.y }};
}


fn float rad_to_deg(float radians)
{
    return radians * (180.0f / math::PI);
}

fn float deg_to_rad(float deg)
{
    return deg * (math::PI / 180.0f);
}

