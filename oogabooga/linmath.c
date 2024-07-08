
#define PI32 3.14159265359f
#define PI64 3.14159265358979323846
#define TAU32 (2.0f * PI32)
#define TAU64 (2.0 * PI64)
#define RAD_PER_DEG (PI64 / 180.0)
#define DEG_PER_RAD (180.0 / PI64)

#define to_radians  (degrees) (((float)degrees)*(float)RAD_PER_DEG)
#define to_degrees  (radians) (((float)radians)*(float)DEG_PER_RAD)
#define to_radians64(degrees) (((float64)degrees)*(float64)RAD_PER_DEG)
#define to_degrees64(radians) (((float64)radians)*(float64)DEG_PER_RAD)
#define to_radians32 to_radians
#define to_degrees32 to_degrees

typedef alignat(16) union Vector2 {
	struct {float32 x, y;};
} Vector2;
inline Vector2 v2(float32 x, float32 y) { return (Vector2){x, y}; }
#define v2_expand(v) (v).x, (v).y

typedef alignat(16) union Vector3 {
	struct {float32 x, y, z;};
	struct {float32 r, g, b;};
	struct {Vector2 xy;};
	struct {float32 _x; Vector2 yz;};
} Vector3;
inline Vector3 v3(float32 x, float32 y, float32 z) { return (Vector3){x, y, z}; }
#define v3_expand(v) (v).x, (v).y, (v).z

typedef alignat(16) union Vector4 {
	struct {float32 x, y, z, w;};
	struct {float32 x1, y1, x2, y2;};
	struct {float32 r, g, b, a;};
	struct {float32 left, bottom, right, top;};
	struct {Vector2 xy; Vector2 zw;};
	struct {Vector3 xyz;};
	struct {float32 _x; Vector3 yzw;};
} Vector4;
inline Vector4 v4(float32 x, float32 y, float32 z, float32 w) { return (Vector4){x, y, z, w}; }
#define v4_expand(v) (v).x, (v).y, (v).z, (v).w

inline Vector2 v2_add(Vector2 a, Vector2 b) {
	simd_add_float32_64((f32*)&a, (f32*)&b, (f32*)&a);
	return a;
}
inline Vector2 v2_sub(Vector2 a, Vector2 b) {
	simd_sub_float32_64((f32*)&a, (f32*)&b, (f32*)&a);
	return a;
}
inline Vector2 v2_mul(Vector2 a, Vector2 b) {
	simd_mul_float32_64((f32*)&a, (f32*)&b, (f32*)&a);
	return a;
}
inline Vector2 v2_mulf(Vector2 a, float32 s) {
	return v2_mul(a, v2(s, s));
}
inline Vector2 v2_div(Vector2 a, Vector2 b) {
	simd_div_float32_64((f32*)&a, (f32*)&b, (f32*)&a);
	return a;
}
inline Vector2 v2_divf(Vector2 a, float32 s) {
	return v2_div(a, v2(s, s));
}

inline Vector3 v3_add(Vector3 a, Vector3 b) {
	Vector4 a128 = v4(a.x, a.y, a.z, 0.0);
	Vector4 b128 = v4(b.x, b.y, b.z, 0.0);
	simd_add_float32_128_aligned((f32*)&a128, (f32*)&b128, (f32*)&a128);
	return a128.xyz;
}
inline Vector3 v3_sub(Vector3 a, Vector3 b) {
	Vector4 a128 = v4(a.x, a.y, a.z, 0.0);
	Vector4 b128 = v4(b.x, b.y, b.z, 0.0);
	simd_sub_float32_128_aligned((f32*)&a128, (f32*)&b128, (f32*)&a128);
	return a128.xyz;
}
inline Vector3 v3_mul(Vector3 a, Vector3 b) {
	Vector4 a128 = v4(a.x, a.y, a.z, 0.0);
	Vector4 b128 = v4(b.x, b.y, b.z, 0.0);
	simd_mul_float32_128_aligned((f32*)&a128, (f32*)&b128, (f32*)&a128);
	return a128.xyz;
}
inline Vector3 v3_mulf(Vector3 a, float32 s) {
	return v3_mul(a, v3(s, s, s));
}
inline Vector3 v3_div(Vector3 a, Vector3 b) {
	Vector4 a128 = v4(a.x, a.y, a.z, 0.0);
	Vector4 b128 = v4(b.x, b.y, b.z, 0.0);
	simd_div_float32_128_aligned((f32*)&a128, (f32*)&b128, (f32*)&a128);
	return a128.xyz;
}
inline Vector3 v3_divf(Vector3 a, float32 s) {
	return v3_div(a, v3(s, s, s));
}

