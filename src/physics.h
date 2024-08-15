#ifndef PHYSICS_H
#define PHYSICS_H

#include "entity.h"

void physics_apply_force(Entity_t *entity, Vector2 force) {
    entity->rigidbody.velocity = v2_add(entity->rigidbody.velocity, force);
}

void physics_update_with_friction(Entity_t *entity, float delta_time, float friction_factor) {
    entity->rigidbody.velocity = v2_mulf(entity->rigidbody.velocity, friction_factor);
    entity->position = v2_add(entity->position, v2_mulf(entity->rigidbody.velocity, delta_time));
}

#endif