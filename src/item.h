#ifndef ITEM_H
#define ITEM_H

#include "sprite.h"

enum ItemID {
    ITEM_ID_NONE = 0,
    ITEM_ID_ROCK,
    ITEM_ID_MAX,
};

typedef struct ItemData {
    int amount;
    enum SpriteID spriteID;
} ItemData_t;

typedef struct ItemTemplate {
    enum ItemID itemID;
    enum SpriteID spriteID;
} ItemTemplate_t;

typedef struct ItemSlot {
    enum ItemID itemID;
} ItemSlot_t;

const ItemTemplate_t ITEM_TEMPLATES[ITEM_ID_MAX] = {
    [ITEM_ID_ROCK] = {
        .itemID = ITEM_ID_ROCK,
        .spriteID = SPRITE_ID_ITEM_ROCK}};

#endif