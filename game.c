#define PARTY_MEMBER_LIMIT 6

typedef struct Monster
{
    bool active;
    s32 level;
    s32 current_xp;
    s32 max_hp;
    s32 mana_cost;
    enum MonsterType mon_type;
} Monster;

typedef struct BattleData
{
    s32 current_mana;
    s32 selected_monster_index_tower_for_placement;
} BattleData;

typedef struct RunData
{
    s32 max_mana;
    Monster current_party_members[PARTY_MEMBER_LIMIT];
} RunData;

int entry(int argc, char **argv)
{

    Gfx_Font *font = load_font_from_disk(STR("C:/windows/fonts/arial.ttf"), get_heap_allocator());
    assert(font, "Failed loading arial.ttf");
    window.title = STR("Tower defense game");
    window.scaled_width = 1280; // We need to set the scaled size if we want to handle system scaling (DPI)
    window.scaled_height = 800;
    window.x = 200;
    window.y = 200;
    window.clear_color = hex_to_rgba(0x11191cff);
    int target_render_width = 320;

    Gfx_Image *sprites = load_image_from_disk(STR("assets/spritesheet.png"), get_heap_allocator());
    assert(sprites, "didnt load image");

    float64 last_time = os_get_current_time_in_seconds();

    Vector2 sprite_size = get_image_size(sprites);

    f32 zoom = 4.0;
    f32 dbl_zoom = zoom * 2.0;

    RunData run_data = {0};
    run_data.max_mana = 15;
    seed_for_random = os_get_current_cycle_count();
    run_data.current_party_members[0].mon_type = get_random_int_in_range(0, MONSTER_MAX - 1);

    run_data.current_party_members[0]
        .active = true;
    BattleData battle_data = {0};
    battle_data.selected_monster_index_tower_for_placement = -1;
    battle_data.current_mana = run_data.max_mana;

    const float64 fps_limit = 69000;
    const float64 min_frametime = 1.0 / fps_limit;

    int tilesX = (window.width / zoom) / SPRITE_PIXEL_SIZE;
    int tilesY = (window.height / zoom) / SPRITE_PIXEL_SIZE;

    while (!window.should_close)
    {
        reset_temporary_storage();
        reset_world_frame();

        float64 now = os_get_current_time_in_seconds();
        float64 delta_t = now - last_time;

        if (delta_t < min_frametime)
        {
            // os_high_precision_sleep((min_frametime - delta_t) * 1000.0);
            now = os_get_current_time_in_seconds();
            delta_t = now - last_time;
        }
        last_time = now;

        os_update();

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

        if (is_key_just_pressed(MOUSE_BUTTON_LEFT) && battle_data.selected_monster_index_tower_for_placement > -1)
        {
            Entity *tower = create_tower();
            tower->monster_type = run_data.current_party_members[0].mon_type;
            tower->position = tile_pos;

            int mana_cost = get_monster_mana_cost(tower->monster_type);
            battle_data.current_mana -= mana_cost;
            battle_data.selected_monster_index_tower_for_placement = -1;
        }

        Vector4 color = ((Vector4){0.0, 0.0, 0.5, 1.0});
        float half_sprite_size = SPRITE_PIXEL_SIZE / 2.0;

        int grid_x_half_size = window.width / zoom * 0.5;
        int grid_y_half_size = window.height / zoom * 0.5;

        float start_x = -grid_x_half_size;
        for (int x = 0; x < tilesX; x++)
        {

            draw_line(
                v2(start_x + (x * SPRITE_PIXEL_SIZE), -grid_y_half_size),
                v2(start_x + (x * SPRITE_PIXEL_SIZE), grid_y_half_size),
                0.5,
                color);
        }

        float start_y = -grid_y_half_size;

        draw_rect(v2(-grid_x_half_size, start_y), v2(SPRITE_PIXEL_SIZE, SPRITE_PIXEL_SIZE), COLOR_GREEN);

        for (int y = 0; y < tilesY; y++)
        {
            draw_line(
                v2(-grid_x_half_size, start_y + (y * SPRITE_PIXEL_SIZE)),
                v2(grid_x_half_size, start_y + (y * SPRITE_PIXEL_SIZE)),
                0.5,
                color);
        }

        if (battle_data.selected_monster_index_tower_for_placement > -1)
        {

            draw_rect(tile_pos, v2(SPRITE_PIXEL_SIZE, SPRITE_PIXEL_SIZE), COLOR_RED);
            Monster mon = run_data.current_party_members[battle_data.selected_monster_index_tower_for_placement];
            // SpriteCell cell = sprite_cells[tower.monster_type];
            Matrix4 xform = m4_scalar(1.0);
            xform = m4_translate(xform, v3(tile_pos.x, tile_pos.y, 0));
            // xform = m4_translate(xform, v3(SPRITE_PIXEL_SIZE * -0.5, SPRITE_PIXEL_SIZE * -0.5, 0));
            Draw_Quad *quad = draw_image_xform(sprites, xform, v2(SPRITE_PIXEL_SIZE, SPRITE_PIXEL_SIZE), COLOR_WHITE);

            quad->uv = getUvCoords(sprite_size, v2(mon.mon_type, 0), SPRITE_PIXEL_SIZE, 6, 1);
        }

        // Do stuff with towers
        for (u8 i = 0; i < MAX_PLAYER_TOWERS; i++)
        {
            if (player_towers[i].active)
            {
                Entity tower = player_towers[i];
                // SpriteCell cell = sprite_cells[tower.monster_type];
                Matrix4 xform = m4_scalar(1.0);
                xform = m4_translate(xform, v3(tower.position.x, tower.position.y, 0));
                // xform = m4_translate(xform, v3(SPRITE_PIXEL_SIZE * -0.5, SPRITE_PIXEL_SIZE * -0.5, 0));
                Draw_Quad *quad = draw_image_xform(sprites, xform, v2(SPRITE_PIXEL_SIZE, SPRITE_PIXEL_SIZE), COLOR_WHITE);

                quad->uv = getUvCoords(sprite_size, v2(tower.monster_type, 0), SPRITE_PIXEL_SIZE, 6, 1);
            }
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
            string unit_str_formatted = sprint(temp, unit_str, count_active_towers(), MAX_PLAYER_TOWERS);
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
            string mana_str_formatted = sprint(temp, mana_str, battle_data.current_mana, run_data.max_mana);

            Gfx_Text_Metrics mana_str_metrics = measure_text(font, mana_str_formatted, font_height, v2(1, 1));

            Vector2 justified_mana_text_pos = v2_add(justified, v2(0, mana_str_metrics.visual_size.y + text_padding));

            draw_text(font, mana_str_formatted, font_height, justified_mana_text_pos, v2(1, 1), COLOR_WHITE);
        }

        // Draw Tower Selection Bar
        {
            Vector2 ui_mouse_pos_world = screen_to_world();
            float offset_x = 400;
            Vector2 bar_start_pos = v2(window.pixel_width * -0.5 + offset_x, window.pixel_height * -0.5 + 40);
            draw_rect(bar_start_pos, v2(window.pixel_width - offset_x * 2, 80), COLOR_WHITE);
            Vector2 tower_btn_start = v2_add(bar_start_pos, v2(25, 25));
            float btn_offset_amount = 10;
            for (int i = 0; i < PARTY_MEMBER_LIMIT; i++)
            {
                Vector2 offset = v2(i * (TOWER_BTN_WIDTH + btn_offset_amount), 0);
                Vector2 btn_pos = v2_add(tower_btn_start, offset);
                Monster monster = run_data.current_party_members[i];
                if (draw_tower_button(btn_pos))
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
                }
            }
        }

        gfx_update();

        if (is_key_just_released('E'))
        {
            log("FPS: %.2f", 1.0 / delta_t);
            log("ms: %.2f", delta_t * 1000.0);
        }
    }

    return 0;
}