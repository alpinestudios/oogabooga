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

fn Vector4 get_uv_coords(Vector2 image_size, Vector2 cell_index, float cell_size, Vector2 cell_count)
{
    // Calculate the width and height of each cell in UV space
    float cell_width = cell_size / image_size.x;
    float cell_height = cell_size / image_size.y;

    float u = (float)engine::fmod(cell_index.x, cell_count.x) * cell_width;
    float v = 1.0f - ((float)engine::fmod(cell_index.y, cell_count.y) + 1) * cell_height;

    return {{
        u,
        v,
        u + cell_width,
        v + cell_height
    }};
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

def SPRITE_PIXEL_SIZE = engine::SPRITE_PIXEL_SIZE;
fn engine::Matrix4 flip_sprite_x(engine::Matrix4 xform) {
    xform = engine::m4_translate(xform, {{SPRITE_PIXEL_SIZE / 2, SPRITE_PIXEL_SIZE / 2, 0}});
    xform = engine::m4_scale(xform, {{ -1, 1, 1 }});
    xform = engine::m4_translate(xform, {{-SPRITE_PIXEL_SIZE / 2, -SPRITE_PIXEL_SIZE / 2, 0}});

    return xform;
}


fn bool point_inside_rect(Vector2 min, Vector2 max, Vector2 p) {
  return p.x >= min.x && p.x <= max.x && p.y >= min.y && p.y <= max.y;
}


// Rect min is the bottom left corner
fn bool rect_circle_collision(Vector2 rect_min, Vector2 size, Vector2 circle_center, float radius) {
    float dist_x = $$abs(circle_center.x - (rect_min.x + size.x / 2));
    float dist_y = $$abs(circle_center.y - (rect_min.y + size.y / 2));

    if (dist_x > (size.x / 2 + radius)) { return false; }
    if (dist_y > (size.y / 2 + radius)) { return false; }

    if (dist_x <= (size.x / 2)) { return true; }
    if (dist_y <= (size.y / 2)) { return true; }

    float dx = dist_x - size.x / 2;
    float dy = dist_y - size.y / 2;
    return (dx * dx + dy * dy <= (radius * radius));
}


fn bool line_circle_collision(Vector2 p1, Vector2 p2, Vector2 circle_center, float radius) {
    // Vector from p1 to p2
    Vector2 d = {{ p2.x - p1.x, p2.y - p1.y }};
    
    // Vector from p1 to the circle center
    Vector2 f = {{ p1.x - circle_center.x, p1.y - circle_center.y }};
    
    float a = d.x * d.x + d.y * d.y;
    float b = 2 * (f.x * d.x + f.y * d.y);
    float c = f.x * f.x + f.y * f.y - radius * radius;

    float discriminant = b * b - 4 * a * c;
    
    // If discriminant is negative, no real roots, the line does not intersect the circle
    if (discriminant < 0) {
        return false;
    }

    discriminant = $$sqrt(discriminant);

    // Find the points of intersection, t1 and t2 are the parameter values
    float t1 = (-b - discriminant) / (2 * a);
    float t2 = (-b + discriminant) / (2 * a);

    // Check if the intersection points are within the segment
    if (t1 >= 0 && t1 <= 1) {
        return true;
    }
    if (t2 >= 0 && t2 <= 1) {
        return true;
    }

    return false;
}


import std::io;
fn bool circles_overlap(Vector2 center1, float radius1, Vector2 center2, float radius2) {
    // Calculate the distance between the centers of the two circles
    float distance = center1.distance(center2);
    // Check if the distance is less than or equal to the sum of the radii
    if (distance <= (radius1 + radius2)) {
        return true;
    }

    return false;
}