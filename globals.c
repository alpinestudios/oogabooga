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

typedef struct Monster
{
    bool active;
    s32 level;
    s32 current_xp;
    s32 max_hp;
    s32 mana_cost;
    enum MonsterType mon_type;
} Monster;

struct Tower {
    s32 current_health;
    s32 max_health;
    s32 attack_power;
} typedef Tower;

#define MAX_CREEPS 100
#define MAX_TOWERS 6
struct Game {


    Entity creeps[MAX_CREEPS];
    Tower towers[MAX_TOWERS];

    s32 max_player_health;
    s32 current_player_health;
    s32 max_mana;
    s32 current_mana;

    Monster party[6];

    Gfx_Sprite *monster_sprites;
    Gfx_Font *font;
} typedef Game;
