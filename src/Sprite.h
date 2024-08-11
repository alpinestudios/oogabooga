#ifndef SPRITE_H
#define SPRITE_H

#define MAX_SPRITES_NUMBER 128

enum SpriteID {
    SPRITE_ID_NONE = 0,
    SPRITE_ID_PLAYER,
    SPRITE_ID_SNAIL_01,
    SPRITE_ID_MAX
};

typedef struct Sprite {
    Gfx_Image *image;
    Vector2 size;
} Sprite_t;

Sprite_t g_sprites[MAX_SPRITES_NUMBER];

void load_sprite(string path, enum SpriteID spriteId, Vector2 size) {
    assert(spriteId >= 0 && spriteId < SPRITE_ID_MAX, "SpriteID of %u is in a wrong range!", spriteId);
    assert(g_sprites[spriteId].image == NULL, "SpriteID of %u is already loaded!", spriteId);

    Gfx_Image *loaded_image = load_image_from_disk(path, get_heap_allocator());
    assert(loaded_image, "Failed loading %s image!", path);

    Sprite_t tempSprite = {
        .image = loaded_image,
        .size = size};

    g_sprites[spriteId] = tempSprite;
}

Sprite_t *get_sprite(enum SpriteID spriteId) {
    assert(spriteId >= 0 && spriteId < SPRITE_ID_MAX, "SpriteID of %u is in a wrong range!", spriteId);
    assert(g_sprites[spriteId].image != NULL, "SpriteID of %u has not been loaded yet!", spriteId);
    return &g_sprites[spriteId];
}

#endif