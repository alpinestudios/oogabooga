#include "animate.h"
#include "entity.h"
#include "range.h"
#include "sprite.h"
#include "tile.h"
#include "vector_ext.h"
#include "world.h"

const float32 PLAYER_MOVE_SPEED = 200.0f;
const float32 PLAYER_SELECT_RANGE = 12.0f;

Vector2 screen_to_world() {
    float mouse_x = input_frame.mouse_x;
    float mouse_y = input_frame.mouse_y;
    Matrix4 projection = draw_frame.projection;
    Matrix4 view = draw_frame.view;
    float window_width = window.pixel_width;
    float window_height = window.pixel_height;

    float ndc_x = (mouse_x / (window_width * 0.5f)) - 1.0f;
    float ndc_y = (mouse_y / (window_height * 0.5f)) - 1.0f;

    Vector4 world_pos = {ndc_x, ndc_y, 0, 1};
    world_pos = m4_transform(m4_inverse(projection), world_pos);
    world_pos = m4_transform(view, world_pos);

    return world_pos.xy;
}

float sin_breathe(float time, float rate) {
    return (sin(time * rate) + 1) / 2;
}

int entry(int argc, char **argv) {
    window.title = STR("Duck's revenge");
    window.scaled_width = 1280; // We need to set the scaled size if we want to handle system scaling (DPI)
    window.scaled_height = 720;
    window.x = 200;
    window.y = 90;
    window.clear_color = hex_to_rgba(0x222b1bff);

    Custom_Mouse_Pointer crosshair_mouse_pointer = os_make_custom_mouse_pointer_from_file(STR("src/res/sprites/crosshair.png"), 4, 4, get_heap_allocator());
    assert(crosshair_mouse_pointer, "Couldn't load crosshair mouse pointer.");

    Gfx_Font *font = load_font_from_disk(STR("C:/windows/fonts/arial.ttf"), get_heap_allocator());
    assert(font, "Failed loading arial.ttf");
    const u32 font_height = 48;

    // :sprites load
    {
        /* General */
        load_sprite(fixed_string("src/res/sprites/player.png"), SPRITE_ID_PLAYER);
        load_sprite(fixed_string("src/res/sprites/snail_01.png"), SPRITE_ID_SNAIL_01);
        load_sprite(fixed_string("src/res/sprites/gun_01.png"), SPRITE_ID_GUN_01);
        /* Map */
        load_sprite(fixed_string("src/res/sprites/rock_01.png"), SPRITE_ID_ROCK_01);
        /* Items */
        load_sprite(fixed_string("src/res/sprites/rock_item.png"), SPRITE_ID_ITEM_ROCK);
    }

    world = alloc(get_heap_allocator(), sizeof(World_t));
    world_frame = alloc(get_heap_allocator(), sizeof(WorldFrame_t));

    // :init
    {
        Entity_t *player = entity_create();
        entity_setup_player(player);
        world_set_player(player);

        /* Create an enemy snails */
        for (size_t i = 0; i < 10; i++) {
            Entity_t *snail = entity_create();
            entity_setup_snail(snail);
            snail->position = v2(get_random_float32_in_range(window.pixel_width * -0.5, window.pixel_width * 0.5), get_random_float32_in_range(window.pixel_height * -0.5, window.pixel_height * 0.5));
            snail->position = round_world_pos_to_tile(snail->position);
        }

        /* Create rocks */
        for (size_t i = 0; i < 10; i++) {
            Entity_t *rock = entity_create();
            entity_setup_rock(rock);
            rock->position = v2(get_random_float32_in_range(window.pixel_width * -0.5, window.pixel_width * 0.5), get_random_float32_in_range(window.pixel_height * -0.5, window.pixel_height * 0.5));
            rock->position = round_world_pos_to_tile(rock->position);
        }

        /* Create item rocks */
        for (size_t i = 0; i < 5; i++) {
            Entity_t *item_rock = entity_create();
            entity_setup_item_rock(item_rock);
            item_rock->position = v2(get_random_float32_in_range(window.pixel_width * -0.25, window.pixel_width * 0.25), get_random_float32_in_range(window.pixel_height * -0.25, window.pixel_height * 0.25));
            item_rock->position = round_world_pos_to_tile(item_rock->position);
        }
    }

    Entity_t *player_entity = world_get_player();
    assert(player_entity != NULL);

    float zoom = 3.3f;
    Vector2 camera_pos = v2(0, 0);
    float64 last_time = os_get_current_time_in_seconds();
    while (!window.should_close) {
        reset_temporary_storage();
        world_frame_reset();
        os_update();

        float64 now = os_get_current_time_in_seconds();
        float64 delta_time = now - last_time;
        if ((int)now != (int)last_time) {
            log("%.2f FPS\t%.2fms", 1.0 / (delta_time), (delta_time) * 1000);
        }
        last_time = now;

        draw_frame.projection = m4_make_orthographic_projection(window.pixel_width * -0.5, window.pixel_width * 0.5, window.pixel_height * -0.5, window.pixel_height * 0.5, -1, 10);

        // :camera
        {
            Vector2 target_pos = world_get_player()->position;
            animate_v2_to_target(&camera_pos, target_pos, delta_time, 30.0f);
            draw_frame.view = m4_make_scale(v3(1.0, 1.0, 1.0));
            draw_frame.view = m4_mul(draw_frame.view, m4_make_translation(v3(camera_pos.x, camera_pos.y, 0)));
            draw_frame.view = m4_mul(draw_frame.view, m4_make_scale(v3(1.0 / zoom, 1.0 / zoom, 1.0)));
        }

        if (is_key_just_released(KEY_ESCAPE)) {
            window.should_close = true;
        }
        os_set_mouse_pointer_custom(crosshair_mouse_pointer);

        // :world space mouse position
        world_frame->world_mouse_pos = screen_to_world();
        world_frame->tile_mouse_pos = world_pos_to_tile_pos(world_frame->world_mouse_pos);

        // :position debug
        {
            draw_text(font, sprint(get_temporary_allocator(), STR("%.2f %.2f"), world_frame->world_mouse_pos.x, world_frame->world_mouse_pos.y), font_height, v2(world_frame->world_mouse_pos.x, world_frame->world_mouse_pos.y - 8.0f), v2(0.1, 0.1), COLOR_RED);
            draw_text(font, sprint(get_temporary_allocator(), STR("%d %d"), world_frame->tile_mouse_pos.x, world_frame->tile_mouse_pos.y), font_height, v2(world_frame->world_mouse_pos.x, world_frame->world_mouse_pos.y - 16.0f), v2(0.1, 0.1), COLOR_RED);
        }

        // :entity selection
        {
            float min_mouse_entity_dist = PLAYER_SELECT_RANGE;
            for (int i = 0; i < WORLD_MAX_ENTITY_COUNT; i++) {
                Entity_t *entity = &world->entities[i];
                if (entity->is_valid && entity->selectable) {
                    float mouse_to_entity_dist = absi(v2_dist(entity->position, world_frame->world_mouse_pos));
                    if (mouse_to_entity_dist < min_mouse_entity_dist) {
                        world_frame->selected_entity = entity;
                        min_mouse_entity_dist = mouse_to_entity_dist;
                    }
                }
            }
        }

        // :tiles
        {
            int tile_radius_x = 40;
            int tile_radius_y = 30;
            Vector2i player_tile_pos = world_pos_to_tile_pos(player_entity->position);

            for (int x = player_tile_pos.x - tile_radius_x; x < tile_radius_x + player_tile_pos.x; x++) {
                for (int y = player_tile_pos.y - tile_radius_y; y < tile_radius_y + player_tile_pos.y; y++) {
                    int x_pos = (x * TILE_WIDTH) - TILE_OFFSET;
                    int y_pos = (y * TILE_WIDTH);

                    Vector2i world_tile_pos_in_tile_pos = world_pos_to_tile_pos(v2(x_pos + TILE_OFFSET, y_pos));
                    if (world_tile_pos_in_tile_pos.x == world_frame->tile_mouse_pos.x && world_tile_pos_in_tile_pos.y == world_frame->tile_mouse_pos.y) {
                        draw_rect(v2(x_pos, y_pos), v2(TILE_WIDTH, TILE_WIDTH), TILE_GRID_COLOR_HOVER);
                    } else if ((x + (y % 2 == 0)) % 2 == 0) {
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

        input_axis = v2_normalize(input_axis);
        input_axis = v2_mulf(input_axis, PLAYER_MOVE_SPEED * delta_time);
        player_entity->position = v2_add(player_entity->position, input_axis);

        // :player actions

        if (is_key_just_pressed(MOUSE_BUTTON_LEFT)) {
            consume_key_just_pressed(MOUSE_BUTTON_LEFT);
            Entity_t *entity = world_frame->selected_entity;
            if (entity && entity->destroyable) {
                entity->health -= 1;
                if (entity->health <= 0) {
                    // Loot then destroy
                    switch (entity->entity_type) {
                    case ENTITY_TYPE_ROCK:
                        Entity_t *spawned_entity = entity_create();
                        entity_setup_item_rock(spawned_entity);
                        spawned_entity->position = entity->position;
                        break;
                    default:
                        log("Loot not implemented for entity type of %u.", entity->entity_type);
                        break;
                    }
                    entity_destroy(entity);
                }
            }
        }

        // Entity rendering & update
        {
            for (size_t i = 0; i < WORLD_MAX_ENTITY_COUNT; i++) {
                Entity_t *entity = &world->entities[i];
                if (entity->is_valid) {
                    /* Update */
                    if (entity->update != NULL) {
                        entity->update(entity, delta_time);
                    }
                    /* Render */
                    if (entity->render_sprite == true) {
                        // Sprite_t *gun_sprite = get_sprite(SPRITE_ID_GUN_01);
                        // draw_image_xform(gun_sprite->image, entity_xform, v2(gun_sprite->image->width, gun_sprite->image->height), COLOR_WHITE);

                        Sprite_t *entity_sprite = get_sprite(entity->spriteID);

                        Matrix4 entity_xform = m4_scalar(1.0);

                        // Player y axis swap depending on mouse position
                        if (entity->entity_type == ENTITY_TYPE_PLAYER && input_frame.mouse_x < window.scaled_width * 0.5) {
                            entity_xform = m4_translate(entity_xform, v3(entity->position.x + entity_sprite->image->width * 0.5f, entity->position.y, 1.0f));
                            entity_xform = m4_mul(entity_xform, m4_make_scale(v3(-1.0f, 1.0f, 1.0f)));
                        } else {
                            if (entity->entity_type == ENTITY_TYPE_ITEM) { /* Item wobbling */
                                entity_xform = m4_translate(entity_xform, v3(entity->position.x - entity_sprite->image->width * 0.5f, entity->position.y + (2 * sin_breathe(os_get_current_time_in_seconds(), 5.0f)), 1.0f));
                            } else {
                                entity_xform = m4_translate(entity_xform, v3(entity->position.x - entity_sprite->image->width * 0.5f, entity->position.y, 1.0f));
                            }
                        }

                        Vector4 draw_color = COLOR_WHITE;
                        if (entity == world_frame->selected_entity) {
                            draw_color = COLOR_RED;
                        }

                        draw_image_xform(entity_sprite->image, entity_xform, v2(entity_sprite->image->width, entity_sprite->image->height), draw_color);
                        draw_text(font, sprint(get_temporary_allocator(), STR("%.2f %.2f"), entity->position.x, entity->position.y), font_height, v2(entity->position.x - entity_sprite->image->width * 0.5f, entity->position.y - 8.0f), v2(0.1, 0.1), COLOR_WHITE);
                    }
                }
            }
        }

        gfx_update();
    }

    return 0;
}