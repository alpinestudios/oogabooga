

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