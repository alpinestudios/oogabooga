inline void player_input(Vector2* input_axes)
{
    if (is_key_down('A'))
        input_axes->x -= 1.0;
    if (is_key_down('D'))
        input_axes->x += 1.0;
    if (is_key_down('S'))
        input_axes->y -= 1.0;
    if (is_key_down('W'))
        input_axes->y += 1.0;
}