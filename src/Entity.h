#ifndef ENTITY_H
#define ENTITY_H

#include "sprite.h"

/* Constants */

const int PLAYER_DEFAULT_HEALTH = 100;
const int ENEMY_DEFAULT_HEALTH = 20;
const int ROCK_DEFAULT_HEALTH = 3;

typedef struct Entity Entity_t;

typedef void (*UpdateFunc_t)(Entity_t *self, float64 deltaTime);

void entity_setup_player(Entity_t *entity);
void entity_setup_snail(Entity_t *entity);
void player_update(Entity_t *self, float64 delta_time);
void enemy_update(Entity_t *self, float64 delta_time);

enum EntityType {
    ENTITY_TYPE_NONE = 0,
    ENTITY_TYPE_PLAYER,
    ENTITY_TYPE_SNAIL,
    ENTITY_TYPE_ROCK,
    ENTITY_TYPE_ITEM,
    ENTITY_TYPE_MAX
};

typedef struct Entity {
    enum EntityType entity_type;
    enum SpriteID spriteID;
    int health;
    Vector2 position;
    bool is_valid;
    bool render_sprite;
    bool selectable;
    bool destroyable;
    UpdateFunc_t update;
} Entity_t;

void entity_setup_player(Entity_t *entity) {
    entity->entity_type = ENTITY_TYPE_PLAYER;
    entity->spriteID = SPRITE_ID_PLAYER;
    entity->position = v2(0.0f, 0.0f);
    entity->render_sprite = true;
    entity->update = player_update;
    entity->health = PLAYER_DEFAULT_HEALTH;
}

void entity_setup_snail(Entity_t *entity) {
    entity->entity_type = ENTITY_TYPE_SNAIL;
    entity->spriteID = SPRITE_ID_SNAIL_01;
    entity->position = v2(0.0f, 0.0f);
    entity->render_sprite = true;
    entity->update = enemy_update;
    entity->health = ENEMY_DEFAULT_HEALTH;
    entity->selectable = true;
    entity->destroyable = true;
}

void entity_setup_rock(Entity_t *entity) {
    entity->entity_type = ENTITY_TYPE_ROCK;
    entity->spriteID = SPRITE_ID_ROCK_01;
    entity->position = v2(0.0f, 0.0f);
    entity->render_sprite = true;
    entity->health = ROCK_DEFAULT_HEALTH;
    entity->selectable = true;
    entity->destroyable = true;
}

void entity_setup_item_rock(Entity_t *entity) {
    entity->entity_type = ENTITY_TYPE_ITEM;
    entity->spriteID = SPRITE_ID_ITEM_ROCK;
    entity->position = v2(0.0f, 0.0f);
    entity->selectable = true;
    entity->render_sprite = true;
}

void player_update(Entity_t *self, float64 delta_time) {
    // log("Updating player");
}

void enemy_update(Entity_t *self, float64 delta_time) {
    // log("Updating enemy");
}

bool entity_is_collectible(Entity_t *entity) {
    return entity->entity_type == ENTITY_TYPE_ITEM;
}

#endif