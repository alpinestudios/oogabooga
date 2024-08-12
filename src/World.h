#ifndef WORLD_H
#define WORLD_H

#include "entity.h"
#include "item.h"

/* Constants */
#define WORLD_MAX_ENTITY_COUNT 1024

/* Structs */
typedef struct World {
    Entity_t entities[WORLD_MAX_ENTITY_COUNT];
    Entity_t *player;
    Item_t inventory[ITEM_ID_MAX];
} World_t;

typedef struct WorldFrame {
    Entity_t *selected_entity;
    Vector2 world_mouse_pos;
    Vector2i tile_mouse_pos;
} WorldFrame_t;

/* Global variables */
World_t *world;
WorldFrame_t *world_frame;

/* Functions */
Entity_t *entity_create() {
    Entity_t *entity = NULL;
    for (size_t i = 0; i < WORLD_MAX_ENTITY_COUNT; i++) {
        entity = &world->entities[i];
        if (!entity->is_valid) {
            entity->is_valid = true;
            return entity;
        }
    }
    assert(entity != NULL, "Couldn't allocate an entity!");
    return NULL;
}

void entity_destroy(Entity_t *entity) {
    memset(entity, 0x00, sizeof(Entity_t));
}

Entity_t *find_first_entity_of_type(enum EntityType entity_type) {
    for (size_t i = 0; i < WORLD_MAX_ENTITY_COUNT; i++) {
        Entity_t *entity = &world->entities[i];
        if (entity->is_valid && entity->entity_type == entity_type) {
            return entity;
        }
    }
    return NULL;
}

void world_set_player(Entity_t *player) {
    world->player = player;
}

Entity_t *world_get_player() {
    return world->player;
}

void world_frame_reset() {
    memset(world_frame, 0x00, sizeof(WorldFrame_t));
}

void world_item_add_to_inventory(enum ItemID id, int amount) {
    assert(id >= 0 && id < ITEM_ID_MAX, "ItemID of %u is in a wrong range!", id);
    world->inventory[id].amount += amount;
}

void world_item_remove_from_inventory(enum ItemID id, int amount) {
    assert(id >= 0 && id < ITEM_ID_MAX, "ItemID of %u is in a wrong range!", id);
    if (world->inventory[id].amount < amount) {
        world->inventory[id].amount = 0;
    } else {
        world->inventory[id].amount -= amount;
    }
}

#endif
