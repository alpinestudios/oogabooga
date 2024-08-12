#ifndef ENTITY_H
#define ENTITY_H

#include "sprite.h"

typedef struct Entity Entity_t;

typedef void (*UpdateFunc_t)(Entity_t *self, float64 deltaTime);

void setup_player_entity(Entity_t *entity);
void setup_snail_entity(Entity_t *entity);
void player_update(Entity_t *self, float64 delta_time);
void enemy_update(Entity_t *self, float64 delta_time);

enum EntityType {
    ENTITY_TYPE_NONE = 0,
    ENTITY_TYPE_PLAYER,
    ENTITY_TYPE_SNAIL
};

typedef struct Entity {
    enum EntityType entityType;
    enum SpriteID spriteID;
    Vector2 position;
    bool isValid;
    UpdateFunc_t update;
} Entity_t;

void setup_player_entity(Entity_t *entity) {
    entity->entityType = ENTITY_TYPE_PLAYER;
    entity->spriteID = SPRITE_ID_PLAYER;
    entity->position = v2(0.0f, 0.0f);
    entity->update = player_update;
}

void setup_snail_entity(Entity_t *entity) {
    entity->entityType = ENTITY_TYPE_SNAIL;
    entity->spriteID = SPRITE_ID_SNAIL_01;
    entity->position = v2(0.0f, 0.0f);
    entity->update = enemy_update;
}

void player_update(Entity_t *self, float64 delta_time) {
    // log("Updating player");
}

void enemy_update(Entity_t *self, float64 delta_time) {
    // log("Updating enemy");
}

#endif