inline Vector4 v4_add(Vector4 a, Vector4 b) {
	simd_add_float32_128_aligned((f32*)&a, (f32*)&b, (f32*)&a);
	return a;
}
inline Vector4 v4_sub(Vector4 a, Vector4 b) {
	simd_sub_float32_128_aligned((f32*)&a, (f32*)&b, (f32*)&a);
	return a;
}
inline Vector4 v4_mul(Vector4 a, Vector4 b) {
	simd_mul_float32_128_aligned((f32*)&a, (f32*)&b, (f32*)&a);
	return a;
}
inline Vector4 v4_mulf(Vector4 a, float32 s) {
	return v4_mul(a, v4(s, s, s, s));
}
inline Vector4 v4_div(Vector4 a, Vector4 b) {
	simd_div_float32_128_aligned((f32*)&a, (f32*)&b, (f32*)&a);
	return a;
}
inline Vector4 v4_divf(Vector4 a, float32 s) {
	return v4_div(a, v4(s, s, s, s));
}


inline Vector2 v2_normalize(Vector2 a) {
    float32 length = sqrt(a.x * a.x + a.y * a.y);
    if (length == 0) {
        return (Vector2){0, 0};
    }
    return v2_divf(a, length);
}

inline float v2_dot_product(Vector2 a, Vector2 b) {
	return simd_dot_product_float32_64((float*)&a, (float*)&b);
}
inline float v3_dot_product(Vector3 a, Vector3 b) {
	return simd_dot_product_float32_96((float*)&a, (float*)&b);
}
inline float v4_dot_product(Vector4 a, Vector4 b) {
	return simd_dot_product_float32_128_aligned((float*)&a, (float*)&b);
}

Vector2 v2_rotate_point_around_pivot(Vector2 point, Vector2 pivot, float32 rotation_radians) {
    float32 s = sin(rotation_radians);
    float32 c = cos(rotation_radians);

    point.x -= pivot.x;
    point.y -= pivot.y;
    point = v2_sub(point, pivot);

    float32 x_new = point.x * c - point.y * s;
    float32 y_new = point.x * s + point.y * c;

    point.x = x_new + pivot.x;
    point.y = y_new + pivot.y;
    point = v2_add(v2(x_new, y_new), pivot);

    return point;
}






typedef struct Matrix4 {
    union {float32 m[4][4]; float32 data[16]; };
} Matrix4;

Matrix4 m4_scalar(float32 scalar) {
    Matrix4 m;
    for (int i = 0; i < 16; i++) {
        m.data[i] = 0.0f;
    }
    m.data[0] = scalar; 
    m.data[5] = scalar; 
    m.data[10] = scalar;
    m.data[15] = scalar;
    return m;
}

Matrix4 m4_make_translation(Vector3 translation) {
    Matrix4 m = m4_scalar(1.0);
    m.m[0][0] = 1.0f; m.m[1][1] = 1.0f; m.m[2][2] = 1.0f; m.m[3][3] = 1.0f;
    m.m[0][3] = translation.x;
    m.m[1][3] = translation.y;
    m.m[2][3] = translation.z;
    return m;
}

