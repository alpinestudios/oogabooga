
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

typedef union Vector2 {
	float data[2];
	struct {float32  x, y;};
} Vector2;
inline Vector2 v2(float32 x, float32 y) { return (Vector2){x, y}; }
#define v2_expand(v) (v).x, (v).y

typedef union Vector3 {
	float data[3];
	struct  {float32  x, y, z;};
	struct  {float32  r, g, b;};
	struct  {Vector2  xy;};
	struct  {float32  _x; Vector2 yz;};
} Vector3;
inline Vector3 v3(float32 x, float32 y, float32 z) { return (Vector3){x, y, z}; }
#define v3_expand(v) (v).x, (v).y, (v).z

typedef union Vector4 {
	float data[4];
	struct {float32  x, y, z, w;};
	struct {Vector2  xy; Vector2 zw;};
	struct {float32  x1, y1, x2, y2;};
	struct {float32  r, g, b, a;};
	struct {float32  left, bottom, right, top;};
	struct {Vector3  xyz;};
	struct {float32  _x; Vector3 yzw;};
} Vector4;
inline Vector4 v4(float32 x, float32 y, float32 z, float32 w) { return (Vector4){x, y, z, w}; }
#define v4_expand(v) (v).x, (v).y, (v).z, (v).w

inline Vector2 v2_add(Vector2 a, Vector2 b) {
	return v2(a.x+b.x, a.y+b.y);
}
inline Vector2 v2_sub(Vector2 a, Vector2 b) {
	return v2(a.x-b.x, a.y-b.y);
}
inline Vector2 v2_mul(Vector2 a, Vector2 b) {
	return v2(a.x*b.x, a.y*b.y);
}
inline Vector2 v2_mulf(Vector2 a, float32 s) {
	return v2_mul(a, v2(s, s));
}
inline Vector2 v2_div(Vector2 a, Vector2 b) {
	return v2(a.x/b.x, a.y/b.y);
}
inline Vector2 v2_divf(Vector2 a, float32 s) {
	return v2_div(a, v2(s, s));
}

inline Vector3 v3_add(Vector3 a, Vector3 b) {
	return v3(a.x+b.x, a.y+b.y, a.z+b.z);
}
inline Vector3 v3_sub(Vector3 a, Vector3 b) {
	return v3(a.x-b.x, a.y-b.y, a.z-b.z);
}
inline Vector3 v3_mul(Vector3 a, Vector3 b) {
	return v3(a.x*b.x, a.y*b.y, a.z*b.z);
}
inline Vector3 v3_div(Vector3 a, Vector3 b) {
	return v3(a.x/b.x, a.y/b.y, a.z/b.z);
}
inline Vector3 v3_mulf(Vector3 a, float32 s) {
	return v3_mul(a, v3(s, s, s));
}
inline Vector3 v3_divf(Vector3 a, float32 s) {
	return v3_div(a, v3(s, s, s));
}

inline Vector4 v4_add(Vector4 a, Vector4 b) {
	return v4(a.x+b.x, a.y+b.y, a.z+b.z, a.w+b.w);
}
inline Vector4 v4_sub(Vector4 a, Vector4 b) {
	return v4(a.x-b.x, a.y-b.y, a.z-b.z, a.w-b.w);
}
inline Vector4 v4_mul(Vector4 a, Vector4 b) {
	return v4(a.x*b.x, a.y*b.y, a.z*b.z, a.w*b.w);
}
inline Vector4 v4_mulf(Vector4 a, float32 s) {
	return v4_mul(a, v4(s, s, s, s));
}
inline Vector4 v4_div(Vector4 a, Vector4 b) {
	return v4(a.x/b.x, a.y/b.y, a.z/b.z, a.w/b.w);
}
inline Vector4 v4_divf(Vector4 a, float32 s) {
	return v4_div(a, v4(s, s, s, s));
}

inline float32 v2_length(Vector2 a) {
	return sqrt(a.x*a.x + a.y*a.y);
}
inline Vector2 v2_normalize(Vector2 a) {
    float32 length = v2_length(a);
    if (length == 0) {
        return (Vector2){0, 0};
    }
    return v2_divf(a, length);
}
inline float32 v2_average(Vector2 a) {
	return (a.x+a.y)/2.0;
}
inline Vector2 v2_abs(Vector2 a) {
	return v2(fabsf(a.x), fabsf(a.y));
}
inline float32 v2_cross(Vector2 a, Vector2 b) {
    return (a.x * b.y) - (a.y * b.x);
}
inline float v2_dot(Vector2 a, Vector2 b) {
	return simd_dot_product_float32_64((float*)&a, (float*)&b);
}

inline float32 v3_length(Vector3 a) {
    return sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
}

inline Vector3 v3_normalize(Vector3 a) {
    float32 length = v3_length(a);
    if (length == 0) {
        return (Vector3){0, 0, 0};
    }
    return v3_divf(a, length);
}

inline float32 v3_average(Vector3 a) {
    return (a.x + a.y + a.z) / 3.0;
}

inline Vector3 v3_abs(Vector3 a) {
    return v3(fabsf(a.x), fabsf(a.y), fabsf(a.z));
}


