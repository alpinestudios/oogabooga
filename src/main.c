#include "Entity.h"
#include "Sprite.h"
#include "World.h"

World_t g_world = {0};

int entry(int argc, char **argv) {
    window.title = STR("Duck revenge");
    window.scaled_width = 1280; // We need to set the scaled size if we want to handle system scaling (DPI)
    window.scaled_height = 720;
    window.x = 200;
    window.y = 90;
    window.clear_color = hex_to_rgba(0x222b1bff);

    load_sprite(fixed_string("src/resources/img/player.png"), SPRITE_ID_PLAYER, v2(0.1, 0.1));
    load_sprite(fixed_string("src/resources/img/snail_01.png"), SPRITE_ID_SNAIL_01, v2(0.1, 0.1));

    Entity_t *player = create_entity();
    setup_player_entity(player);

    for (size_t i = 0; i < 10; i++) {
        /* Create an enemy snail */
        Entity_t *snail = create_entity();
        setup_snail_entity(snail);
        snail->position = v2(get_random_float32_in_range(-1.0f, 1.0f), get_random_float32_in_range(-1.0f, 1.0f));
    }

    float64 last_time = os_get_current_time_in_seconds();

    while (!window.should_close) {
        reset_temporary_storage();

        float64 now = os_get_current_time_in_seconds();
        float64 delta = now - last_time;
        if ((int)now != (int)last_time)
            log("%.2f FPS\t%.2fms", 1.0 / (delta), (delta) * 1000);
        last_time = now;

        if (is_key_just_released(KEY_ESCAPE)) {
            window.should_close = true;
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

        const float32 player_move_speed = 1;

        input_axis = v2_normalize(input_axis);
        input_axis = v2_mulf(input_axis, player_move_speed * delta);

        Entity_t *player_entity = find_first_entity_of_type(ENTITY_TYPE_PLAYER);
        assert(player_entity != NULL);

        player_entity->position = v2_add(player_entity->position, input_axis);

        // Entity rendering
        {
            for (size_t i = 0; i < WORLD_MAX_ENTITIES; i++) {
                Entity_t *entity = &g_world.entities[i];
                if (entity->isValid) {
                    Sprite_t *entity_sprite = get_sprite(entity->spriteID);
                    Matrix4 entity_xform = m4_scalar(1.0);
                    entity_xform = m4_translate(entity_xform, v3(entity->position.x, entity->position.y, 1.0f));
                    draw_image_xform(entity_sprite->image, entity_xform, entity_sprite->size, COLOR_WHITE);
                }
            }
        }

        os_update();
        gfx_update();
    }

    return 0;
}