Matrix4 m4_make_rotation(Vector3 axis, float32 radians) {
    Matrix4 m = m4_scalar(1.0);
    float32 c = cosf(radians);
    float32 s = sinf(radians);
    float32 t = 1.0f - c;

    m.m[0][0] = c + axis.x * axis.x * t;
    m.m[0][1] = axis.x * axis.y * t + axis.z * s;
    m.m[0][2] = axis.x * axis.z * t - axis.y * s;

    m.m[1][0] = axis.y * axis.x * t - axis.z * s;
    m.m[1][1] = c + axis.y * axis.y * t;
    m.m[1][2] = axis.y * axis.z * t + axis.x * s;

    m.m[2][0] = axis.z * axis.x * t + axis.y * s;
    m.m[2][1] = axis.z * axis.y * t - axis.x * s;
    m.m[2][2] = c + axis.z * axis.z * t;

    m.m[3][3] = 1.0f;
    return m;
}

inline Matrix4 m4_make_rotation_z(float32 radians) {
	return m4_make_rotation(v3(0, 0, 1), radians);
}

Matrix4 m4_make_scale(Vector3 scale) {
    Matrix4 m = m4_scalar(1.0);
    m.m[0][0] = scale.x;
    m.m[1][1] = scale.y;
    m.m[2][2] = scale.z;
    m.m[3][3] = 1.0f;
    return m;
}

Matrix4 m4_mul(Matrix4 a, Matrix4 b) {
    Matrix4 result;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            result.m[i][j] = a.m[i][0] * b.m[0][j] +
                             a.m[i][1] * b.m[1][j] +
                             a.m[i][2] * b.m[2][j] +
                             a.m[i][3] * b.m[3][j];
        }
    }
    return result;
}

inline Matrix4 m4_translate(Matrix4 m, Vector3 translation) {
    Matrix4 translation_matrix = m4_make_translation(translation);
    return m4_mul(m, translation_matrix);
}

inline Matrix4 m4_rotate(Matrix4 m, Vector3 axis, float32 radians) {
    Matrix4 rotation_matrix = m4_make_rotation(axis, radians);
    return m4_mul(m, rotation_matrix);
}
inline Matrix4 m4_rotate_z(Matrix4 m, float32 radians) {
    Matrix4 rotation_matrix = m4_make_rotation(v3(0, 0, 1), radians);
    return m4_mul(m, rotation_matrix);
}

inline Matrix4 m4_scale(Matrix4 m, Vector3 scale) {
    Matrix4 scale_matrix = m4_make_scale(scale);
    return m4_mul(m, scale_matrix);
}


// _near & _far because microsoft...
Matrix4 m4_make_orthographic_projection(float32 left, float32 right, float32 bottom, float32 top, float32 _near, float32 _far) {
    Matrix4 m = m4_scalar(1.0f);
    m.m[0][0] = 2.0f / (right - left);
    m.m[1][1] = 2.0f / (top - bottom);
    m.m[2][2] = -2.0f / (_far - _near);
    m.m[0][3] = -(right + left) / (right - left);
    m.m[1][3] = -(top + bottom) / (top - bottom);
    m.m[2][3] = -(_far + _near) / (_far - _near);
    m.m[3][3] = 1.0f;
    return m;
}

