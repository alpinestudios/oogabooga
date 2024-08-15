#ifndef ENTITY_H
#define ENTITY_H

#include "item.h"
#include "pathfinding.h"
#include "sprite.h"
#include "tile.h"

/* Constants */
const float PLAYER_ITEM_COLLECT_RANGE = 8.0f;
const int PLAYER_DEFAULT_HEALTH = 100;
const int ENEMY_DEFAULT_HEALTH = 20;
const int ROCK_DEFAULT_HEALTH = 3;
const int BULLET_DEFAULT_HEALTH = 1;
const float ENEMY_PATH_UPDATE_INTERVAL = 0.33f;

typedef struct Entity Entity_t;

typedef void (*UpdateFunc_t)(Entity_t *self, float64 deltaTime);

void common_update(Entity_t *self, float64 delta_time);
void player_update(Entity_t *self, float64 delta_time);
void enemy_update(Entity_t *self, float64 delta_time);

// External functions
extern void physics_apply_force(Entity_t *entity, Vector2 force);
extern void physics_update_with_friction(Entity_t *entity, float delta_time);
extern void entity_destroy(Entity_t *entity);
extern Entity_t *world_get_player();
extern bool world_tile_is_occupied(Vector2i tile_pos);
extern bool world_tile_is_occupied_close_distance(Vector2i tile_pos);
extern bool world_tile_is_occupied_self(Entity_t *self, Vector2i tile_pos);
extern Entity_t *entity_position_hashmap_get(Vector2i pos);

enum EntityType {
    ENTITY_TYPE_NONE = 0,
    ENTITY_TYPE_PLAYER,
    ENTITY_TYPE_SNAIL,
    ENTITY_TYPE_ROCK,
    ENTITY_TYPE_ITEM,
    ENTITY_TYPE_BULLET,
    ENTITY_TYPE_MAX
};

typedef struct PathData {
    Vector2i *path;
    int path_length;
    int current_path_index;
} PathData_t;

typedef struct Rigidbody {
    Vector2 velocity;
    float acceleration;
    float max_speed;
    float friction;
} Rigidbody_t;

typedef struct Entity {
    enum EntityType entity_type;
    union {
        enum ItemID itemID;
    } subtype;
    enum SpriteID spriteID;
    int health;
    Vector2 position;
    Rigidbody_t rigidbody;
    bool is_valid;
    bool render_sprite;
    bool selectable;
    bool destroyable;
    bool is_walkable;
    float lifetime;
    UpdateFunc_t update;
    PathData_t path_data;
    float path_refresh_counter;
} Entity_t;

const Entity_t ENTITY_TEMPLATES[] = {
    [ENTITY_TYPE_PLAYER] = {
        .entity_type = ENTITY_TYPE_PLAYER,
        .spriteID = SPRITE_ID_PLAYER,
        .health = PLAYER_DEFAULT_HEALTH,
        .render_sprite = true,
        .update = player_update,
        .rigidbody = {.acceleration = 10.0f, .friction = 0.95f, .max_speed = 500.0f},
    },
    [ENTITY_TYPE_SNAIL] = {
        .entity_type = ENTITY_TYPE_SNAIL,
        .spriteID = SPRITE_ID_SNAIL_01,
        .health = ENEMY_DEFAULT_HEALTH,
        .render_sprite = true,
        .selectable = true,
        .destroyable = true,
        .update = enemy_update,
        .rigidbody = {.acceleration = 32.0f, .friction = 0.98f, .max_speed = 120.0f},
    },
    [ENTITY_TYPE_ROCK] = {
        .entity_type = ENTITY_TYPE_ROCK,
        .spriteID = SPRITE_ID_ROCK_01, // TODO: Handle it later, change this field to array, add all sprite variants and when creating entity, select one by random.
        .health = ROCK_DEFAULT_HEALTH,
        .render_sprite = true,
        .selectable = true,
        .destroyable = true,
    },
    [ENTITY_TYPE_ITEM] = {
        .entity_type = ENTITY_TYPE_ITEM,
        .render_sprite = true,
        .selectable = true,
        .is_walkable = true,
    },
    [ENTITY_TYPE_BULLET] = {
        .entity_type = ENTITY_TYPE_BULLET,
        .spriteID = SPRITE_ID_BULLET_01,
        .health = BULLET_DEFAULT_HEALTH,
        .render_sprite = true,
        .destroyable = true,
        .update = common_update,
        .lifetime = 1.0f,
        .is_walkable = true,
        .rigidbody = {
            .friction = 0.9999f,
            .max_speed = 750.0f,
        },
    },
};

