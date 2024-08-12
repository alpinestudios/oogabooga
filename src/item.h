#ifndef ITEM_H
#define ITEM_H

enum ItemID {
    ITEM_ID_NONE = 0,
    ITEM_ID_ROCK,
    ITEM_ID_MAX,
};

typedef struct ItemData {
    int amount;
    enum SpriteID spriteID;
} ItemData_t;

typedef struct ItemSlot {
    enum ItemID itemID;
} ItemSlot_t;

#endif