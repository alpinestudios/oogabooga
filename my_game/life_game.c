#include "player_input.c"

typedef enum {
    ENTITY_PLAYER,
    ENTITY_ENEMY,
    ENTITY_ITEM,
    ENTITY_PROJECTILE,
    ENTITY_NPC,
    ENTITY_OBSTACLE,
    // Add more entity types as needed
} EntityType;

typedef struct Entity {
    Vector2 position;
    float width; // Width of the entity for collision detection
    float height; // Height of the entity for collision detection
    float speed;
    bool isActive; // Status to check if the entity is active in the game
    int health; // Health points of the entity
    EntityType type; // Type of the entity (using enum)
} Entity;

int entry(int argc, char** argv)
{
    window.title = STR("Game Example");
    window.width = 1280;
    window.height = 720;
    window.x = 200;
    window.y = 100;
    window.clear_color = hex_to_rgba(0x2a2a3aff);

    // Load player sprite
    Gfx_Image* player_sprite = load_image_from_disk(STR("my_game/man.png"), get_heap_allocator());
    assert(player_sprite, "Failed loading player sprite");

    Entity player = { v2(0, 0), 50.0f, 50.0f, 400.0f, true, 100, ENTITY_PLAYER };

    // Vector2 player_pos = v2(0, 0);

    float64 last_time = os_get_elapsed_seconds();

    while (!window.should_close) {
        reset_temporary_storage();
        float64 now = os_get_elapsed_seconds();
        float64 delta_time = now - last_time;
        last_time = now;

        Vector2 input_axes = v2(0, 0);

        if (is_key_just_released(KEY_ESCAPE)) {
            window.should_close = true;
        }
        player_input(&input_axes);

        input_axes = v2_normalize(input_axes);

        player.position = v2_add(player.position, v2_mulf(input_axes, player.speed * delta_time));

        Matrix4 rect_xform = m4_scalar(1.0);

        rect_xform = m4_translate(rect_xform, v3(player.position.x, player.position.y, 0));

        // Draw the player
        draw_image_xform(player_sprite, rect_xform, v2(player.width, player.height), COLOR_WHITE);

        os_update();
        gfx_update();
    }

    return 0;
}
