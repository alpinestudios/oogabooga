#ifndef PLAYER_H
#define PLAYER_H

#include "entity.h"
#include "world.h"

const float GUN_KNOCKBACK_FACTOR = 0.15f;

void player_shoot(Vector2 target) {
    Entity_t *player = world_get_player();

    Entity_t *bullet = entity_create();
    entity_setup(bullet, ENTITY_TYPE_BULLET);
    bullet->position = player->position;

    Vector2 bullet_force = v2_sub(target, player->position);
    bullet_force = v2_normalize(bullet_force);
    bullet_force = v2_mulf(bullet_force, bullet->rigidbody.acceleration);

    // Apply knockback
    physics_apply_force(player, v2_mulf(bullet_force, -GUN_KNOCKBACK_FACTOR));

    // Compensate player's velocity
    bullet_force = v2_add(bullet_force, player->rigidbody.velocity);

    // Apply force to the bullet
    physics_apply_force(bullet, bullet_force);

    // TODO: Play sound here
    // play_sound(SOUND_PLAYER_SHOOT);
}

void player_roll(Vector2 direction, float roll_strength) {
    Entity_t *player = world_get_player();
    Vector2 roll_force = v2_mulf(direction, roll_strength);
    entity_set_state(player, ENTITY_STATE_ROLL);
    entity_set_sprite(player, SPRITE_ID_PLAYER_ROLL);
    player->state_reset_counter = 0.0f;
    player->rigidbody.velocity = v2(0.0, 0.0);
    physics_apply_force(player, roll_force);
}

#endif