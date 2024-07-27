
///
// Build config stuff

#define OOGABOOGA_BUILD_SHARED_LIBRARY 1

#include "oogabooga/oogabooga.c"
#include "globals.c"
#include "utils.c"
#include "entity.c"
#include "ui.c"
///
///
// This is the "engine" part of your game, which will call into your game.dll

typedef void (*Game_Update_Proc)(f64);
Game_Update_Proc game_update;

Dynamic_Library_Handle dll = 0;

void load_game_dll(char **argv)
{

    // Here we load all the game symbols

    if (dll)
    {
        os_unload_dynamic_library(dll);
    }

    string exe_path = STR(argv[0]);
    string exe_dir = get_directory_of(exe_path);

    // We need to copy the original and open the copy, so we can recompile the original and then close & replace
    // the copy.
    string dll_path = string_concat(exe_dir, STR("/game.dll"), get_temporary_allocator());
    string used_dll_path = string_concat(exe_dir, STR("/game-in-use.dll"), get_temporary_allocator());

    bool ok = os_file_copy(dll_path, used_dll_path, true);
    assert(ok, "Could not copy %s to %s", dll_path, used_dll_path);

    dll = os_load_dynamic_library(used_dll_path);
    assert(dll, "Failed loading game dll");

    game_update = os_dynamic_library_load_symbol(dll, STR("game_update"));
    assert(game_update, "game is missing game_update()");

    log("Loaded game procedures");
}

int entry(int argc, char **argv)
{

    load_game_dll(argv);

    enum TILES Tiles[TILES_X * TILES_Y];

    {
        // WHO SAID C WAS HARD?? >=[
        Allocator heap = get_heap_allocator();

        string read_data;
        bool ok = os_read_entire_file("assets/level.txt", &read_data, heap);

        assert(ok, "Failed: os_file_write_bytes");

        const u8 *ptr = read_data.data;
        const u8 *end = ptr + read_data.count;
        int i = 0;
        while (ptr < end)
        {
            if (is_char_a_digit(*ptr))
            {
                Tiles[i] = *ptr - '0';
                i += 1;
            }
            ptr += 1;
        }
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
    seed_for_random = rdtsc();
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

    float64 last_time = os_get_current_time_in_seconds();
    while (!window.should_close)
    {
        float64 now = os_get_current_time_in_seconds();
        float64 delta_t = now - last_time;

        if (delta_t < min_frametime)
        {
            os_high_precision_sleep((min_frametime - delta_t) * 1000.0);
            now = os_get_current_time_in_seconds();
            delta_t = now - last_time;
        }
        last_time = now;

        reset_temporary_storage();

        game_update(delta_t);

        if (is_key_just_pressed('R'))
        {
            load_game_dll(argv);
        }

        os_update();
        gfx_update();
    }

    return 0;
}