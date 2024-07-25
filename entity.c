

enum MonsterType
{
    Bear = 0,
    Snake = 1,
    Cobra = 2,
    Wolf = 3,
    Ram = 4,
    Mouse = 5,
    Pigeon = 6,
    SeaGull = 7,
    MONSTER_MAX = 8
};

typedef struct SpriteCell
{
    u8 x;
    u8 y;
} SpriteCell;

typedef struct Entity
{
    bool active;
    bool flip_x;
    // Vector2 grid_pos;
    s32 health;
    s32 max_health;
    f32 speed;

    /* Player specific data, we will just set to -1 for enemy ents*/
    s32 player_monster_relation_array_index;
    enum MonsterType monster_type;
    Vector2 position;

} Entity;

#define MAX_PLAYER_TOWERS 15
Entity player_towers[MAX_PLAYER_TOWERS];

#define MAX_ENEMIES 25
Entity enemies[MAX_ENEMIES];

Entity *create_tower()
{
    Entity *entity_found = 0;

    for (u8 i = 0; i < MAX_PLAYER_TOWERS; i++)
    {
        if (!player_towers[i].active)
        {
            entity_found = &player_towers[i];
            entity_found->active = true;
            break;
        }
    }

    assert(entity_found, "NO MORE FREE TOWERS");
    return entity_found;
}

Entity *create_enemy()
{
    Entity *entity_found = 0;

    for (u8 i = 0; i < MAX_ENEMIES; i++)
    {
        if (!enemies[i].active)
        {
            entity_found = &enemies[i];
            entity_found->active = true;
            entity_found->player_monster_relation_array_index = -1;
            break;
        }
    }

    assert(entity_found, "NO MORE FREE TOWERS");
    return entity_found;
}

int count_active_towers()
{
    int count = 0;
    for (u8 i = 0; i < MAX_PLAYER_TOWERS; i++)
    {
        if (player_towers[i].active)
        {
            count += 1;
        }
    }

    return count;
}

void destory(Entity *entity)
{
    memset(entity, 0, sizeof(Entity));
}

void render_entity(Entity *entity, Gfx_Image *sprite, Vector2 sprite_size)
{
    if (entity->active)
    {
        Matrix4 xform = m4_scalar(1.0);
        xform = m4_translate(xform, v3(entity->position.x, entity->position.y, 0));

        if (entity->flip_x)
        {
            xform = m4_scale(xform, v3(-1, 1, 1));
            xform = m4_translate(xform, v3(-SPRITE_PIXEL_SIZE, 0.0, 0));
        }

        Draw_Quad *quad = draw_image_xform(sprite, xform, v2(SPRITE_PIXEL_SIZE, SPRITE_PIXEL_SIZE), COLOR_WHITE);
        quad->uv = getUvCoords(sprite_size, v2(entity->monster_type, 0), SPRITE_PIXEL_SIZE, 6, 1);
    }
}

int get_monster_mana_cost(enum MonsterType monster_type)
{

    switch (monster_type)
    {
    case Bear:
        return 2;
    case Snake:
        return 1;
    case Cobra:
        return 2;
    case Wolf:
        return 2;
    case Ram:
        return 1;
    case Mouse:
        return 1;
    case Pigeon:
        return 1;
    case SeaGull:
        return 1;
    case MONSTER_MAX:
        assert(false, "INVALID MONSTER");
        return 0;
    }
}