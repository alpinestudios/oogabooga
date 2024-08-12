#ifndef WORLD_H
#define WORLD_H

#include "entity.h"

/* Constants */
#define WORLD_MAX_ENTITIES 1024

/* Structs */

typedef struct World {
    Entity_t entities[WORLD_MAX_ENTITIES];
    Entity_t *player;
} World_t;

/* Global variables */
World_t *g_world;

/* Functions */
Entity_t *entity_create() {
    Entity_t *entity = NULL;
    for (size_t i = 0; i < WORLD_MAX_ENTITIES; i++) {
        entity = &g_world->entities[i];
        if (!entity->isValid) {
            entity->isValid = true;
            return entity;
        }
    }
    assert(entity != NULL, "Couldn't allocate an entity!");
    return NULL;
}

void entity_destroy(Entity_t *entity) {
    memset(entity, 0x00, sizeof(Entity_t));
}

Entity_t *find_first_entity_of_type(enum EntityType entityType) {
    for (size_t i = 0; i < WORLD_MAX_ENTITIES; i++) {
        Entity_t *entity = &g_world->entities[i];
        if (entity->isValid && entity->entityType == entityType) {
            return entity;
        }
    }
    return NULL;
}

void world_set_player(Entity_t *player) {
    g_world->player = player;
}

Entity_t *world_get_player() {
    return g_world->player;
}

#endif
