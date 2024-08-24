#ifndef SOUND_H
#define SOUND_H

#define MAX_SOUNDS_NUMBER 128

enum SoundID {
    SOUND_ID_NONE = 0,
    SOUND_ID_SHOT_01,
    SOUND_ID_MAX,
};

typedef struct Sound {
    Audio_Source *audio_src;
} Sound_t;

Sound_t g_sounds[MAX_SOUNDS_NUMBER];

bool load_audio(string path, enum SoundID sound_id) {
    assert(sound_id >= 0 && sound_id < SOUND_ID_MAX, "sound_id of %u is in a wrong range!", sound_id);
    assert(g_sounds[sound_id].audio_src == NULL, "sound_id of %u is already loaded!", sound_id);

    Audio_Source *audio_src = alloc(get_heap_allocator(), sizeof(Audio_Source));
    assert(audio_src, "Couldn't allocate a memory for audio source(%s)!", path);

    bool result = audio_open_source_load(audio_src, path, get_heap_allocator());

    if (!result) {
        dealloc(get_heap_allocator(), audio_src);
        assert(false, "Could not load an audio(%s).", path);
        return false;
    }

    g_sounds[sound_id] = (Sound_t){.audio_src = audio_src};
    return true;
}

void unload_audio(enum SoundID sound_id) {
    assert(sound_id >= 0 && sound_id < SOUND_ID_MAX, "sound_id of %u is in a wrong range!", sound_id);
    assert(g_sounds[sound_id].audio_src != NULL, "sound_id of %u has not been loaded yet!", sound_id);
    audio_source_destroy(g_sounds[sound_id].audio_src);
    dealloc(get_heap_allocator(), g_sounds[sound_id].audio_src);
    g_sounds[sound_id].audio_src = NULL;
}

Sound_t *get_sound(enum SoundID sound_id) {
    assert(sound_id >= 0 && sound_id < SOUND_ID_MAX, "sound_id of %u is in a wrong range!", sound_id);
    assert(g_sounds[sound_id].audio_src != NULL, "sound_id of %u has not been loaded yet!", sound_id);
    return &g_sounds[sound_id];
}

#endif