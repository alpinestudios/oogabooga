#ifndef WORLD_H
#define WORLD_H

#include "entity.h"
#include "item.h"
#include "ui.h"
#include "vector_ext.h"

/* Constants */
#define WORLD_MAX_ENTITY_COUNT 1024
#define UI_INVENTORY_SLOTS 6

typedef struct EntityHashEntry EntityHashEntry_t;

/* Structs */
typedef struct EntityHashEntry {
    Vector2i position;
    Entity_t *entity;
    EntityHashEntry_t *next;
} EntityHashEntry_t;

typedef struct World {
    Entity_t entities[WORLD_MAX_ENTITY_COUNT];
    Entity_t *player;
    ItemData_t inventory[ITEM_ID_MAX];
    ItemSlot_t slots[UI_INVENTORY_SLOTS];
    u32 active_item_slot;
    enum UIState ui_state;
} World_t;

typedef struct WorldFrame {
    Entity_t *selected_entity;
    Vector2 world_mouse_pos;
    Vector2i tile_mouse_pos;
    Matrix4 view;
    Matrix4 proj;
    EntityHashEntry_t *entity_position_hashmap[WORLD_MAX_ENTITY_COUNT];
} WorldFrame_t;

/* Global variables */
World_t *world;
WorldFrame_t *world_frame;

Entity_t *entity_position_hashmap_get(Vector2i pos) {
    unsigned int hash = v2i_hash(pos, WORLD_MAX_ENTITY_COUNT);
    EntityHashEntry_t *entry = world_frame->entity_position_hashmap[hash];
    int counter = 0;
    while (entry != NULL) {
        counter++;
        if (v2i_equal(entry->position, pos)) {
            return entry->entity;
        }
        entry = entry->next;
    }
    return NULL;
}

void entity_position_hashmap_insert(Vector2i pos, Entity_t *entity) {
    unsigned int hash = v2i_hash(pos, WORLD_MAX_ENTITY_COUNT);
    EntityHashEntry_t *new_entry = alloc(get_temporary_allocator(), sizeof(EntityHashEntry_t));
    new_entry->position = pos;
    new_entry->entity = entity;
    if (world_frame->entity_position_hashmap[hash] != NULL) {
        new_entry->next = world_frame->entity_position_hashmap[hash];
    }
    world_frame->entity_position_hashmap[hash] = new_entry;
}

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

void world_init_inventory_item_data() {
    world->inventory[ITEM_ID_ROCK].spriteID = SPRITE_ID_ITEM_ROCK;
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

void world_select_next_item_slot() {
    world->active_item_slot = (world->active_item_slot + 1) % UI_INVENTORY_SLOTS;
}

void world_select_previous_item_slot() {
    if (world->active_item_slot == 0) {
        world->active_item_slot = UI_INVENTORY_SLOTS - 1;
    } else {
        world->active_item_slot = (world->active_item_slot - 1) % UI_INVENTORY_SLOTS;
    }
}

void world_set_ui_state(enum UIState state) {
    assert(state >= 0 && state < UI_STATE_MAX, "UIState of %u is in a wrong range!", state);
    world->ui_state = state;
}

bool world_tile_is_occupied(Vector2i tile_pos) {
    Entity_t *entity = entity_position_hashmap_get(tile_pos);
    return entity != NULL && entity->is_valid && entity->is_walkable == false;
}

bool world_tile_is_occupied_self(Entity_t *self, Vector2i tile_pos) {
    Entity_t *other = entity_position_hashmap_get(tile_pos);
    return self != other && other != NULL && other->is_valid && other->is_walkable == false;
}

bool world_tile_is_occupied_close_distance(Vector2i tile_pos) {
    return false;
}

#endif
