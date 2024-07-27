// based on linmath.c

#if ENABLE_SIMD
// #Redundant maybe, possibly even degrades performance #Speed
#define LMATH_ALIGN alignat(16)
#else
#define LMATH_ALIGN
#endif

typedef union Vector2i {
    s32 data[2];
    struct {s32  x, y;};
} Vector2i;

inline Vector2i v2i(s32 x, s32 y) { return (Vector2i){x, y}; }
inline Vector2 v2i_to_v2(Vector2i a) { return v2((f32)a.x, (f32)a.y); };
#define v2i_expand(v) (v).x, (v).y

typedef union Vector3i {
    s32 data[3];
    struct  {s32  x, y, z;};
    struct  {s32  r, g, b;};
    struct  {Vector2i  xy;};
    struct  {s32  _x; Vector2i yz;};
} Vector3i;

inline Vector3i v3i(s32 x, s32 y, s32 z) { return (Vector3i){x, y, z}; }
inline Vector3 v3i_to_v3(Vector3i a) { return v3((f32)a.x, (f32)a.y, (f32)a.z); };
#define v3i_expand(v) (v).x, (v).y, (v).z

typedef union alignat(16) Vector4i {
    s32 data[4];
    struct {s32 x, y, z, w;};
    struct {Vector2i  xy; Vector2i zw;};
    struct {s32  x1, y1, x2, y2;};
    struct {s32  r, g, b, a;};
    struct {s32  left, bottom, right, top;};
    struct {Vector3i  xyz;};
    struct {s32  _x; s32 yzw;};
} Vector4i;

inline Vector4i v4i(s32 x, s32 y, s32 z, s32 w) { return (Vector4i){x, y, z, w}; }
inline Vector4 v4i_to_v4(Vector4i a) { return v4((f32)a.x, (f32)a.y, (f32)a.z, (f32)a.w); };
#define v4i_expand(v) (v).x, (v).y, (v).z, (v).w

// Vector2i
inline Vector2i v2i_add(LMATH_ALIGN Vector2i a, LMATH_ALIGN Vector2i b) {
    return v2i(a.x + b.x, a.y + b.y);
}
inline Vector2i v2i_sub(LMATH_ALIGN Vector2i a, LMATH_ALIGN Vector2i b) {
    return v2i(a.x - b.x, a.y - b.y);
}
inline Vector2i v2i_mul(LMATH_ALIGN Vector2i a, LMATH_ALIGN Vector2i b) {
    return v2i(a.x * b.x, a.y * b.y);
}
inline Vector2i v2i_muli(LMATH_ALIGN Vector2i a, s32 s) {
    return v2i_mul(a, v2i(s, s));
}
inline Vector2i v2i_div(Vector2i a, Vector2i b) {
    return v2i(a.x / b.x, a.y / b.y);
}
inline Vector2i v2i_divi(Vector2i a, s32 s) {
    return v2i_div(a, v2i(s, s));
}

// Vector3i
inline Vector3i v3i_add(LMATH_ALIGN Vector3i a, LMATH_ALIGN Vector3i b) {
    LMATH_ALIGN Vector4i a128 = v4i(a.x, a.y, a.z, 0.0);
    LMATH_ALIGN Vector4i b128 = v4i(b.x, b.y, b.z, 0.0);
    simd_add_int32_128_aligned((s32*)&a128, (s32*)&b128, (s32*)&a128);
    return a128.xyz;
}
inline Vector3i v3i_sub(LMATH_ALIGN Vector3i a, LMATH_ALIGN Vector3i b) {
    LMATH_ALIGN Vector4i a128 = v4i(a.x, a.y, a.z, 0.0);
    LMATH_ALIGN Vector4i b128 = v4i(b.x, b.y, b.z, 0.0);
    simd_sub_int32_128_aligned((s32*)&a128, (s32*)&b128, (s32*)&a128);
    return a128.xyz;
}
inline Vector3i v3i_mul(LMATH_ALIGN Vector3i a, LMATH_ALIGN Vector3i b) {
    LMATH_ALIGN Vector4i a128 = v4i(a.x, a.y, a.z, 0.0);
    LMATH_ALIGN Vector4i b128 = v4i(b.x, b.y, b.z, 0.0);
    simd_mul_int32_128_aligned((s32*)&a128, (s32*)&b128, (s32*)&a128);
    return a128.xyz;
}
inline Vector3i v3i_muli(LMATH_ALIGN Vector3i a, s32 s) {
    return v3i_mul(a, v3i(s, s, s));
}
inline Vector3i v3i_div(Vector3i a, Vector3i b) {
    return v3i(a.x / b.x, a.y / b.y, a.z / b.z);
}
inline Vector3i v3i_divi(Vector3i a, s32 s) {
    return v3i_div(a, v3i(s, s, s));
}

// Vector4i
inline Vector4i v4i_add(LMATH_ALIGN Vector4i a, LMATH_ALIGN Vector4i b) {
    simd_add_int32_128_aligned((s32*)&a, (s32*)&b, (s32*)&a);
    return a;
}
inline Vector4i v4i_sub(LMATH_ALIGN Vector4i a, LMATH_ALIGN Vector4i b) {
    simd_sub_int32_128_aligned((s32*)&a, (s32*)&b, (s32*)&a);
    return a;
}
inline Vector4i v4i_mul(LMATH_ALIGN Vector4i a, LMATH_ALIGN Vector4i b) {
    simd_mul_int32_128_aligned((s32*)&a, (s32*)&b, (s32*)&a);
    return a;
}
inline Vector4i v4i_muli(LMATH_ALIGN Vector4i a, s32 s) {
    return v4i_mul(a, v4i(s, s, s, s));
}

inline Vector4i v4i_div(Vector4i a, Vector4i b) {
    return v4i(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w);
}
inline Vector4i v4i_divi(Vector4i a, s32 s) {
    return v4i_div(a, v4i(s, s, s, s));
}

#define absi(x) ((x) > 0 ? (x) : -(x))

inline Vector2i v2i_abs(LMATH_ALIGN Vector2i a) {
    return v2i(absi(a.x), absi(a.y));
}

inline Vector3i v3i_abs(LMATH_ALIGN Vector3i a) {
    return v3i(absi(a.x), absi(a.y), absi(a.z));
}

inline Vector4i v4i_abs(LMATH_ALIGN Vector4i a) {
    return v4i(absi(a.x), absi(a.y), absi(a.z), absi(a.w));
}
