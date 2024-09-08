#ifndef VECTOR_UTILS_H
#define VECTOR_UTILS_H

#include "oogabooga/linmath.c"

typedef struct
{
    float x;
    float y;
} Vector2;

typedef union
{
    struct
    {
        float x, y;
    };
    float v[2];
} Vector2f32;

inline float v2_length(Vector2 a);
inline float v2_dist(Vector2 a, Vector2 b);

#endif // VECTOR_UTILS_H