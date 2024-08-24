#ifndef STATISTICS_H
#define STATISTICS_H

typedef struct DebugStatistics {
    int valid_entities;
    int valid_particles;
    int hashmap_correct_entries;
    int hashmap_collisions;
} DebugStatistics_t;

static DebugStatistics_t debug_statistics;

DebugStatistics_t *get_debug_statistics() {
    return &debug_statistics;
}

void reset_debug_statistics() {
    memset(&debug_statistics, 0x00, sizeof(DebugStatistics_t));
}

#endif