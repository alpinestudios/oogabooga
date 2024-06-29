
#define PI32 3.14159265359f
#define PI64 3.14159265358979323846
#define TAU32 (2.0f * PI32)
#define TAU64 (2.0 * PI64)
#define RAD_PER_DEG (PI64 / 180.0)
#define DEG_PER_RAD (180.0 / PI64)

#define to_radians  (degrees) (((float32)degrees)*(float32)RAD_PER_DEG)
#define to_degrees  (radians) (((float32)radians)*(float32)DEG_PER_RAD)
#define to_radians64(degrees) (((float64)degrees)*(float64)RAD_PER_DEG)
#define to_degrees64(radians) (((float64)radians)*(float64)DEG_PER_RAD)
#define to_radians32 to_radians
#define to_degrees32 to_degrees

typedef struct Vector2 {
	union {float32 x, r;};
	union {float32 y, g;};
} Vector2;
inline Vector2 v2(float32 x, float32 y) { return (Vector2){x, y}; }
#define v2_expand(v) (v).x, (v).y

typedef struct Vector3 {
	union {float32 x, r;};
	union {float32 y, g;};
	union {float32 z, b;};
} Vector3;
inline Vector3 v3(float32 x, float32 y, float32 z) { return (Vector3){x, y, z}; }
#define v3_expand(v) (v).x, (v).y, (v).z

typedef struct Vector4 {
	union {float32 x, r, left;  };
	union {float32 y, g, bottom;};
	union {float32 z, b, right; };
	union {float32 w, a, top;   };
} Vector4;
inline Vector4 v4(float32 x, float32 y, float32 z, float32 w) { return (Vector4){x, y, z, w}; }
#define v4_expand(v) (v).x, (v).y, (v).z, (v).w

// #Simd #Speed

Vector2 v2_add(Vector2 a, Vector2 b) {
	return v2(a.x + b.x, a.y + b.y);
}
Vector3 v3_add(Vector3 a, Vector3 b) {
	return v3(a.x + b.x, a.y + b.y, a.z + b.z);
}
Vector4 v4_add(Vector4 a, Vector4 b) {
	return v4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}
Vector2 v2_sub(Vector2 a, Vector2 b) {
	return v2(a.x - b.x, a.y - b.y);
}
Vector3 v3_sub(Vector3 a, Vector3 b) {
	return v3(a.x - b.x, a.y - b.y, a.z - b.z);
}
Vector4 v4_sub(Vector4 a, Vector4 b) {
	return v4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}
Vector2 v2_mul(Vector2 a, Vector2 b) {
	return v2(a.x * b.x, a.y * b.y);
}
Vector3 v3_mul(Vector3 a, Vector3 b) {
	return v3(a.x * b.x, a.y * b.y, a.z * b.z);
}
Vector4 v4_mul(Vector4 a, Vector4 b) {
	return v4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
}
Vector2 v2_div(Vector2 a, Vector2 b) {
	return v2(a.x / b.x, a.y / b.y);
}
Vector3 v3_div(Vector3 a, Vector3 b) {
	return v3(a.x / b.x, a.y / b.y, a.z / b.z);
}
Vector4 v4_div(Vector4 a, Vector4 b) {
	return v4(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w);
}


Vector2 v2_rotate_point_around_pivot(Vector2 point, Vector2 pivot, float32 rotation_radians) {
    float32 s = sin(rotation_radians);
    float32 c = cos(rotation_radians);

    point.x -= pivot.x;
    point.y -= pivot.y;

    float32 x_new = point.x * c - point.y * s;
    float32 y_new = point.x * s + point.y * c;

    point.x = x_new + pivot.x;
    point.y = y_new + pivot.y;

    return point;
}