Vector4 m4_transform(Matrix4 m, Vector4 v) {
    Vector4 result;
    result.x = m.m[0][0] * v.x + m.m[0][1] * v.y + m.m[0][2] * v.z + m.m[0][3] * v.w;
    result.y = m.m[1][0] * v.x + m.m[1][1] * v.y + m.m[1][2] * v.z + m.m[1][3] * v.w;
    result.z = m.m[2][0] * v.x + m.m[2][1] * v.y + m.m[2][2] * v.z + m.m[2][3] * v.w;
    result.w = m.m[3][0] * v.x + m.m[3][1] * v.y + m.m[3][2] * v.z + m.m[3][3] * v.w;
    return result;
}
Matrix4 m4_inverse(Matrix4 m) {
    Matrix4 inv;
    float32 det;

    inv.m[0][0] = m.m[1][1] * m.m[2][2] * m.m[3][3] -
                  m.m[1][1] * m.m[2][3] * m.m[3][2] -
                  m.m[2][1] * m.m[1][2] * m.m[3][3] +
                  m.m[2][1] * m.m[1][3] * m.m[3][2] +
                  m.m[3][1] * m.m[1][2] * m.m[2][3] -
                  m.m[3][1] * m.m[1][3] * m.m[2][2];

    inv.m[1][0] = -m.m[1][0] * m.m[2][2] * m.m[3][3] +
                  m.m[1][0] * m.m[2][3] * m.m[3][2] +
                  m.m[2][0] * m.m[1][2] * m.m[3][3] -
                  m.m[2][0] * m.m[1][3] * m.m[3][2] -
                  m.m[3][0] * m.m[1][2] * m.m[2][3] +
                  m.m[3][0] * m.m[1][3] * m.m[2][2];

    inv.m[2][0] = m.m[1][0] * m.m[2][1] * m.m[3][3] -
                  m.m[1][0] * m.m[2][3] * m.m[3][1] -
                  m.m[2][0] * m.m[1][1] * m.m[3][3] +
                  m.m[2][0] * m.m[1][3] * m.m[3][1] +
                  m.m[3][0] * m.m[1][1] * m.m[2][3] -
                  m.m[3][0] * m.m[1][3] * m.m[2][1];

    inv.m[3][0] = -m.m[1][0] * m.m[2][1] * m.m[3][2] +
                   m.m[1][0] * m.m[2][2] * m.m[3][1] +
                   m.m[2][0] * m.m[1][1] * m.m[3][2] -
                   m.m[2][0] * m.m[1][2] * m.m[3][1] -
                   m.m[3][0] * m.m[1][1] * m.m[2][2] +
                   m.m[3][0] * m.m[1][2] * m.m[2][1];

    inv.m[0][1] = -m.m[0][1] * m.m[2][2] * m.m[3][3] +
                  m.m[0][1] * m.m[2][3] * m.m[3][2] +
                  m.m[2][1] * m.m[0][2] * m.m[3][3] -
                  m.m[2][1] * m.m[0][3] * m.m[3][2] -
                  m.m[3][1] * m.m[0][2] * m.m[2][3] +
                  m.m[3][1] * m.m[0][3] * m.m[2][2];

    inv.m[1][1] = m.m[0][0] * m.m[2][2] * m.m[3][3] -
                  m.m[0][0] * m.m[2][3] * m.m[3][2] -
                  m.m[2][0] * m.m[0][2] * m.m[3][3] +
                  m.m[2][0] * m.m[0][3] * m.m[3][2] +
                  m.m[3][0] * m.m[0][2] * m.m[2][3] -
                  m.m[3][0] * m.m[0][3] * m.m[2][2];

    inv.m[2][1] = -m.m[0][0] * m.m[2][1] * m.m[3][3] +
                   m.m[0][0] * m.m[2][3] * m.m[3][1] +
                   m.m[2][0] * m.m[0][1] * m.m[3][3] -
                   m.m[2][0] * m.m[0][3] * m.m[3][1] -
                   m.m[3][0] * m.m[0][1] * m.m[2][3] +
                   m.m[3][0] * m.m[0][3] * m.m[2][1];

    inv.m[3][1] = m.m[0][0] * m.m[2][1] * m.m[3][2] -
                  m.m[0][0] * m.m[2][2] * m.m[3][1] -
                  m.m[2][0] * m.m[0][1] * m.m[3][2] +
                  m.m[2][0] * m.m[0][2] * m.m[3][1] +
                  m.m[3][0] * m.m[0][1] * m.m[2][2] -
                  m.m[3][0] * m.m[0][2] * m.m[2][1];

    inv.m[0][2] = m.m[0][1] * m.m[1][2] * m.m[3][3] -
                  m.m[0][1] * m.m[1][3] * m.m[3][2] -
                  m.m[1][1] * m.m[0][2] * m.m[3][3] +
                  m.m[1][1] * m.m[0][3] * m.m[3][2] +
                  m.m[3][1] * m.m[0][2] * m.m[1][3] -
                  m.m[3][1] * m.m[0][3] * m.m[1][2];

    inv.m[1][2] = -m.m[0][0] * m.m[1][2] * m.m[3][3] +
                  m.m[0][0] * m.m[1][3] * m.m[3][2] +
                  m.m[1][0] * m.m[0][2] * m.m[3][3] -
                  m.m[1][0] * m.m[0][3] * m.m[3][2] -
                  m.m[3][0] * m.m[0][2] * m.m[1][3] +
                  m.m[3][0] * m.m[0][3] * m.m[1][2];

    inv.m[2][2] = m.m[0][0] * m.m[1][1] * m.m[3][3] -
                  m.m[0][0] * m.m[1][3] * m.m[3][1] -
                  m.m[1][0] * m.m[0][1] * m.m[3][3] +
                  m.m[1][0] * m.m[0][3] * m.m[3][1] +
                  m.m[3][0] * m.m[0][1] * m.m[1][3] -
                  m.m[3][0] * m.m[0][3] * m.m[1][1];

    inv.m[3][2] = -m.m[0][0] * m.m[1][1] * m.m[3][2] +
                   m.m[0][0] * m.m[1][2] * m.m[3][1] +
                   m.m[1][0] * m.m[0][1] * m.m[3][2] -
                   m.m[1][0] * m.m[0][2] * m.m[3][1] -
                   m.m[3][0] * m.m[0][1] * m.m[1][2] +
                   m.m[3][0] * m.m[0][2] * m.m[1][1];

    inv.m[0][3] = -m.m[0][1] * m.m[1][2] * m.m[2][3] +
                   m.m[0][1] * m.m[1][3] * m.m[2][2] +
                   m.m[1][1] * m.m[0][2] * m.m[2][3] -
                   m.m[1][1] * m.m[0][3] * m.m[2][2] -
                   m.m[2][1] * m.m[0][2] * m.m[1][3] +
                   m.m[2][1] * m.m[0][3] * m.m[1][2];

    inv.m[1][3] = m.m[0][0] * m.m[1][2] * m.m[2][3] -
                  m.m[0][0] * m.m[1][3] * m.m[2][2] -
                  m.m[1][0] * m.m[0][2] * m.m[2][3] +
                  m.m[1][0] * m.m[0][3] * m.m[2][2] +
                  m.m[2][0] * m.m[0][2] * m.m[1][3] -
                  m.m[2][0] * m.m[0][3] * m.m[1][2];

    inv.m[2][3] = -m.m[0][0] * m.m[1][1] * m.m[2][3] +
                   m.m[0][0] * m.m[1][3] * m.m[2][1] +
                   m.m[1][0] * m.m[0][1] * m.m[2][3] -
                   m.m[1][0] * m.m[0][3] * m.m[2][1] -
                   m.m[2][0] * m.m[0][1] * m.m[1][3] +
                   m.m[2][0] * m.m[0][3] * m.m[1][1];

    inv.m[3][3] = m.m[0][0] * m.m[1][1] * m.m[2][2] -
                  m.m[0][0] * m.m[1][2] * m.m[2][1] -
                  m.m[1][0] * m.m[0][1] * m.m[2][2] +
                  m.m[1][0] * m.m[0][2] * m.m[2][1] +
                  m.m[2][0] * m.m[0][1] * m.m[1][2] -
                  m.m[2][0] * m.m[0][2] * m.m[1][1];

    det = m.m[0][0] * inv.m[0][0] + m.m[0][1] * inv.m[1][0] + m.m[0][2] * inv.m[2][0] + m.m[0][3] * inv.m[3][0];

    if (det == 0)
        return m4_scalar(0); 

    det = 1.0f / det;

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            inv.m[i][j] *= det;
        }
    }

    return inv;
}



// This isn't really linmath but just putting it here for now
#define clamp(x, lo, hi) ((x) < (lo) ? (lo) : ((x) > (hi) ? (hi) : (x)))

