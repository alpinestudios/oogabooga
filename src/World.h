#ifndef WORLD_H
#define WORLD_H

#include "Entity.h"

/* Constants */

#define WORLD_MAX_ENTITIES 1024

/* Structs */

typedef struct World {
    Entity_t entities[WORLD_MAX_ENTITIES];
} World_t;

/* Global variables */

World_t g_world;

/* Functions */

Entity_t *create_entity() {
    Entity_t *entity = NULL;
    for (size_t i = 0; i < WORLD_MAX_ENTITIES; i++) {
        entity = &g_world.entities[i];
        if (!entity->isValid) {
            entity->isValid = true;
            return entity;
        }
    }
    assert(entity != NULL, "Couldn't allocate an entity!");
    return NULL;
}

void destroy_entity(Entity_t *entity_ptr) {
    memset(entity_ptr, 0x00, sizeof(Entity_t));
}

Entity_t *find_first_entity_of_type(enum EntityType entityType) {
    for (size_t i = 0; i < WORLD_MAX_ENTITIES; i++) {
        Entity_t *entity = &g_world.entities[i];
        if (entity->isValid && entity->entityType == entityType) {
            return entity;
        }
    }
    return NULL;
}

#endif
