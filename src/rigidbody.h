#ifndef RIGIDBODY_H
#define RIGIDBODY_H

typedef struct Rigidbody {
    Vector2 velocity;
    float acceleration;
    float friction;
} Rigidbody_t;

#endif