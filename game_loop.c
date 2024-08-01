// !!!!!!!! BUILD CONFIG SHOULD BE DONE IN build_engine.c

#ifdef OOGABOOGA_LINK_EXTERNAL_INSTANCE
    #pragma message("OOGABOOGA_LINK_EXTERNAL_INSTANCE is defined")
    #include "oogabooga/oogabooga.c"
    #include "game/utils.c"
    #include "game/entity.c"
	#include "game/weapons.c"
    #include "game/globals.c"
    #include "game/ui.c"
#endif

///
///
// This is the game module which is what can be recompiled in the engine runtime

// For the engine to be able to detect a symbol, it needs to be marked with SHARED_EXPORT
void
#ifdef OOGABOOGA_LINK_EXTERNAL_INSTANCE
	SHARED_EXPORT
#endif
game_update(f64 delta_t, Game *game)
{

	float64 now = os_get_current_time_in_seconds();

	int target_render_width = 320;
	f32 zoom = (float)window.width / (float)target_render_width;
	float scaled_render_height = (float)window.height / zoom;

	float half_width = target_render_width * 0.5f;
	float half_height = scaled_render_height * 0.5f;

	animate_v2_to_target(&game->camera_pos, game->player_entity.position, delta_t, 10.0f);
	// Camera
	{
		draw_frame.projection = m4_make_orthographic_projection(-half_width, half_width, -half_height, half_height, -1, 10);
		draw_frame.view = m4_make_scale(v3(1.0, 1.0, 1.0));
		draw_frame.view = m4_mul(draw_frame.view, m4_make_translation(v3(game->camera_pos.x, game->camera_pos.y, 0)));
	}

	world_frame.world_mouse_pos = screen_to_world();


	Vector2 player_input = V2_ZERO;
	if (is_key_down(KEY_ARROW_UP) || is_key_down('W'))
	{
		player_input.y += 1;
	}

	if (is_key_down(KEY_ARROW_DOWN) || is_key_down('S'))
	{
		player_input.y -= 1;
	}

	if (is_key_down(KEY_ARROW_RIGHT) || is_key_down('D'))
	{
		player_input.x += 1;
	}

	if (is_key_down(KEY_ARROW_LEFT) || is_key_down('A'))
	{
		player_input.x -= 1;
	}


	Vector2 room_size = v2(272, 156);
	f32 room_left_x = -(room_size.x / 2.0);
	f32 room_left_y = -(room_size.y / 2.0);

	f32 room_right_x = (room_size.x / 2.0) - 16;
	f32 room_right_y = (room_size.y / 2.0) - 6;

	player_input = v2_normalize(player_input);

	game->player_entity.flip_x = world_frame.world_mouse_pos.x < game->player_entity.position.x + SPRITE_PIXEL_SIZE * 0.5f;

	Vector2 potential_pos = v2_add(game->player_entity.position, v2_mulf(player_input, game->player_entity.speed * delta_t));
	potential_pos.x = clamp(potential_pos.x, room_left_x, room_right_x);
	potential_pos.y = clamp(potential_pos.y, room_left_y, room_right_y);
	game->player_entity.position = potential_pos;


	Matrix4 walls_xform = m4_scalar(1.0);
	Vector2 walls_size = get_image_size(game->walls);
	walls_xform = m4_translate(walls_xform, v3(-walls_size.x * 0.5, -walls_size.y * 0.5, 0.0));
	draw_image_xform(game->walls, walls_xform, walls_size, COLOR_WHITE);

	Matrix4 player_xform = render_player(&game->player_entity, game->player_sprites);
	render_player_weapons(&game->player_entity, player_xform, game->weapon_sprites, delta_t);
	if (is_key_just_released('E'))
	{
		log("FPS: %.2f", 1.0 / delta_t);
		log("ms: %.2f", delta_t * 1000.0);
	}
}
