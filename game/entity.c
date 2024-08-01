typedef struct SpriteCell
{
    u8 x;
    u8 y;
} SpriteCell;


enum Weapons {
	weapon_nil,
	weapon_sword,
	weapon_shield
};

typedef struct Entity
{
    bool active;
    bool flip_x;
    // Vector2 grid_pos;
    s32 health;
    s32 max_health;
    f32 speed;

    /* Player specific data, we will just set to -1 for enemy ents*/
    Vector2 position;


	enum Weapons right_hand_weapon;
	enum Weapons left_hand_weapon;


} Entity;



#define ROOM_ENTITIES 20
// Entity *create_enemy()
// {
//     Entity *entity_found = 0;

//     for (u8 i = 0; i < ROOM_ENTITIES; i++)
//     {
//         if (!enemies[i].active)
//         {
//             entity_found = &enemies[i];
//             entity_found->active = true;
//             entity_found->player_monster_relation_array_index = -1;
//             break;
//         }
//     }

//     assert(entity_found, "NO MORE FREE TOWERS");
//     return entity_found;
// }

// int count_active_towers()
// {
//     int count = 0;
//     for (u8 i = 0; i < MAX_PLAYER_TOWERS; i++)
//     {
//         if (player_towers[i].active)
//         {
//             count += 1;
//         }
//     }

//     return count;
// }

// void destory(Entity *entity)
// {
//     memset(entity, 0, sizeof(Entity));
// }


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






Matrix4 render_player(Entity *entity, Gfx_Image *player_sprite) {

    Vector2 sprite_size = get_image_size(player_sprite);
    Matrix4 player_xform = m4_scalar(1.0);
	player_xform = m4_translate(player_xform, v3(entity->position.x, entity->position.y, 0.0));


	bool sword_facing_right = world_frame.world_mouse_pos.x >= entity->position.x;

	{
	   Draw_Quad *quad = draw_image_xform(player_sprite, player_xform, v2(SPRITE_PIXEL_SIZE, SPRITE_PIXEL_SIZE), COLOR_WHITE);
	   quad->uv = getUvCoords(sprite_size, V2_ZERO, SPRITE_PIXEL_SIZE, 3, 1);
	}

	return player_xform;
}


// void render_entity(Entity *entity, Gfx_Image *sprite, Vector2 sprite_size)
// {
//     if (entity->active)
//     {
//         Matrix4 xform = m4_scalar(1.0);
//         xform = m4_translate(xform, v3(entity->position.x, entity->position.y, 0));

//         if (entity->flip_x)
//         {
//             xform = m4_scale(xform, v3(-1, 1, 1));
//             xform = m4_translate(xform, v3(-SPRITE_PIXEL_SIZE, 0.0, 0));
//         }

//         Draw_Quad *quad = draw_image_xform(sprite, xform, v2(SPRITE_PIXEL_SIZE, SPRITE_PIXEL_SIZE), COLOR_WHITE);
//         // quad->uv = getUvCoords(sprite_size, v2(entity->monster_type, 0), SPRITE_PIXEL_SIZE, 6, 1);
//     }
// }
