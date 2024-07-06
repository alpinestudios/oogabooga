

#define PRIME64_1 11400714785074694791ULL
#define PRIME64_2 14029467366897019727ULL
#define PRIME64_3 1609587929392839161ULL
#define PRIME64_4 9650029242287828579ULL
#define PRIME64_5 2870177450012600261ULL

static inline u64 xx_hash(u64 x) {
    u64 h64 = PRIME64_5 + 8;
    h64 += x * PRIME64_3;
    h64 = ((h64 << 23) | (h64 >> (64 - 23))) * PRIME64_2 + PRIME64_4;
    h64 ^= h64 >> 33;
    h64 *= PRIME64_2;
    h64 ^= h64 >> 29;
    h64 *= PRIME64_3;
    h64 ^= h64 >> 32;
    return h64;
}

static inline u64 city_hash(string s) {
    const u64 k = 0x9ddfea08eb382d69ULL;
    u64 a = s.count;
    u64 b = s.count * 5;
    u64 c = 9;
    u64 d = b;

    if (s.count <= 16) {
        memcpy(&a, s.data, sizeof(u64));
        memcpy(&b, s.data + s.count - 8, sizeof(u64));
    } else {
        memcpy(&a, s.data, sizeof(u64));
        memcpy(&b, s.data + 8, sizeof(u64));
        memcpy(&c, s.data + s.count - 8, sizeof(u64));
        memcpy(&d, s.data + s.count - 16, sizeof(u64));
    }

    a += b;
    a = (a << 43) | (a >> (64 - 43));
    a += c;
    a = a * 5 + 0x52dce729;
    d ^= a;
    d = (d << 44) | (d >> (64 - 44));
    d += b;

    return d * k;
}

u64 djb2_hash(string s) {
    u64 hash = 5381;
    for (u64 i = 0; i < s.count; i++) {
        hash = ((hash << 5) + hash) + s.data[i];
    }
    return hash;
}

u64 string_get_hash(string s) {
    if (s.count > 32) return djb2_hash(s);
    return city_hash(s);
}
u64 pointer_get_hash(void *p) {
	return xx_hash((u64)p);
}
u64 float64_get_hash(float64 x) {
	return xx_hash(*(u64*)&x);
}
u64 float32_get_hash(float32 x) {
	return float64_get_hash((float64)x);
}

#define get_hash(x) _Generic((x), \
		    string: string_get_hash, \
		    s8: xx_hash, \
		    u8: xx_hash, \
		    s16: xx_hash, \
		    u16: xx_hash, \
		    s32: xx_hash, \
		    u32: xx_hash, \
		    s64: xx_hash, \
		    u64: xx_hash, \
		    f32: float32_get_hash, \
		    f64: float64_get_hash, \
		    default: pointer_get_hash \
		    )(x)