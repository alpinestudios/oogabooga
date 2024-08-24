#ifndef PARTICLE_H
#define PARTICLE_H

#include "statistics.h"

#define MAX_PARTICLES 4096

typedef struct Particle {
    Vector2 position;
    Vector2 velocity;
    Vector2 size;
    Vector4 color;
    float friction;
    float lifetime;
    float max_lifetime;
    bool is_valid;
} Particle_t;

Particle_t particles[MAX_PARTICLES];

Particle_t *particle_allocate() {
    Particle_t *particle = NULL;
    for (size_t i = 0; i < MAX_PARTICLES; i++) {
        particle = &particles[i];
        if (!particle->is_valid) {
            memset(particle, 0x00, sizeof(Particle_t));
            particle->is_valid = true;
            return particle;
        }
    }
    assert(particle, "Couldn't allocate a particle!");
    return NULL;
}

Particle_t *particle_create(Vector2 position, Vector2 velocity, float lifetime, Vector2 size, Vector4 color) {
    Particle_t *particle = particle_allocate();
    particle->position = position;
    particle->velocity = velocity;
    particle->lifetime = particle->max_lifetime = lifetime;
    particle->size = size;
    particle->color = color;
    return particle;
}

void particle_update(float delta_time) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        Particle_t *particle = &particles[i];
        if (particle->is_valid) {
            particle->lifetime -= delta_time;
            if (particle->lifetime <= 0) {
                particle->is_valid = false;
                continue;
            }
            particle->position = v2_add(particle->position, v2_mulf(particle->velocity, delta_time));
        }
    }
}

void particles_draw() {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        Particle_t *particle = &particles[i];
        if (particle->is_valid) {
            float alpha = particle->lifetime / particle->max_lifetime;
            Vector4 fade_color = v4(particle->color.r, particle->color.g, particle->color.b, alpha);
            draw_circle(particle->position, particle->size, fade_color);
        }
    }
}

#endif