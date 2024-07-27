// !!!!!!!! BUILD CONFIG SHOULD BE DONE IN build_engine.c

#define OOGABOOGA_LINK_EXTERNAL_INSTANCE 1
#include "oogabooga/oogabooga.c"
#include "globals.c"
#include "utils.c"
#include "entity.c"
#include "ui.c"

///
///
// This is the game module which is what can be recompiled in the engine runtime

// For the engine to be able to detect a symbol, it needs to be marked with SHARED_EXPORT
void SHARED_EXPORT
game_update(f64 delta_t)
{

	float64 now = os_get_current_time_in_seconds();

	f32 zoom = (float)window.width / (float)target_render_width;
	float scaled_render_height = (float)window.height / zoom;

	float half_width = target_render_width * 0.5f;
	float half_height = scaled_render_height * 0.5f;
	// Camera
	{

		draw_frame.projection = m4_make_orthographic_projection(-half_width, half_width, -half_height, half_height, -1, 10);
		draw_frame.view = m4_make_scale(v3(1.0, 1.0, 1.0));
	}

	world_frame.world_mouse_pos = screen_to_world();
	Vector2 tile_pos = round_v2_to_tile(world_frame.world_mouse_pos);

	float half_sprite_size = SPRITE_PIXEL_SIZE / 2.0;

	int window_relative_width = window.width / zoom;
	int window_relative_height = window.height / zoom;
	int window_width_half_size_relative = window_relative_width * 0.5;
	int window_height_half_size_relative = window_relative_height * 0.5;

	float grid_horizontal_length = SPRITE_PIXEL_SIZE * TILES_X;
	float grid_vertical_length = SPRITE_PIXEL_SIZE * TILES_Y;

	float horizontal_grid_offset = (window_relative_width - grid_horizontal_length) * 0.5f;
	float vertical_grid_offset = (window_relative_height - grid_vertical_length) * 0.5f;

	int y_grid_padding = 30;

	Vector2 offset = v2(SPRITE_PIXEL_SIZE * TILES_X * 0.5, SPRITE_PIXEL_SIZE * TILES_Y * 0.5);

	Matrix4 tile_xform = m4_scalar(1.0);
	tile_xform = m4_translate(tile_xform, v3(-offset.x, -offset.y, 0.0));

	// ref: https://www.reddit.com/r/rust/comments/nfoi4j/how_can_i_create_a_2d_array/
	for (int tile_index = 0; tile_index < TILES_X * TILES_Y; tile_index++)
	{
		int y = (int)floorf((float)tile_index / (float)TILES_X);
		int x = tile_index - TILES_X * y;
		Vector4 color = v4(0.2, 0.5, 0.2, 1.0);

		enum TILES tile = Tiles[tile_index];

		if (tile == tile_wall)
		{
			color = v4(0.6, 0.6, 0.6, 1.0);
		}

		if (tile == tile_path || tile == tile_entry)
		{
			color = v4(0.6, 0.1, 0.2, 1.0);
		}

		Matrix4 xform = tile_xform;
		xform = m4_translate(xform, v3(x * SPRITE_PIXEL_SIZE, y * SPRITE_PIXEL_SIZE, 0.0));
		draw_rect_xform(xform, v2(SPRITE_PIXEL_SIZE, SPRITE_PIXEL_SIZE), color);
	}

	if (battle_data.selected_monster_index_tower_for_placement > -1)
	{
		Vector4 color = ((Vector4){0.0, 0.0, 0.3, 1.0});
		for (int x = 0; x < TILES_X; x++)
		{

			draw_line(
				v2(-window_width_half_size_relative + (x * SPRITE_PIXEL_SIZE) + horizontal_grid_offset, -window_height_half_size_relative + vertical_grid_offset),
				v2(-window_width_half_size_relative + (x * SPRITE_PIXEL_SIZE) + horizontal_grid_offset, -window_height_half_size_relative + grid_vertical_length + vertical_grid_offset),
				1,
				color);
		}

		for (int y = 0; y < TILES_Y; y++)
		{

			draw_line(
				v2(-window_width_half_size_relative + horizontal_grid_offset, -window_height_half_size_relative + (y * SPRITE_PIXEL_SIZE) + vertical_grid_offset),
				v2(-window_width_half_size_relative + grid_horizontal_length + horizontal_grid_offset, -window_height_half_size_relative + (y * SPRITE_PIXEL_SIZE) + vertical_grid_offset),
				1,
				color);
		}

		draw_line(
			v2(-window_width_half_size_relative + (TILES_X * SPRITE_PIXEL_SIZE) + horizontal_grid_offset, -window_height_half_size_relative + vertical_grid_offset),
			v2(-window_width_half_size_relative + (TILES_X * SPRITE_PIXEL_SIZE) + horizontal_grid_offset, -window_height_half_size_relative + grid_vertical_length + vertical_grid_offset),
			1,
			color);

		draw_line(
			v2(-window_width_half_size_relative + horizontal_grid_offset, -window_height_half_size_relative + (TILES_Y * SPRITE_PIXEL_SIZE) + vertical_grid_offset),
			v2(-window_width_half_size_relative + grid_horizontal_length + horizontal_grid_offset, -window_height_half_size_relative + (TILES_Y * SPRITE_PIXEL_SIZE) + vertical_grid_offset),
			1,
			color);
	}
	{
		Matrix4 xform = m4_translate(tile_xform, v3(battle_data.health_placement.x * SPRITE_PIXEL_SIZE, battle_data.health_placement.y * SPRITE_PIXEL_SIZE, 0.0));
		xform = m4_translate(xform, v3(SPRITE_PIXEL_SIZE * 0.5, SPRITE_PIXEL_SIZE * 0.5, 0.0));
		draw_image_xform(heart_sprite, xform, get_image_size(heart_sprite), COLOR_WHITE);
	}

	bool can_place_tower = false;

	if (battle_data.selected_monster_index_tower_for_placement > -1)
	{
		Vector4 rect_color = COLOR_RED;
		rect_color.a = 0.25f;

		Monster mon = run_data.current_party_members[battle_data.selected_monster_index_tower_for_placement];
		// SpriteCell cell = sprite_cells[tower.monster_type];
		Matrix4 xform = m4_scalar(1.0);
		int half_tile_x = TILES_X * 0.5f;
		int half_tile_y = TILES_Y * 0.5f;

		int tile_index_x = world_pos_to_tile_pos(tile_pos.x);
		int tile_index_y = world_pos_to_tile_pos(tile_pos.y);

		Vector4 sprite_color = COLOR_WHITE;

		enum Tile tile = TILES[tiles]

			if (tile_index_x >= -half_tile_x && tile_index_x < half_tile_x && tile_index_y >= -half_tile_y && tile_index_y < half_tile_y)
		{
			xform = m4_translate(xform, v3(tile_pos.x, tile_pos.y, 0));
			can_place_tower = true;
			sprite_color.a = 0.9;
			rect_color = COLOR_GREEN;
			rect_color.a = 0.25f;
		}
		else
		{
			xform = m4_translate(xform, v3(world_frame.world_mouse_pos.x, world_frame.world_mouse_pos.y, 0.0));
			xform = m4_translate(xform, v3(SPRITE_PIXEL_SIZE * -0.5, SPRITE_PIXEL_SIZE * -0.5, 0.0));
			sprite_color.a = 0.5;
		}

		draw_rect_xform(xform, v2(SPRITE_PIXEL_SIZE, SPRITE_PIXEL_SIZE), rect_color);

		Draw_Quad *quad = draw_image_xform(sprites, xform, v2(SPRITE_PIXEL_SIZE, SPRITE_PIXEL_SIZE), sprite_color);
		quad->uv = getUvCoords(sprite_size, v2(mon.mon_type, 0), SPRITE_PIXEL_SIZE, 6, 1);
	}

	// Do stuff with towers
	for (u8 i = 0; i < MAX_PLAYER_TOWERS; i++)
	{
		render_entity(&player_towers[i], sprites, sprite_size);
	}

	for (u8 i = 0; i < MAX_ENEMIES; i++)
	{
		render_entity(&enemies[i], sprites, sprite_size);
	}

	//: DRAW UI

	// Reset Camera for UI
	{

		float32 aspect = (float32)window.width / (float32)window.height;
		draw_frame.projection = m4_make_orthographic_projection(window.pixel_width * -0.5, window.pixel_width * 0.5, window.pixel_height * -0.5, window.pixel_height * 0.5, -1, 10);
		draw_frame.view = m4_scalar(1.0);

		world_frame.ui_mouse_pos = screen_to_world();
	}

	// Player Info
	{
		string unit_str = STR("Units: %d / %d");
		string unit_str_formatted = sprint(get_temporary_allocator(), unit_str, count_active_towers(), MAX_PLAYER_TOWERS);
		float text_padding = 15;
		int font_height = 30;

		// To align/justify text we need to measure it.
		Gfx_Text_Metrics unit_str_metrics = measure_text(font, unit_str_formatted, font_height, v2(1, 1));

		// This is where we want the bottom left of the text to be...
		Vector2 bottom_left = v2(-window.width / 2 + 20, -window.height / 2 + 20);

		// ... So we have to justify that bottom_left according to text metrics
		Vector2 justified = v2_sub(bottom_left, unit_str_metrics.functional_pos_min);

		draw_text(font, unit_str_formatted, font_height, justified, v2(1, 1), COLOR_WHITE);

		string mana_str = STR("Mana: %d / %d");
		string mana_str_formatted = sprint(get_temporary_allocator(), mana_str, battle_data.current_mana, run_data.max_mana);

		Gfx_Text_Metrics mana_str_metrics = measure_text(font, mana_str_formatted, font_height, v2(1, 1));

		Vector2 justified_mana_text_pos = v2_add(justified, v2(0, mana_str_metrics.visual_size.y + text_padding));

		draw_text(font, mana_str_formatted, font_height, justified_mana_text_pos, v2(1, 1), COLOR_WHITE);
	}

	Matrix4 xform = m4_scalar(1.0);
	xform = m4_translate(xform, v3(window.pixel_width * 0.5f - 120.0, -window.pixel_height * 0.5 + 20.0, 0.0));
	if (draw_btn(xform, font, STR("PLAY")))
	{
		for (int i = 0; i < 3; i++)
		{
			Entity *ent = create_enemy();
			ent->monster_type = get_random_int_in_range(0, MONSTER_MAX - 1);
			ent->health = 3;
			ent->max_health = 3;
			ent->speed = 5;
			ent->position = v2(tile_pos_to_world_pos(9), tile_pos_to_world_pos(get_random_int_in_range(-TILES_Y * 0.5, TILES_Y * 0.5 - 1)));
		}
	}

	// Draw Tower Selection Bar
	{

		Vector2 ui_mouse_pos_world = screen_to_world();
		float offset_x = 400;
		Vector2 bar_start_pos = v2(window.pixel_width * -0.5 + offset_x, window.pixel_height * -0.5 + 10);
		draw_rect(bar_start_pos, v2(window.pixel_width - offset_x * 2, 50), COLOR_WHITE);
		Vector2 tower_btn_start = v2_add(bar_start_pos, v2(25, 25));
		float btn_offset_amount = 10;
		for (int i = 0; i < PARTY_MEMBER_LIMIT; i++)
		{

			Matrix4 btn_xform = m4_scalar(1.0);

			Vector2 offset = v2(i * (TOWER_BTN_WIDTH + btn_offset_amount), 0);
			Vector2 btn_pos = v2_add(tower_btn_start, offset);
			btn_xform = m4_translate(btn_xform, v3(btn_pos.x, btn_pos.y, 0));
			Monster monster = run_data.current_party_members[i];
			if (draw_tower_button(btn_xform))
			{
				if (monster.active)
				{
					battle_data.selected_monster_index_tower_for_placement = i;
				}
			}

			if (monster.active)
			{
				Matrix4 xform = m4_scalar(1.0);
				xform = m4_translate(xform, v3(btn_pos.x, btn_pos.y, 0));
				xform = m4_translate(xform, v3(SPRITE_PIXEL_SIZE * 0.5, SPRITE_PIXEL_SIZE * 0.5, 0));
				xform = m4_scale(xform, v3(3.0, 3.0, 1.0));

				Draw_Quad *quad = draw_image_xform(sprites, xform, v2(SPRITE_PIXEL_SIZE, SPRITE_PIXEL_SIZE), COLOR_WHITE);

				quad->uv = getUvCoords(sprite_size, v2(monster.mon_type, 0), SPRITE_PIXEL_SIZE, 6, 1);
				string mana_str = STR("%d");
				string mana_str_formatted = sprint(get_temporary_allocator(), mana_str, monster.mana_cost);
				btn_xform = m4_translate(btn_xform, v3(5., 5., 0.));
				draw_text_xform(font, mana_str_formatted, 30, btn_xform, v2(1.0, 1.0), COLOR_WHITE);
			}
		}
	}

	// LOGIC for tower placement, needs to be after the buttons so it doesn't conflict with input capturing
	// E.g if you press the button, don't place a tower
	// @TODO: the buttons shouldn't overlap the grid, so this should be back above the draw logic?
	if (is_key_just_pressed(KEY_ESCAPE))
	{
		consume_key_just_pressed(KEY_ESCAPE);
		battle_data.selected_monster_index_tower_for_placement = -1;
	}

	if (is_key_just_released(MOUSE_BUTTON_LEFT))
	{
		consume_key_just_released(MOUSE_BUTTON_LEFT);

		if (can_place_tower)
		{
			Entity *tower = create_tower();
			tower->monster_type = run_data.current_party_members[battle_data.selected_monster_index_tower_for_placement].mon_type;
			tower->position = tile_pos;

			int mana_cost = get_monster_mana_cost(tower->monster_type);
			battle_data.current_mana -= mana_cost;
		}

		battle_data.selected_monster_index_tower_for_placement = -1;
	}

	if (is_key_just_released('E'))
	{
		log("FPS: %.2f", 1.0 / delta_t);
		log("ms: %.2f", delta_t * 1000.0);
	}
}