#ifndef TIMER_H
#define TIMER_H

#define MAX_TIMERS 128

// TODO: Review, refactor, use this

typedef struct Timer {
    float duration;
    float elapsed;
    bool active;
    void (*callback)(void *);
    void *user_data;
    bool is_valid;
} Timer_t;

typedef struct TimerStatistics {
    int active_timers;
} TimerStatistics_t;

Timer_t timers[MAX_TIMERS] = {0};
TimerStatistics_t timer_statistics = {0};

Timer_t *timer_allocate() {
    for (size_t i = 0; i < MAX_TIMERS; i++) {
        Timer_t *timer = &timers[i];
        if (!timer->is_valid) {
            memset(timer, 0x00, sizeof(Timer_t));
            timer->is_valid = true;
            return timer;
        }
    }
    return NULL;
}

Timer_t *timer_create(float duration, void (*callback)(void *), void *user_data) {
    Timer_t *timer = timer_allocate();
    assert(timer, "Could not allocate a timer!");
    timer->duration = duration;
    timer->callback = callback;
    timer->user_data = user_data;
    return timer;
}

void timer_destroy(Timer_t *timer) {
    assert(timer, "Timer is not valid");
    if (timer->user_data) {
        dealloc(get_heap_allocator(), timer->user_data);
    }
    memset(timer, 0x00, sizeof(Timer_t));
}

void timer_update(float delta_time) {
    timer_statistics.active_timers = 0;
    for (int i = 0; i < MAX_TIMERS; i++) {
        Timer_t *timer = &timers[i];
        if (timer->is_valid) {
            timer->elapsed += delta_time;
            if (timer->elapsed >= timer->duration) {
                if (timer->callback) {
                    timer->callback(timer->user_data);
                }
                timer_destroy(timer);
            } else {
                timer_statistics.active_timers++;
            }
        }
    }
}

bool timer_exists(Timer_t *timer) {
    return timer && timer->is_valid;
}

void timer_cancel(Timer_t *timer) {
    timer_destroy(timer);
}

#endif