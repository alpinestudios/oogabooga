
///
// Build config stuff
#ifdef OOGABOOGA_BUILD_SHARED_LIBRARY
    #pragma message("OOGABOOGA_BUILD_SHARED_LIBRARY is defined")
    #include "oogabooga/oogabooga.c"
    #include "game/utils.c"
    #include "game/entity.c"
    #include "game/weapons.c"
    #include "game/globals.c"
    #include "game/ui.c"
#endif
///
///
// This is the "engine" part of your game, which will call into your game.dll
#ifdef OOGABOOGA_BUILD_SHARED_LIBRARY
    typedef void (*Game_Update_Proc)(f64, Game*);
    Game_Update_Proc game_update;
#endif

Dynamic_Library_Handle dll = 0;


#ifdef OOGABOOGA_BUILD_SHARED_LIBRARY
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
#endif

int entry(int argc, char **argv)
{
    #ifdef OOGABOOGA_BUILD_SHARED_LIBRARY
        load_game_dll(argv);
    #endif
    window.scaled_width = 1280;
    window.scaled_height = 800;
    window.x = 200;
    window.y = 200;
    window.clear_color = hex_to_rgba(0x0f101dff);
    window.title = STR("HELLO WORLD");
    Game game = {0};
    game.player_entity.speed = 80;
    game.player_entity.right_hand_weapon = weapon_sword;
    game.player_entity.left_hand_weapon = weapon_shield;
    game.weapon_sprites = load_image_from_disk(STR("assets/weapons.png"), get_heap_allocator());
    game.player_sprites = load_image_from_disk(STR("assets/player.png"), get_heap_allocator());
    game.walls = load_image_from_disk(STR("assets/walls.png"), get_heap_allocator());
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

        game_update(delta_t, &game);

        if (is_key_just_pressed('R'))
        {
             #ifdef OOGABOOGA_BUILD_SHARED_LIBRARY
                load_game_dll(argv);
            #endif
        }

        os_update();
        gfx_update();
    }

    return 0;
}
