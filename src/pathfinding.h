#ifndef PATHFINDING_H
#define PATHFINDING_H

#include "tile.h"
#include "vector_ext.h"

// Note: Pathfinding algorithm implementation is based on temporary allocator.

typedef struct Node Node_t;

typedef bool (*IsOccupied_fn)(Vector2i);

typedef struct Node {
    Vector2i position;
    int g_cost;
    int h_cost;
    Node_t *parent;
} Node_t;

typedef struct NodeHeap {
    Node_t **nodes;
    int size;
    int capacity;
} NodeHeap_t;

void add_to_heap(NodeHeap_t *heap, Node_t *node) {
    if (heap->size >= heap->capacity) {
        int old_capacity = heap->capacity;
        heap->capacity = heap->capacity == 0 ? 1 : heap->capacity * 2;
        if (heap->nodes != NULL) {
            Node_t **temp = alloc(get_temporary_allocator(), sizeof(Node_t *) * heap->capacity);
            memcpy(temp, heap->nodes, sizeof(Node_t *) * old_capacity);
            heap->nodes = temp;
        } else {
            heap->nodes = alloc(get_temporary_allocator(), sizeof(Node_t *) * heap->capacity);
        }
    }

    heap->nodes[heap->size] = node;
    int current = heap->size;
    heap->size++;

    while (current > 0) {
        int parent = (current - 1) / 2;
        if (heap->nodes[current]->g_cost + heap->nodes[current]->h_cost >=
            heap->nodes[parent]->g_cost + heap->nodes[parent]->h_cost) {
            break;
        }
        Node_t *temp = heap->nodes[current];
        heap->nodes[current] = heap->nodes[parent];
        heap->nodes[parent] = temp;
        current = parent;
    }
}

void remove_from_heap(NodeHeap_t *heap, Node_t *node) {
    int index = -1;
    for (int i = 0; i < heap->size; i++) {
        if (heap->nodes[i] == node) {
            index = i;
            break;
        }
    }

    if (index == -1)
        return;

    heap->nodes[index] = heap->nodes[heap->size - 1];
    heap->size--;

    int current = index;
    while (true) {
        int left_child = 2 * current + 1;
        int right_child = 2 * current + 2;
        int smallest = current;

        if (left_child < heap->size &&
            heap->nodes[left_child]->g_cost + heap->nodes[left_child]->h_cost <
                heap->nodes[smallest]->g_cost + heap->nodes[smallest]->h_cost) {
            smallest = left_child;
        }

        if (right_child < heap->size &&
            heap->nodes[right_child]->g_cost + heap->nodes[right_child]->h_cost <
                heap->nodes[smallest]->g_cost + heap->nodes[smallest]->h_cost) {
            smallest = right_child;
        }

        if (smallest == current)
            break;

        Node_t *temp = heap->nodes[current];
        heap->nodes[current] = heap->nodes[smallest];
        heap->nodes[smallest] = temp;
        current = smallest;
    }
}

Node_t *get_lowest_f_cost(NodeHeap_t *heap) {
    if (heap->size == 0)
        return NULL;
    return heap->nodes[0];
}

bool is_in_heap_v2i(NodeHeap_t *closed_set, Vector2i position) {
    for (int i = 0; i < closed_set->size; i++) {
        if (v2i_equal(closed_set->nodes[i]->position, position)) {
            return true;
        }
    }
    return false;
}

bool is_in_heap_node(NodeHeap_t *open_set, Node_t *node) {
    for (int i = 0; i < open_set->size; i++) {
        if (open_set->nodes[i] == node) {
            return true;
        }
    }
    return false;
}

Node_t *get_node_from_heap(NodeHeap_t *heap, Vector2i position) {
    for (int i = 0; i < heap->size; i++) {
        if (v2i_equal(heap->nodes[i]->position, position)) {
            return heap->nodes[i];
        }
    }
    return NULL;
}

Node_t *a_star(Vector2i start, Vector2i end, IsOccupied_fn is_occupied, NodeHeap_t *open_set, NodeHeap_t *closed_set) {
    Node_t *start_node = alloc(get_temporary_allocator(), sizeof(Node_t));
    start_node->position = start;
    start_node->g_cost = 0;
    start_node->h_cost = v2i_manhattan_distance(start, end);
    start_node->parent = NULL;

    add_to_heap(open_set, start_node);

    while (open_set->size > 0) {
        Node_t *current = get_lowest_f_cost(open_set);
        remove_from_heap(open_set, current);
        add_to_heap(closed_set, current);

        if (v2i_equal(current->position, end)) {
            return current; // Path found
        }

        Vector2i neighbors[4] = {
            {current->position.x + 1, current->position.y},
            {current->position.x - 1, current->position.y},
            {current->position.x, current->position.y + 1},
            {current->position.x, current->position.y - 1}};

        for (int i = 0; i < 4; i++) {
            if (is_occupied(neighbors[i]) || is_in_heap_v2i(closed_set, neighbors[i])) {
                continue;
            }

            int new_g_cost = current->g_cost + 1;
            Node_t *neighbor = get_node_from_heap(open_set, neighbors[i]);

            if (neighbor == NULL || new_g_cost < neighbor->g_cost) {
                if (neighbor == NULL) {
                    neighbor = alloc(get_temporary_allocator(), sizeof(Node_t));
                    neighbor->position = neighbors[i];
                }
                neighbor->g_cost = new_g_cost;
                neighbor->h_cost = v2i_manhattan_distance(neighbors[i], end);
                neighbor->parent = current;

                if (!is_in_heap_node(open_set, neighbor)) {
                    add_to_heap(open_set, neighbor);
                }
            }
        }
    }

    return NULL; // No path found
}

Vector2i *get_path(Node_t *end_node, int *path_length) {
    int length = 0;
    Node_t *current = end_node;
    while (current != NULL) {
        length++;
        current = current->parent;
    }

    Vector2i *path = alloc(get_heap_allocator(), sizeof(Vector2i) * length);
    current = end_node;
    for (int i = length - 1; i >= 0; i--) {
        path[i] = current->position;
        current = current->parent;
    }

    *path_length = length;
    return path;
}

#endif