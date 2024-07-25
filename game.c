#define PARTY_MEMBER_LIMIT 6
#define TILES_X 18
#define TILES_Y 10

enum TILES
{
    tile_nil = 0,
    tile_wall = 1,
    tile_path = 2,
    tile_entry = 3,
    tile_heart = 4,
};

enum TILES tiles[TILES_X * TILES_Y];

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
    Vector2 health_placement;
    s32 player_health;
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

    enum TILES Tiles[TILES_X * TILES_Y];

    {
        // WHO SAID C WAS HARD?? >=[
        Allocator heap = get_heap_allocator();

        string read_data;
        bool ok = os_read_entire_file("assets/level.txt", &read_data, heap);

        assert(ok, "Failed: os_file_write_bytes");
        log("length: %llu", read_data.count);

        const u8 *ptr = read_data.data;
        const u8 *end = ptr + read_data.count;
        int i = 0;
        while (ptr < end)
        {
            if (is_char_a_digit(*ptr))
            {
                Tiles[i] = *ptr - '0';
                log("numbaaaa??? %d, index: %d", *ptr - '0', i);
                i += 1;
            }
            ptr += 1;
        }

        log("%d", TILES_X * TILES_Y);
    }

    Gfx_Font *font = load_font_from_disk(STR("C:/windows/fonts/arial.ttf"), get_heap_allocator());
    assert(font, "Failed loading arial.ttf");
    window.title = STR("Tower defense game");
    window.scaled_width = 1280; // We need to set the scaled size if we want to handle system scaling (DPI)
    window.scaled_height = 800;
    window.x = 200;
    window.y = 200;
    window.clear_color = hex_to_rgba(0x11191cff);
    int target_render_width = 480;

    Gfx_Image *sprites = load_image_from_disk(STR("assets/spritesheet.png"), get_heap_allocator());
    assert(sprites, "didnt load image");

    Gfx_Image *heart_sprite = load_image_from_disk(STR("assets/heart.png"), get_heap_allocator());
    assert(heart_sprite, "didnt load image");

    float64 last_time = os_get_current_time_in_seconds();

    Vector2 sprite_size = get_image_size(sprites);

    f32 zoom = 4.0;
    f32 dbl_zoom = zoom * 2.0;

    RunData run_data = {0};
    run_data.max_mana = 15;
    seed_for_random = os_get_current_cycle_count();
    run_data.current_party_members[0].mon_type = get_random_int_in_range(0, MONSTER_MAX - 1);
    run_data.current_party_members[0].mana_cost = get_monster_mana_cost(run_data.current_party_members[0].mon_type);
    run_data.current_party_members[0]
        .active = true;

    run_data.current_party_members[1].mon_type = get_random_int_in_range(0, MONSTER_MAX - 1);
    run_data.current_party_members[1].mana_cost = get_monster_mana_cost(run_data.current_party_members[1].mon_type);
    run_data.current_party_members[1]
        .active = true;
    BattleData battle_data = {0};
    battle_data.selected_monster_index_tower_for_placement = -1;
    battle_data.current_mana = run_data.max_mana;
    battle_data.player_health = 15;
    battle_data.health_placement = v2(4, 4);

    const float64 fps_limit = 144;
    const float64 min_frametime = 1.0 / fps_limit;

    while (!window.should_close)
    {
        reset_temporary_storage();
        reset_world_frame();

        float64 now = os_get_current_time_in_seconds();
        float64 delta_t = now - last_time;

        if (delta_t < min_frametime)
        {
            os_high_precision_sleep((min_frametime - delta_t) * 1000.0);
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

            log("half_tile_x: %d, tile_pos: %d %d", half_tile_x, tile_index_x, tile_index_y);
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

        Matrix4 xform = m4_scalar(1.0);
        xform = m4_translate(xform, v3(window.pixel_width * 0.5f - 120.0, -window.pixel_height * 0.5 + 20.0, 0.0));
        if (draw_btn(xform, font, STR("PLAY")))
        {
            log("FIRES");
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
                    string mana_str_formatted = sprint(temp, mana_str, monster.mana_cost);
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

        gfx_update();

        if (is_key_just_released('E'))
        {
            log("FPS: %.2f", 1.0 / delta_t);
            log("ms: %.2f", delta_t * 1000.0);
        }
    }

    return 0;
}