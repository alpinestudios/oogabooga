#ifndef ENTITY_H
#define ENTITY_H

#include "Sprite.h"

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
} Entity_t;

void setup_player_entity(Entity_t *entity) {
    entity->entityType = ENTITY_TYPE_PLAYER;
    entity->spriteID = SPRITE_ID_PLAYER;
    entity->position = v2(0.0f, 0.0f);
}

void setup_snail_entity(Entity_t *entity) {
    entity->entityType = ENTITY_TYPE_SNAIL;
    entity->spriteID = SPRITE_ID_SNAIL_01;
    entity->position = v2(0.0f, 0.0f);
}

#endif