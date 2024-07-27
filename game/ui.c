#define TOWER_BTN_WIDTH 60

bool point_contains_rect(Vector2 point, Vector2 min, Vector2 max)
{
    return point.x >= min.x && point.x <= max.x && point.y >= min.y && point.y <= max.y;
}

enum ButtonState
{
    btn_nil,
    btn_hover,
    btn_down,
    btn_just_pressed,
};

enum ButtonState
button(Vector4 button_size)
{
    enum ButtonState btn_state = btn_nil;
    if (point_contains_rect(world_frame.ui_mouse_pos, v2(button_size.x, button_size.y), v2(button_size.z, button_size.w)))
    {
        btn_state = btn_hover;
    }

    if (btn_state == btn_hover && is_key_down(MOUSE_BUTTON_LEFT))
    {
        // consume_key_down(MOUSE_BUTTON_LEFT);
        btn_state = btn_down;
    }

    // There's a slight problem here, we kinda need to know if the button was down last frame but for now we will just used hover
    if (btn_state == btn_hover && is_key_just_released(MOUSE_BUTTON_LEFT))
    {
        consume_key_just_released(MOUSE_BUTTON_LEFT);
        btn_state = btn_just_pressed;
    }

    return btn_state;
}

bool draw_tower_button(Matrix4 xform)
{

    Vector2 min = get_xform_position(xform);
    Vector2 size = v2(TOWER_BTN_WIDTH, TOWER_BTN_WIDTH);
    Vector2 max = v2_add(min, size);

    enum ButtonState btn_state = button(v4(min.x, min.y, max.x, max.y));

    Vector4 color = COLOR_RED;

    switch (btn_state)
    {
    case btn_hover:
        color.a = 0.5f;
        break;
    case btn_down:
        color = COLOR_GREEN;
        color.a = 0.1f;
        break;
    case btn_just_pressed:
        color = COLOR_GREEN;
        log("PRESSED");
        break;
    default:
        break;
    }

    draw_rect_xform(xform, size, color);
    return btn_state == btn_just_pressed;
}

bool draw_btn(Matrix4 xform, Gfx_Font *font, string text)
{

    Vector2 min = get_xform_position(xform);
    Vector2 size = v2(100, 50);
    Vector2 max = v2_add(min, size);

    enum ButtonState btn_state = button(v4(min.x, min.y, max.x, max.y));

    Vector4 color = v4(0.5, 0.5, 0.5, 1.0);

    switch (btn_state)
    {
    case btn_hover:
        color.a = 0.5f;
        break;
    case btn_down:
        color = COLOR_GREEN;
        color.a = 0.1f;
        break;
    case btn_just_pressed:
        color = COLOR_GREEN;
        log("PRESSED");
        break;
    default:
        break;
    }

    draw_rect_xform(xform, size, color);
    Gfx_Text_Metrics metrics = measure_text(font, text, 30, V2_ONE);
    xform = m4_translate(xform, v3(size.x * 0.5 - metrics.visual_size.x * 0.5, size.y * 0.5 - metrics.visual_size.y * 0.5, 0.0));

    draw_text_xform(font, STR("PLAY"), 30, xform, V2_ONE, COLOR_WHITE);
    // log("btn state: %d", btn_state);
    return btn_state == btn_just_pressed;
}