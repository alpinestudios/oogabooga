#ifndef PLAYER_H
#define PLAYER_H

#include "entity.h"
#include "particle.h"
#include "sound.h"
#include "timer.h"
#include "world.h"

const float GUN_KNOCKBACK_FACTOR = 0.25f;
const float BULLET_SPREAD_ANGLE = 0.1f;
const float PLAYER_ROLL_TIME = 0.15f;

void update_bullet_trail(Entity_t *bullet, float delta_time) {
    if (bullet->entity_type != ENTITY_TYPE_BULLET)
        return;

    Entity_t *player = world_get_player();
    Vector4 trail_color = v4(1.0, 0.78, 0.39, 1.0);
    float speed = v2_length(bullet->rigidbody.velocity);
    Vector2 direction = v2_normalize(v2_sub(bullet->rigidbody.velocity, player->rigidbody.velocity));
    Vector2 particle_vel = v2_mulf(v2_mulf(direction, speed), -0.1f);
    particle_create(bullet->position, particle_vel, 0.10f, v2(1.0f, 1.0f), trail_color);
}

void player_shoot(Vector2 target) {
    Entity_t *player = world_get_player();

    if (player->state == ENTITY_STATE_ROLL) {
        return;
    }

    Entity_t *bullet = entity_create();
    entity_setup(bullet, ENTITY_TYPE_BULLET);
    bullet->position = player->position;

    float spread_angle = get_random_float32_in_range(-1, 1) * BULLET_SPREAD_ANGLE;

    Vector2 bullet_direction = v2_normalize(v2_sub(target, player->position));
    bullet_direction = v2_rotate(bullet_direction, spread_angle);

    Vector2 bullet_force = v2_mulf(bullet_direction, bullet->rigidbody.acceleration);

    // Apply knockback
    physics_apply_force(player, v2_mulf(bullet_force, -GUN_KNOCKBACK_FACTOR));

    // Compensate player's velocity
    // bullet_force = v2_add(bullet_force, player->rigidbody.velocity); // TODO: Think if its needed.

    // Apply force to the bullet
    physics_apply_force(bullet, bullet_force);

    Audio_Playback_Config player_shot_playback_config = {.volume = 0.1f, .playback_speed = 0.80};
    Sound_t *player_shot_sound = get_sound(SOUND_ID_SHOT_01);
    play_one_audio_clip_source_with_config(*player_shot_sound->audio_src, player_shot_playback_config);
}

void player_roll_callback(void *data) {
    // log("Roll timer callback invoked with data(%p)", data);
    Entity_t *player = world_get_player();
    entity_set_state(player, ENTITY_STATE_NONE);
    entity_set_sprite(player, SPRITE_ID_PLAYER);
}

void player_roll(Vector2 direction, float roll_strength) {
    Entity_t *player = world_get_player();
    if (player->state == ENTITY_STATE_ROLL)
        return;

    Vector2 roll_force = v2_mulf(direction, roll_strength);
    entity_set_state(player, ENTITY_STATE_ROLL);
    entity_set_sprite(player, SPRITE_ID_PLAYER_ROLL);
    player->state_reset_counter = 0.0f;
    player->rigidbody.velocity = v2(0.0, 0.0);
    physics_apply_force(player, roll_force);
    Timer_t *roll_timer = timer_create(PLAYER_ROLL_TIME, player_roll_callback, NULL);
}

#endif