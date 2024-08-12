#ifndef ANIMATE_H
#define ANIMATE_H

bool almost_equals(float a, float b, float epsilon) {
    return fabs(a - b) <= epsilon;
}

bool animate_f32_to_target(float *value, float target, float delta_t, float rate) {
    *value += (target - *value) * (1.0 - pow(2.0f, -rate * delta_t));
    if (almost_equals(*value, target, 0.001f)) {
        *value = target;
        return true; // reached
    }
    return false;
}

void animate_v2_to_target(Vector2 *value, Vector2 target, float delta_t, float rate) {
    animate_f32_to_target(&(value->x), target.x, delta_t, rate);
    animate_f32_to_target(&(value->y), target.y, delta_t, rate);
}

#endif