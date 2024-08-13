#ifndef ENTITY_H
#define ENTITY_H

#include "item.h"
#include "sprite.h"

/* Constants */
const float PLAYER_ITEM_COLLECT_RANGE = 8.0f;
const int PLAYER_DEFAULT_HEALTH = 100;
const int ENEMY_DEFAULT_HEALTH = 20;
const int ROCK_DEFAULT_HEALTH = 3;

typedef struct Entity Entity_t;

typedef void (*UpdateFunc_t)(Entity_t *self, float64 deltaTime);

void entity_setup_player(Entity_t *entity);
void entity_setup_snail(Entity_t *entity);
void player_update(Entity_t *self, float64 delta_time);
void enemy_update(Entity_t *self, float64 delta_time);

// External functions
extern void physics_apply_force(Entity_t *entity, Vector2 force, float delta_time);
extern void physics_update_with_friction(Entity_t *entity, float delta_time);

enum EntityType {
    ENTITY_TYPE_NONE = 0,
    ENTITY_TYPE_PLAYER,
    ENTITY_TYPE_SNAIL,
    ENTITY_TYPE_ROCK,
    ENTITY_TYPE_ITEM,
    ENTITY_TYPE_MAX
};

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
    UpdateFunc_t update;
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

    physics_apply_force(self, movement_force, delta_time);

    physics_update_with_friction(self, delta_time);
}

void enemy_update(Entity_t *self, float64 delta_time) {
    // log("Updating enemy");
}

bool entity_is_collectible(Entity_t *entity) {
    return entity->entity_type == ENTITY_TYPE_ITEM;
}

#endif