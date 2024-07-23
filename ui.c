#define TOWER_BTN_WIDTH 60

bool point_contains_rect(Vector2 point, Vector2 min, Vector2 max)
{
    return point.x >= min.x && point.x <= max.x && point.y >= min.y && point.y <= max.y;
}

bool draw_tower_button(Vector2 position)
{
    bool pressed = false;
    bool hover = false;
    bool down = false;

    Vector2 min = position;
    Vector2 size = v2(TOWER_BTN_WIDTH, TOWER_BTN_WIDTH);
    Vector2 max = v2_add(min, size);

    if (point_contains_rect(world_frame.ui_mouse_pos, min, max))
    {
        hover = true;
    }

    if (hover && is_key_down(MOUSE_BUTTON_LEFT))
    {
        // consume_key_down(MOUSE_BUTTON_LEFT);
        down = true;
    }

    if (hover && is_key_just_released(MOUSE_BUTTON_LEFT))
    {
        consume_key_just_released(MOUSE_BUTTON_LEFT);

        pressed = true;
    }

    Vector4 color = COLOR_RED;

    if (hover)
    {
        color.a = 0.5f;
    }

    if (down)
    {
        color = COLOR_GREEN;
        color.a = 0.1f;
    }

    if (pressed)
    {
        color = COLOR_GREEN;
    }

    draw_rect(position, size, color);
    return pressed;
}