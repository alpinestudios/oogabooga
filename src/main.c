#include "animate.h"
#include "entity.h"
#include "range.h"
#include "sprite.h"
#include "tile.h"
#include "world.h"

Vector2 screen_to_world() {
    float mouseX = input_frame.mouse_x;
    float mouseY = input_frame.mouse_y;
    Matrix4 projection = draw_frame.projection;
    Matrix4 view = draw_frame.view;
    float window_width = window.pixel_width;
    float window_height = window.pixel_height;

    float ndcX = (mouseX / (window_width * 0.5f)) - 1.0f;
    float ndcY = (mouseY / (window_height * 0.5f)) - 1.0f;

    Vector4 worldPos = {ndcX, ndcY, 0, 1};
    worldPos = m4_transform(m4_inverse(projection), worldPos);
    worldPos = m4_transform(view, worldPos);

    return (Vector2){worldPos.x, worldPos.y};
}

int entry(int argc, char **argv) {
    window.title = STR("Duck's revenge");
    window.scaled_width = 1280; // We need to set the scaled size if we want to handle system scaling (DPI)
    window.scaled_height = 720;
    window.x = 200;
    window.y = 90;
    window.clear_color = hex_to_rgba(0x222b1bff);

    Custom_Mouse_Pointer crosshair_mouse_pointer = os_make_custom_mouse_pointer_from_file(STR("src/res/img/crosshair.png"), 4, 4, get_heap_allocator());
    assert(crosshair_mouse_pointer, "Couldn't load crosshair mouse pointer.");

    Gfx_Font *font = load_font_from_disk(STR("C:/windows/fonts/arial.ttf"), get_heap_allocator());
    assert(font, "Failed loading arial.ttf");
    const u32 font_height = 48;

    /* Sprites load */
    {
        load_sprite(fixed_string("src/res/img/player.png"), SPRITE_ID_PLAYER);
        load_sprite(fixed_string("src/res/img/snail_01.png"), SPRITE_ID_SNAIL_01);
        load_sprite(fixed_string("src/res/img/rock_01.png"), SPRITE_ID_ROCK_01);
        load_sprite(fixed_string("src/res/img/gun_01.png"), SPRITE_ID_GUN_01);
    }

    g_world = alloc(get_heap_allocator(), sizeof(World_t));

    Entity_t *player = entity_create();
    setup_player_entity(player);
    world_set_player(player);

    /* Create an enemy snails */
    for (size_t i = 0; i < 10; i++) {
        Entity_t *snail = entity_create();
        setup_snail_entity(snail);
        snail->position = v2(get_random_float32_in_range(window.pixel_width * -0.5, window.pixel_width * 0.5), get_random_float32_in_range(window.pixel_height * -0.5, window.pixel_height * 0.5));
        snail->position = round_world_pos_to_tile(snail->position);
    }

    /* Create rocks */
    for (size_t i = 0; i < 10; i++) {
        Entity_t *rock = entity_create();
        setup_rock_entity(rock);
        rock->position = v2(get_random_float32_in_range(window.pixel_width * -0.5, window.pixel_width * 0.5), get_random_float32_in_range(window.pixel_height * -0.5, window.pixel_height * 0.5));
        rock->position = round_world_pos_to_tile(rock->position);
    }

    float zoom = 3.3f;
    Vector2 camera_pos = v2(0, 0);
    float64 last_time = os_get_current_time_in_seconds();
    while (!window.should_close) {
        reset_temporary_storage();
        os_update();

        draw_frame.projection = m4_make_orthographic_projection(window.pixel_width * -0.5, window.pixel_width * 0.5, window.pixel_height * -0.5, window.pixel_height * 0.5, -1, 10);

        float64 now = os_get_current_time_in_seconds();
        float64 delta_time = now - last_time;
        if ((int)now != (int)last_time) {
            log("%.2f FPS\t%.2fms", 1.0 / (delta_time), (delta_time) * 1000);
        }
        last_time = now;

        if (is_key_just_released(KEY_ESCAPE)) {
            window.should_close = true;
        }
        os_set_mouse_pointer_custom(crosshair_mouse_pointer);

        // :camera
        {
            Vector2 target_pos = world_get_player()->position;
            animate_v2_to_target(&camera_pos, target_pos, delta_time, 30.0f);
            draw_frame.view = m4_make_scale(v3(1.0, 1.0, 1.0));
            draw_frame.view = m4_mul(draw_frame.view, m4_make_translation(v3(camera_pos.x, camera_pos.y, 0)));
            draw_frame.view = m4_mul(draw_frame.view, m4_make_scale(v3(1.0 / zoom, 1.0 / zoom, 1.0)));
        }

        // :world space mouse
        {
            Vector2 worldMousePos = screen_to_world();
            draw_text(font, sprint(get_temporary_allocator(), STR("%.2f %.2f"), worldMousePos.x, worldMousePos.y), font_height, v2(worldMousePos.x, worldMousePos.y - 8.0f), v2(0.1, 0.1), COLOR_RED);

            Vector2i mouseTilePos = world_pos_to_tile_pos(worldMousePos);
            draw_text(font, sprint(get_temporary_allocator(), STR("%d %d"), mouseTilePos.x, mouseTilePos.y), font_height, v2(worldMousePos.x, worldMousePos.y - 16.0f), v2(0.1, 0.1), COLOR_RED);

            for (int i = 0; i < WORLD_MAX_ENTITY_COUNT; i++) {
                Entity_t *entity = &g_world->entities[i];
                if (entity->isValid) {
                    Sprite_t *sprite = get_sprite(entity->spriteID);
                    Range2f bounds = range2f_make_bottom_center(v2(sprite->image->width, sprite->image->height));
                    bounds = range2f_shift(bounds, entity->position);

                    Vector4 color = COLOR_WHITE;
                    color.a = 0.4;

                    if (range2f_contains(bounds, worldMousePos)) {
                        color.a = 1.0f;
                    }

                    draw_rect(bounds.min, range2f_size(bounds), color);
                }
            }
        }

        // :tiles
        {
            Vector2i player_tile_pos = world_pos_to_tile_pos(player->position);

            int tile_radius_x = 40;
            int tile_radius_y = 30;

            for (int x = player_tile_pos.x - tile_radius_x; x < tile_radius_x + player_tile_pos.x; x++) {
                for (int y = player_tile_pos.y - tile_radius_y; y < tile_radius_y + player_tile_pos.y; y++) {
                    if ((x + (y % 2 == 0)) % 2 == 0) {
                        float x_pos = (x * TILE_WIDTH) - TILE_OFFSET;
                        float y_pos = (y * TILE_WIDTH);
                        draw_rect(v2(x_pos, y_pos), v2(TILE_WIDTH, TILE_WIDTH), TILE_GRID_COLOR);
                    }
                }
            }
        }

        Vector2 input_axis = v2(0, 0);
        if (is_key_down('A')) {
            input_axis.x -= 1.0;
        }
        if (is_key_down('D')) {
            input_axis.x += 1.0;
        }
        if (is_key_down('S')) {
            input_axis.y -= 1.0;
        }
        if (is_key_down('W')) {
            input_axis.y += 1.0;
        }

        if (is_key_down(KEY_ARROW_UP)) {
            zoom += 1.0f * 10.0f * delta_time;
        }
        if (is_key_down(KEY_ARROW_DOWN)) {
            if (zoom > 2.0f) {
                zoom -= 1.0f * 10.0f * delta_time;
            }
        }

        const float32 player_move_speed = 100.0f;

        input_axis = v2_normalize(input_axis);
        input_axis = v2_mulf(input_axis, player_move_speed * delta_time);

        Entity_t *player_entity = world_get_player();
        assert(player_entity != NULL);

        player_entity->position = v2_add(player_entity->position, input_axis);

        // Entity rendering & update
        {
            for (size_t i = 0; i < WORLD_MAX_ENTITY_COUNT; i++) {
                Entity_t *entity = &g_world->entities[i];
                if (entity->isValid) {
                    /* Update */
                    if (entity->update != NULL) {
                        entity->update(entity, delta_time);
                    }
                    /* Render */
                    if (entity->renderSprite == true) {
                        // Sprite_t *gun_sprite = get_sprite(SPRITE_ID_GUN_01);
                        // draw_image_xform(gun_sprite->image, entity_xform, v2(gun_sprite->image->width, gun_sprite->image->height), COLOR_WHITE);

                        Sprite_t *entity_sprite = get_sprite(entity->spriteID);

                        Matrix4 entity_xform = m4_scalar(1.0);
                        if (entity->entityType == ENTITY_TYPE_PLAYER && input_frame.mouse_x < window.scaled_width * 0.5) {
                            entity_xform = m4_translate(entity_xform, v3(entity->position.x + entity_sprite->image->width * 0.5f, entity->position.y, 1.0f));
                            entity_xform = m4_mul(entity_xform, m4_make_scale(v3(-1.0f, 1.0f, 1.0f)));
                        } else {
                            entity_xform = m4_translate(entity_xform, v3(entity->position.x - entity_sprite->image->width * 0.5f, entity->position.y, 1.0f));
                        }

                        draw_image_xform(entity_sprite->image, entity_xform, v2(entity_sprite->image->width, entity_sprite->image->height), COLOR_WHITE);
                        draw_text(font, sprint(get_temporary_allocator(), STR("%.2f %.2f"), entity->position.x, entity->position.y), font_height, v2(entity->position.x - entity_sprite->image->width * 0.5f, entity->position.y - 8.0f), v2(0.1, 0.1), COLOR_WHITE);
                    }
                }
            }
        }

        gfx_update();
    }

    return 0;
}