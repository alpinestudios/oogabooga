#include "animate.h"
#include "entity.h"
#include "pathfinding.h"
#include "physics.h"
#include "player.h"
#include "range.h"
#include "sprite.h"
#include "tile.h"
#include "vector_ext.h"
#include "world.h"

const float32 PLAYER_MOVE_SPEED = 200.0f;
const float32 PLAYER_SELECT_RANGE = 12.0f;

const float SCREEN_WIDTH = 240.0;
const float SCREEN_HEIGHT = 135.0;

void set_screen_space() {
    draw_frame.view = m4_make_scale(v3(1.0, 1.0, 1.0));
    draw_frame.projection = m4_make_orthographic_projection(-1.0f, 1.0f, -1.0f, 1.0f, -1, 10);
    draw_frame.projection = m4_make_orthographic_projection(0.0, SCREEN_WIDTH, 0.0, SCREEN_HEIGHT, -1, 10);
}

void set_world_space() {
    draw_frame.projection = world_frame->proj;
    draw_frame.view = world_frame->view;
}

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
    window.x = 3440 / 2;
    window.y = 1440 / 2;
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
        load_sprite(fixed_string("src/res/sprites/player_roll.png"), SPRITE_ID_PLAYER_ROLL);
        load_sprite(fixed_string("src/res/sprites/snail_01.png"), SPRITE_ID_SNAIL_01);
        load_sprite(fixed_string("src/res/sprites/gun_01.png"), SPRITE_ID_GUN_01);
        load_sprite(fixed_string("src/res/sprites/bullet_01.png"), SPRITE_ID_BULLET_01);
        /* Map */
        load_sprite(fixed_string("src/res/sprites/rock_01.png"), SPRITE_ID_ROCK_01);
        /* Items */
        load_sprite(fixed_string("src/res/sprites/rock_item.png"), SPRITE_ID_ITEM_ROCK);
        /* UI */
        load_sprite(fixed_string("src/res/sprites/ui_item_frame.png"), SPRITE_ID_UI_ITEM_FRAME);
    }

    world = alloc(get_heap_allocator(), sizeof(World_t));
    world_frame = alloc(get_heap_allocator(), sizeof(WorldFrame_t));

    // :init
    {
        Entity_t *player = entity_create();
        entity_setup(player, ENTITY_TYPE_PLAYER);
        world_set_player(player);
        world_init_inventory_item_data();

        /* Create an enemy snails */
        for (size_t i = 0; i < 10; i++) {
            Entity_t *snail = entity_create();
            entity_setup(snail, ENTITY_TYPE_SNAIL);
            snail->position = v2(get_random_float32_in_range(window.pixel_width * -0.5, window.pixel_width * 0.5), get_random_float32_in_range(window.pixel_height * -0.5, window.pixel_height * 0.5));
            snail->position = round_world_pos_to_tile(snail->position);
        }

        /* Create rocks */
        for (size_t i = 0; i < 10; i++) {
            Entity_t *rock = entity_create();
            entity_setup(rock, ENTITY_TYPE_ROCK);
            rock->position = v2(get_random_float32_in_range(window.pixel_width * -0.5, window.pixel_width * 0.5), get_random_float32_in_range(window.pixel_height * -0.5, window.pixel_height * 0.5));
            rock->position = round_world_pos_to_tile(rock->position);
        }

        /* Create item rocks */
        for (size_t i = 0; i < 5; i++) {
            Entity_t *item_rock = entity_create();
            entity_setup_item(item_rock, ITEM_ID_ROCK);
            item_rock->position = v2(get_random_float32_in_range(window.pixel_width * -0.25, window.pixel_width * 0.25), get_random_float32_in_range(window.pixel_height * -0.25, window.pixel_height * 0.25));
            item_rock->position = round_world_pos_to_tile(item_rock->position);
        }

        // Mock rock in slot 0; TODO: Remove
        world->slots[0].itemID = ITEM_ID_ROCK;
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

        {
            world_frame->proj = draw_frame.projection;
            world_frame->view = draw_frame.view;
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

        // :entity selection && hashmap filling
        int hashmap_correct_entries = 0;
        int hashmap_collisions = 0;
        {
            float min_mouse_entity_dist = PLAYER_SELECT_RANGE;
            for (int i = 0; i < WORLD_MAX_ENTITY_COUNT; i++) {
                Entity_t *entity = &world->entities[i];
                if (entity->is_valid) {
                    Vector2i entity_tile_pos = world_pos_to_tile_pos(entity->position);
                    entity_position_hashmap_insert(entity_tile_pos, entity);
                    if (entity->selectable) {
                        float mouse_to_entity_dist = absi(v2_dist(entity->position, world_frame->world_mouse_pos));
                        float player_to_entity_dist = absi(v2_dist(entity->position, player_entity->position));
                        if (mouse_to_entity_dist < min_mouse_entity_dist && player_to_entity_dist <= PLAYER_SELECT_RANGE) {
                            world_frame->selected_entity = entity;
                            min_mouse_entity_dist = mouse_to_entity_dist;
                        }
                    }
                }
            }
            // Debug hashmap stats collection
            for (size_t i = 0; i < WORLD_MAX_ENTITY_COUNT; i++) {
                EntityHashEntry_t *entry = world_frame->entity_position_hashmap[i];
                if (entry != NULL) {
                    hashmap_correct_entries++;
                    entry = entry->next;
                    while (entry != NULL) {
                        hashmap_collisions++;
                        entry = entry->next;
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

                    if (world_tile_is_occupied(world_tile_pos_in_tile_pos)) {
                        draw_rect(v2(x_pos, y_pos), v2(TILE_WIDTH, TILE_WIDTH), COLOR_BLUE);
                    } else {
                        if (world_tile_pos_in_tile_pos.x == world_frame->tile_mouse_pos.x && world_tile_pos_in_tile_pos.y == world_frame->tile_mouse_pos.y) {
                            draw_rect(v2(x_pos, y_pos), v2(TILE_WIDTH, TILE_WIDTH), TILE_GRID_COLOR_HOVER);
                        } else if ((x + (y % 2 == 0)) % 2 == 0) {
                            draw_rect(v2(x_pos, y_pos), v2(TILE_WIDTH, TILE_WIDTH), TILE_GRID_COLOR);
                        }
                    }
                }
            }
        }

        if (is_key_down(KEY_ARROW_UP)) {
            zoom += 1.0f * 10.0f * delta_time;
        }
        if (is_key_down(KEY_ARROW_DOWN)) {
            if (zoom > 2.0f) {
                zoom -= 1.0f * 10.0f * delta_time;
            }
        }

        // :item collection
        {
            for (size_t i = 0; i < WORLD_MAX_ENTITY_COUNT; i++) {
                Entity_t *entity = &world->entities[i];
                if (entity->is_valid && entity->entity_type == ENTITY_TYPE_ITEM) {
                    assert(entity->subtype.itemID > 0 && entity->subtype.itemID < ITEM_ID_MAX, "Entity has wrong subtype.itemID of %u assigned!", entity->subtype.itemID);
                    float distance = fabs(v2_dist(entity->position, player_entity->position));
                    if (distance <= PLAYER_ITEM_COLLECT_RANGE) {
                        world_item_add_to_inventory(entity->subtype.itemID, 1);
                        entity_destroy(entity);
                    }
                }
            }
        }

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
                        entity_setup_item(spawned_entity, ITEM_ID_ROCK);
                        spawned_entity->position = entity->position;
                        break;
                    default:
                        log("Loot not implemented for entity type of %u.", entity->entity_type);
                        break;
                    }
                    entity_destroy(entity);
                }
            } else if (world->slots[world->active_item_slot].itemID != ITEM_ID_NONE) { // Entity not selected and a weapon armed
                Vector2 target = world_frame->world_mouse_pos;
                player_shoot(target);
            }
        }

        if (is_key_just_pressed(KEY_TAB)) {
            consume_key_just_pressed(KEY_TAB);
            if (world->ui_state == UI_STATE_INVENTORY) {
                world_set_ui_state(UI_STATE_NONE);
            } else {
                world_set_ui_state(UI_STATE_INVENTORY);
            }
        }

        if (world->ui_state == UI_STATE_INVENTORY) {
            if (is_key_just_pressed(KEY_ARROW_RIGHT)) {
                consume_key_just_pressed(KEY_ARROW_RIGHT);
                world_select_next_item_slot();
            }

            if (is_key_just_pressed(KEY_ARROW_LEFT)) {
                consume_key_just_pressed(KEY_ARROW_LEFT);
                world_select_previous_item_slot();
            }
        }

        if (is_key_just_pressed(KEY_SPACEBAR)) {
            const float ROLL_ACCELERATION = v2_length(player_entity->rigidbody.velocity) * 2.0f;
            consume_key_just_pressed(KEY_SPACEBAR);
            Vector2 player_move_direction = v2_normalize(player_entity->rigidbody.velocity);
            player_roll(player_move_direction, ROLL_ACCELERATION);
        }

        // :debug player's velocity line
        {
            Entity_t *player = world_get_player();
            Vector2 player_move_direction = v2_normalize(player->rigidbody.velocity);
            float length = v2_length(player->rigidbody.velocity);
            draw_line(player->position, v2_add(player->position, v2_mulf(player_move_direction, length)), 2, COLOR_BLUE);
        }

        int valid_entities = 0;
        // Entity rendering & update
        {
            for (size_t i = 0; i < WORLD_MAX_ENTITY_COUNT; i++) {
                Entity_t *entity = &world->entities[i];
                if (entity->is_valid) {
                    valid_entities++;
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

        // :debug info
        {
            set_screen_space();
            draw_text(font, sprint(get_temporary_allocator(), STR("Current valid entities: %d"), valid_entities), font_height, v2(0.0f, 135.0f - (font_height * 0.1f)), v2(0.1f, 0.1f), COLOR_WHITE);
            draw_text(font, sprint(get_temporary_allocator(), STR("Hashmap valid entities: %d"), hashmap_correct_entries), font_height, v2(0.0f, 135.0f - (font_height * 0.1f) - 5.0f), v2(0.1f, 0.1f), COLOR_WHITE);
            draw_text(font, sprint(get_temporary_allocator(), STR("Hashmap collisions: %d"), hashmap_collisions), font_height, v2(0.0f, 135.0f - (font_height * 0.1f) - 10.0f), v2(0.1f, 0.1f), COLOR_WHITE);
            draw_text(font, sprint(get_temporary_allocator(), STR("Player state: %s"), fixed_string(EntityStateStr[player_entity->state])), font_height, v2(0.0f, 135.0f - (font_height * 0.1f) - 15.0f), v2(0.1f, 0.1f), COLOR_WHITE);
            draw_text(font, sprint(get_temporary_allocator(), STR("Player state counter: %.02f"), player_entity->state_reset_counter), font_height, v2(0.0f, 135.0f - (font_height * 0.1f) - 20.0f), v2(0.1f, 0.1f), COLOR_WHITE);
            set_world_space();
        }

        // :UI Rendering
        {
            if (world->ui_state == UI_STATE_INVENTORY) {
                set_screen_space();
                Sprite_t *ui_frame_sprite = get_sprite(SPRITE_ID_UI_ITEM_FRAME);
                const int FRAME_GAP = 4.0f;
                const int BOTTOM_MARGIN = 4.0f;
                const int LEFT_MARGIN = 4.0f;
                const int TOTAL_INVENTORY_WIDTH = (UI_INVENTORY_SLOTS * ui_frame_sprite->image->width) + (UI_INVENTORY_SLOTS > 1 ? FRAME_GAP * (UI_INVENTORY_SLOTS - 1) : 0);
                for (int i = 0; i < UI_INVENTORY_SLOTS; i++) {
                    Vector3 slot_position = v3(LEFT_MARGIN + (i * (ui_frame_sprite->image->width + FRAME_GAP)), BOTTOM_MARGIN, 0.0f);
                    Matrix4 ui_frame_xform = m4_scalar(1.0);
                    ui_frame_xform = m4_translate(ui_frame_xform, slot_position);
                    Vector4 slot_render_color = v4(0.7f, 0.7f, 0.7f, 0.7f);
                    if (i == world->active_item_slot) {
                        slot_render_color = v4(1.0f, 1.0f, 1.0f, 1.0f);
                    }
                    draw_image_xform(ui_frame_sprite->image, ui_frame_xform, v2(ui_frame_sprite->image->width, ui_frame_sprite->image->height), slot_render_color);
                    enum ItemID itemID = world->slots[i].itemID;
                    if (itemID != ITEM_ID_NONE) {
                        Sprite_t *item_sprite = get_sprite(world->inventory[itemID].spriteID);
                        Vector2 item_ui_pos = v2(slot_position.x + item_sprite->image->width, slot_position.y + item_sprite->image->height);
                        draw_image(item_sprite->image, item_ui_pos, v2(item_sprite->image->width, item_sprite->image->height), slot_render_color);
                    }
                }
            }
        }

        gfx_update();
    }

    return 0;
}