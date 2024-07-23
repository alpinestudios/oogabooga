// LCG
// Meh distribution, but good enough for general purposes

#define RAND_MAX_64 0xFFFFFFFFFFFFFFFFull
#define MULTIPLIER 6364136223846793005ull
#define INCREMENT 1442695040888963407ull

// #Global
// set this to something like os_get_current_cycle_count() for very randomized seed
ogb_instance u64 seed_for_random;

#if !OOGABOOGA_LINK_EXTERNAL_INSTANCE
u64 seed_for_random = 1;
#endif

u64 get_random() {
    seed_for_random = seed_for_random * MULTIPLIER + INCREMENT;
    return seed_for_random;
}

f32 get_random_float32() {
	return (float32)get_random()/(float32)UINT64_MAX;
}

f64 get_random_float64() {
	return (float64)get_random()/(float64)UINT64_MAX;
}

f32 get_random_float32_in_range(f32 min, f32 max) {
	return (max-min)*get_random_float32()+min;
}
f64 get_random_float64_in_range(f64 min, f64 max) {
	return (max-min)*get_random_float64()+min;
}

s64 get_random_int_in_range(s64 min, s64 max) {
    return min + (s64)(get_random() % (max - min + 1));
}