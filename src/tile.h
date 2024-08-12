#ifndef TILE_H
#define TILE_H

const int TILE_WIDTH = 20;
const int TILE_OFFSET = TILE_WIDTH * 0.5;

const Vector4 TILE_GRID_COLOR = {0.25f, 0.25f, 0.25f, 0.25f};
const Vector4 TILE_GRID_COLOR_HOVER = {0.25f, 0.25f, 0.25f, 0.6f};

Vector2i world_pos_to_tile_pos(Vector2 world_pos) {
    return v2i(ceilf((world_pos.x - TILE_OFFSET) / (float)TILE_WIDTH), floorf(world_pos.y / (float)TILE_WIDTH));
}

Vector2 tile_pos_to_world_pos(Vector2i tile_pos) {
    return v2((float)tile_pos.x * TILE_WIDTH,
              (float)tile_pos.y * TILE_WIDTH);
}

Vector2 round_world_pos_to_tile(Vector2 pos) {
    Vector2i world_tile_pos = world_pos_to_tile_pos(pos);
    return tile_pos_to_world_pos(world_tile_pos);
}

#endif