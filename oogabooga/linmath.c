
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
	struct {float x, y;};
} Vector2;
inline Vector2 v2(float x, float y) { return (Vector2){x, y}; }
#define v2_expand(v) (v).x, (v).y

typedef union Vector3 {
	struct {float x, y, z;};
	struct {float r, g, b;};
	struct {Vector2 xy;};
	struct {float _x; Vector2 yz;};
} Vector3;
inline Vector3 v3(float x, float y, float z) { return (Vector3){x, y, z}; }
#define v3_expand(v) (v).x, (v).y, (v).z

typedef union Vector4 {
	struct {float x, y, z, w;};
	struct {float x1, y1, x2, y2;};
	struct {float r, g, b, a;};
	struct {float left, bottom, right, top;};
	struct {Vector2 xy; Vector2 zw;};
	struct {Vector3 xyz;};
	struct {float _x; Vector3 yzw;};
} Vector4;
inline Vector4 v4(float x, float y, float z, float w) { return (Vector4){x, y, z, w}; }
#define v4_expand(v) (v).x, (v).y, (v).z, (v).w

// #Simd #Speed

inline Vector2 v2_add(Vector2 a, Vector2 b) {
	return v2(a.x + b.x, a.y + b.y);
}
inline Vector3 v3_add(Vector3 a, Vector3 b) {
	return v3(a.x + b.x, a.y + b.y, a.z + b.z);
}
inline Vector4 v4_add(Vector4 a, Vector4 b) {
	return v4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}
inline Vector2 v2_sub(Vector2 a, Vector2 b) {
	return v2(a.x - b.x, a.y - b.y);
}
inline Vector3 v3_sub(Vector3 a, Vector3 b) {
	return v3(a.x - b.x, a.y - b.y, a.z - b.z);
}
inline Vector4 v4_sub(Vector4 a, Vector4 b) {
	return v4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}
inline Vector2 v2_mul(Vector2 a, Vector2 b) {
	return v2(a.x * b.x, a.y * b.y);
}
inline Vector2 v2_mulf(Vector2 a, float b) {
	return v2(a.x * b, a.y * b);
}
inline Vector3 v3_mul(Vector3 a, Vector3 b) {
	return v3(a.x * b.x, a.y * b.y, a.z * b.z);
}
inline Vector4 v4_mul(Vector4 a, Vector4 b) {
	return v4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
}
inline Vector2 v2_div(Vector2 a, Vector2 b) {
	return v2(a.x / b.x, a.y / b.y);
}
inline Vector3 v3_div(Vector3 a, Vector3 b) {
	return v3(a.x / b.x, a.y / b.y, a.z / b.z);
}
inline Vector4 v4_div(Vector4 a, Vector4 b) {
	return v4(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w);
}
inline Vector2 v2_normalize(Vector2 a) {
    float length = sqrt(a.x * a.x + a.y * a.y);
    if (length == 0) {
        return (Vector2){0, 0};
    }
    return (Vector2){a.x / length, a.y / length};
}


Vector2 v2_rotate_point_around_pivot(Vector2 point, Vector2 pivot, float rotation_radians) {
    float s = sin(rotation_radians);
    float c = cos(rotation_radians);

    point.x -= pivot.x;
    point.y -= pivot.y;

    float x_new = point.x * c - point.y * s;
    float y_new = point.x * s + point.y * c;

    point.x = x_new + pivot.x;
    point.y = y_new + pivot.y;

    return point;
}






typedef struct Matrix4 {
    union {float m[4][4]; float data[16]; };
} Matrix4;

Matrix4 m4_scalar(float scalar) {
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

Matrix4 m4_make_rotation(Vector3 axis, float radians) {
    Matrix4 m = m4_scalar(1.0);
    float c = cosf(radians);
    float s = sinf(radians);
    float t = 1.0f - c;

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

inline Matrix4 m4_make_rotation_z(float radians) {
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

Matrix4 m4_multiply(Matrix4 a, Matrix4 b) {
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
    return m4_multiply(m, translation_matrix);
}

inline Matrix4 m4_rotate(Matrix4 m, Vector3 axis, float radians) {
    Matrix4 rotation_matrix = m4_make_rotation(axis, radians);
    return m4_multiply(m, rotation_matrix);
}
inline Matrix4 m4_rotate_z(Matrix4 m, float radians) {
    Matrix4 rotation_matrix = m4_make_rotation(v3(0, 0, 1), radians);
    return m4_multiply(m, rotation_matrix);
}

inline Matrix4 m4_scale(Matrix4 m, Vector3 scale) {
    Matrix4 scale_matrix = m4_make_scale(scale);
    return m4_multiply(m, scale_matrix);
}


// _near & _far because microsoft...
Matrix4 m4_make_orthographic_projection(float left, float right, float bottom, float top, float _near, float _far) {
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
    float det;

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