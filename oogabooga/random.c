// This is a naive implementation for now (LCG)
// Distribution will probably suck.

#define RAND_MAX_64 0xFFFFFFFFFFFFFFFFull
#define MULTIPLIER 6364136223846793005ull
#define INCREMENT 1ull

u64 seed_for_random = 1;

u64 get_random() {
    seed_for_random = seed_for_random * MULTIPLIER + INCREMENT;
    return seed_for_random;
}

f32 get_random_float32() {
	return (float32)get_random()/(float32)UINT64_MAX;
}