inline Vector3 v3_cross(Vector3 a, Vector3 b) {
    return (Vector3){
        (a.y * b.z) - (a.z * b.y),
        (a.z * b.x) - (a.x * b.z),
        (a.x * b.y) - (a.y * b.x)
    };
}
inline float v3_dot(Vector3 a, Vector3 b) {
	return simd_dot_product_float32_96((float*)&a, (float*)&b);
}
inline float32 v4_length(Vector4 a) {
    return sqrt(a.x * a.x + a.y * a.y + a.z * a.z + a.w * a.w);
}

inline Vector4 v4_normalize(Vector4 a) {
    float32 length = v4_length(a);
    if (length == 0) {
        return (Vector4){0, 0, 0, 0};
    }
    return v4_divf(a, length);
}

inline float32 v4_average(Vector4 a) {
    return (a.x + a.y + a.z + a.w) / 4.0;
}

inline Vector4 v4_abs(Vector4 a) {
    return v4(fabsf(a.x), fabsf(a.y), fabsf(a.z), fabsf(a.w));
}
inline float v4_dot(Vector4 a, Vector4 b) {
	return simd_dot_product_float32_128_aligned((float*)&a, (float*)&b);
}

Vector2 v2_rotate_point_around_pivot(Vector2 point, Vector2 pivot, float32 rotation_radians) {
    float32 s = sin(rotation_radians);
    float32 c = cos(rotation_radians);

    point = v2_sub(point, pivot);

    float32 x_new = point.x * c - point.y * s;
    float32 y_new = point.x * s + point.y * c;

    point = v2_add(v2(x_new, y_new), pivot);

    return point;
}



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

inline Vector2i v2i_add(Vector2i a, Vector2i b) {
	return v2i(a.x+b.x, a.y+b.y);
}
inline Vector2i v2i_sub(Vector2i a, Vector2i b) {
	return v2i(a.x-b.x, a.y-b.y);
}
inline Vector2i v2i_mul(Vector2i a, Vector2i b) {
	return v2i(a.x*b.x, a.y*b.y);
}
inline Vector2i v2i_muli(Vector2i a, int s) {
	return v2i_mul(a, v2i(s, s));
}
inline Vector2i v2i_div(Vector2i a, Vector2i b) {
	return v2i(a.x/b.x, a.y/b.y);
}
inline Vector2i v2i_divi(Vector2i a, int s) {
	return v2i_div(a, v2i(s, s));
}

inline Vector3i v3i_add(Vector3i a, Vector3i b) {
	return v3i(a.x+b.x, a.y+b.y, a.z+b.z);
}
inline Vector3i v3i_sub(Vector3i a, Vector3i b) {
	return v3i(a.x-b.x, a.y-b.y, a.z-b.z);
}
inline Vector3i v3i_mul(Vector3i a, Vector3i b) {
	return v3i(a.x*b.x, a.y*b.y, a.z*b.z);
}
inline Vector3i v3i_div(Vector3i a, Vector3i b) {
	return v3i(a.x/b.x, a.y/b.y, a.z/b.z);
}
inline Vector3i v3i_muli(Vector3i a, int s) {
	return v3i_mul(a, v3i(s, s, s));
}
inline Vector3i v3i_divi(Vector3i a, int s) {
	return v3i_div(a, v3i(s, s, s));
}

inline Vector4i v4i_add(Vector4i a, Vector4i b) {
	return v4i(a.x+b.x, a.y+b.y, a.z+b.z, a.w+b.w);
}
inline Vector4i v4i_sub(Vector4i a, Vector4i b) {
	return v4i(a.x-b.x, a.y-b.y, a.z-b.z, a.w-b.w);
}
inline Vector4i v4i_mul(Vector4i a, Vector4i b) {
	return v4i(a.x*b.x, a.y*b.y, a.z*b.z, a.w*b.w);
}
inline Vector4i v4i_muli(Vector4i a, int s) {
	return v4i_mul(a, v4i(s, s, s, s));
}
inline Vector4i v4i_div(Vector4i a, Vector4i b) {
	return v4i(a.x/b.x, a.y/b.y, a.z/b.z, a.w/b.w);
}
inline Vector4i v4i_divi(Vector4i a, int s) {
	return v4i_div(a, v4i(s, s, s, s));
}
#define absi(x) ((x) > 0 ? (x) : -(x))

inline Vector2i v2i_abs(Vector2i a) {
    return v2i(absi(a.x), absi(a.y));
}

inline Vector3i v3i_abs(Vector3i a) {
    return v3i(absi(a.x), absi(a.y), absi(a.z));
}

inline Vector4i v4i_abs(Vector4i a) {
    return v4i(absi(a.x), absi(a.y), absi(a.z), absi(a.w));
}

typedef struct alignat(16) Matrix4 {
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

f64 lerpf(f64 from, f64 to, f64 x) {
	return (to-from)*x+from;
}
s64 lerpi(s64 from, s64 to, f64 x) {
	return (s64)((round((f64)to-(f64)from)*x)+from);
}

f64 smerpf(f64 from, f64 to, f64 t) {
	float64 smooth = t * t * (3.0 - 2.0 * t);
	return lerpf(from, to, smooth);
}
s64 smerpi(s64 from, s64 to, f64 t) {
	float64 smooth = t * t * (3.0 - 2.0 * t);
	return lerpi(from, to, smooth);
}