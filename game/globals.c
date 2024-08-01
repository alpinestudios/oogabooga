


struct Game {
    Vector2 camera_pos;
    Entity room_enemies[ROOM_ENTITIES];
    Entity player_entity;
    Gfx_Image *player_sprites;
    Gfx_Image *weapon_sprites;
    Gfx_Image *walls;
    Gfx_Font *font;
} typedef Game;
