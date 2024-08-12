#ifndef ITEM_H
#define ITEM_H

enum ItemID {
    ITEM_ID_NONE = 0,
    ITEM_ID_ROCK,
    ITEM_ID_MAX,
};

typedef struct Item {
    int amount;
} Item_t;

#endif