#ifndef VECTOR_EXT_H
#define VECTOR_EXT_H

float v2_dist(Vector2 a, Vector2 b) {
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    return sqrtf(dx * dx + dy * dy);
}

#endif