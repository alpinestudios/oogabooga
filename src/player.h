#ifndef PLAYER_H
#define PLAYER_H

#include "entity.h"
#include "world.h"

void player_shoot(Vector2 target) {
    Entity_t *player = world_get_player();

    Entity_t *bullet = entity_create();
    entity_setup(bullet, ENTITY_TYPE_BULLET);
    bullet->position = player->position;

    Vector2 direction = v2_sub(target, player->position);
    direction = v2_normalize(direction);
    direction = v2_mulf(direction, bullet->rigidbody.max_speed);

    // Apply force to the bullet
    physics_apply_force(bullet, direction);

    // Apply knockback
    physics_apply_force(player, v2_mulf(direction, -0.15));

    // TODO: Play sound here
    // play_sound(SOUND_PLAYER_SHOOT);
}

#endif