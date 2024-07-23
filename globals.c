typedef struct WorldFrame
{
    Vector2 world_mouse_pos;
    // UI uses different matrix
    Vector2 ui_mouse_pos;
} WorldFrame;

// SpriteCell sprite_cells[MONSTER_MAX];

WorldFrame world_frame = (WorldFrame){0};

void reset_world_frame()
{
    world_frame = (WorldFrame){0};
}