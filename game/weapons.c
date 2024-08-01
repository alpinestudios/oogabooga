

f32 rad_to_deg(f32 radians)
{
    return radians * (180.0 / M_PI);
}

const f32 deg_to_rad(f32 deg)
{
    return deg * (M_PI / 180.0);
}


bool attack_down = false;
f32 offset_angle = 0.0;
f32 offset_angle_bump_amount = 5 * (M_PI / 180.0);
void calculate_rotation_offset(f32 target_rotation, f32 delta) {

    f32 bump = 0.0f;

    if (attack_down) {
        bump = offset_angle_bump_amount;
    }

    offset_angle = slerp(target_rotation, target_rotation + bump, delta * 10);
}

void calculate_rotation_animated(f32 target_rotation, f32 delta)
{

    if (is_key_just_pressed(MOUSE_BUTTON_LEFT))
    {
        attack_down = !attack_down;
    }


    calculate_rotation_offset(target_rotation, delta);
    log("offset: %f, delta: %f", offset_angle, delta);
}


f32 calc_rotation_to_target(Vector2 a, Vector2 b) {
	f32 delta_x = a.x - b.x;
	f32 delta_y = a.y - b.y;
	f32 angle = atan2(delta_y, delta_x);
	return angle;
}



void render_player_weapons(Entity *player, Matrix4 player_xform, Gfx_Image *weapons_sprite, f32 delta)
{
    Vector2 sprite_size = get_image_size(weapons_sprite);
    if (player->right_hand_weapon != weapon_nil)
    {

        Matrix4 xform = player_xform;
        xform = m4_translate(xform, v3(SPRITE_PIXEL_SIZE * 0.5, SPRITE_PIXEL_SIZE * 0.5, 0.0));
        bool flip_y = world_frame.world_mouse_pos.y < player->position.y && player->right_hand_weapon != weapon_shield;

        f32 x_scale = 1.0;
        f32 y_scale = 1.0;

        if (flip_y)
        {
            y_scale = -1.0;
        }

        if (player->flip_x)
        {
            x_scale = -1.0;
        }

        xform = m4_scale(xform, v3(x_scale, y_scale, 1));

        if (!player->flip_x)
        {
           
            f32 angle_to_target = calc_rotation_to_target(world_frame.world_mouse_pos, player->position);
            calculate_rotation_animated(angle_to_target, delta);
            f32 angle = offset_angle;
            if (flip_y)
            {
                angle = -angle;
            }
            xform = m4_rotate_z(xform, -angle);
            draw_circle_xform(xform, V2_ONE, COLOR_RED);

            if (player->right_hand_weapon == weapon_shield)
            {
                xform = m4_translate(xform, v3(0, -SPRITE_PIXEL_SIZE * 0.5, 0.0));
            }
        }
        else
        {
            f32 angle_to_target = calc_rotation_to_target(v2(player->position.x + SPRITE_PIXEL_SIZE * 0.5f, player->position.y), world_frame.world_mouse_pos);
            if (flip_y)
            {
                angle_to_target = -angle_to_target;
            }

            xform = m4_rotate_z(xform, angle_to_target);

            if (player->right_hand_weapon == weapon_shield)
            {
                xform = m4_translate(xform, v3(0, -SPRITE_PIXEL_SIZE * 0.5, 0.0));
            }
        }

        Draw_Quad *quad = draw_image_xform(weapons_sprite, xform, v2(SPRITE_PIXEL_SIZE, SPRITE_PIXEL_SIZE), COLOR_WHITE);
        quad->uv = getUvCoords(sprite_size, v2(player->right_hand_weapon, 0), SPRITE_PIXEL_SIZE, 3, 1);
    }

    if (player->left_hand_weapon != weapon_nil)
    {
        Matrix4 xform = player_xform;
        if (!player->flip_x)
        {
            xform = m4_translate(xform, v3(-4, 0, 0));
            xform = m4_scale(xform, v3(-1, 1, 1));
            xform = m4_translate(xform, v3(-SPRITE_PIXEL_SIZE, 0, 0));
        }
        else
        {
            xform = m4_translate(xform, v3(4, 0, 0));
        }

        Draw_Quad *quad = draw_image_xform(weapons_sprite, xform, v2(SPRITE_PIXEL_SIZE, SPRITE_PIXEL_SIZE), COLOR_WHITE);
        quad->uv = getUvCoords(sprite_size, v2(player->left_hand_weapon, 0), SPRITE_PIXEL_SIZE, 3, 1);
    }
}