void entity_setup_general(Entity_t *entity, enum EntityType type, enum ItemID item_id) {
    assert(type > ENTITY_TYPE_NONE && type < ENTITY_TYPE_MAX, "Entity type of %u is in a wrong range!", type);
    const Entity_t *template = &ENTITY_TEMPLATES[type];

    entity->entity_type = template->entity_type;
    entity->spriteID = template->spriteID;
    entity->health = template->health;
    entity->render_sprite = template->render_sprite;
    entity->selectable = template->selectable;
    entity->destroyable = template->destroyable;
    entity->update = template->update;
    entity->lifetime = template->lifetime;
    entity->rigidbody = template->rigidbody;

    if (type == ENTITY_TYPE_ITEM) {
        assert(item_id >= ITEM_ID_NONE && item_id < ITEM_ID_MAX, "ItemID of %u is in a wrong range!", item_id);
        const ItemTemplate_t *template = &ITEM_TEMPLATES[item_id];
        entity->subtype.itemID = template->itemID;
        entity->spriteID = template->spriteID;
    }
}

void entity_setup(Entity_t *entity, enum EntityType type) {
    entity_setup_general(entity, type, ITEM_ID_NONE);
}

void entity_setup_item(Entity_t *entity, enum ItemID item_id) {
    entity_setup_general(entity, ENTITY_TYPE_ITEM, item_id);
}

void entity_set_target_astar(Entity_t *entity, Vector2 target) {
    Vector2i start_tile = world_pos_to_tile_pos(entity->position);
    Vector2i end_tile = world_pos_to_tile_pos(target);

    NodeHeap_t open_set = {0};
    NodeHeap_t closed_set = {0};

    // TODO: Fix final approach/attack on the player.
    int manhattan_distance = v2i_manhattan_distance(start_tile, end_tile);
    IsOccupied_fn is_occupied = world_tile_is_occupied_close_distance;
    if (manhattan_distance <= 6) {
        is_occupied = world_tile_is_occupied_close_distance;
    }
    Node_t *end_node = a_star(start_tile, end_tile, is_occupied, &open_set, &closed_set);

    if (end_node != NULL) {
        if (entity->path_data.path != NULL) {
            dealloc(get_heap_allocator(), entity->path_data.path);
        }
        entity->path_data.path = get_path(end_node, &entity->path_data.path_length);
        entity->path_data.current_path_index = 0;
    }

    // NodeHeaps cleanup is not needed since it uses temprorary memory. It may be improved in the future.
}

void common_update(Entity_t *self, float64 delta_time) {
    if (self->lifetime > 0.0f) {
        self->lifetime -= delta_time;
        if (self->lifetime <= 0.0f) {
            entity_destroy(self);
        }
    }
    physics_update_with_friction(self, delta_time);
}

void player_update(Entity_t *self, float64 delta_time) {
    Vector2 movement_force = v2(0, 0);
    if (is_key_down('A')) {
        movement_force.x -= 1.0f;
    }
    if (is_key_down('D')) {
        movement_force.x += 1.0f;
    }
    if (is_key_down('S')) {
        movement_force.y -= 1.0f;
    }
    if (is_key_down('W')) {
        movement_force.y += 1.0f;
    }

    movement_force = v2_normalize(movement_force);
    movement_force = v2_mulf(movement_force, self->rigidbody.acceleration);

    physics_apply_force(self, movement_force);

    common_update(self, delta_time);
}

void entity_refresh_path(Entity_t *self, Vector2 target, float update_interval) {
    if (self->path_refresh_counter >= update_interval) {
        if (self->path_data.path == NULL || self->path_data.current_path_index != 0) {
            entity_set_target_astar(self, target);
            self->rigidbody.velocity = v2(0.0, 0.0);
        }
        self->path_refresh_counter = 0.0f;
    }
}

void enemy_update(Entity_t *self, float64 delta_time) {

    if (is_key_just_pressed(MOUSE_BUTTON_RIGHT)) {
        entity_set_target_astar(self, world_get_player()->position);
    }

    if (self->path_data.path != NULL && self->path_data.current_path_index < self->path_data.path_length) {
        // Draw debug path; Keep it for now, there will be changes in pathfinding mechanism
        for (int i = self->path_data.current_path_index; i < self->path_data.path_length; i++) {
            Vector2 world_tile_pos = tile_pos_to_world_pos(self->path_data.path[i]);
            draw_rect(v2(world_tile_pos.x - TILE_OFFSET, world_tile_pos.y), v2(TILE_WIDTH, TILE_WIDTH), v4(1.0, 0.0, 0.0, 0.15));
        }

        Vector2 target_world_pos = tile_pos_to_world_pos(self->path_data.path[self->path_data.current_path_index]);
        Vector2 to_target = v2_sub(target_world_pos, self->position);
        float distance_to_target = v2_length(to_target);

        if (distance_to_target < 1.0f) {
            self->path_data.current_path_index++;
            entity_refresh_path(self, world_get_player()->position, ENEMY_PATH_UPDATE_INTERVAL);
        } else {
            Vector2 direction = v2_normalize(to_target);
            Vector2 acceleration = v2_mulf(direction, self->rigidbody.acceleration);
            physics_apply_force(self, acceleration);
        }
    }

    self->path_refresh_counter += delta_time;

    common_update(self, delta_time);
}

bool entity_is_collectible(Entity_t *entity) {
    return entity->entity_type == ENTITY_TYPE_ITEM;
}

#endif
