

typedef struct Vector2 {
	union {float x, r;};
	union {float y, g;};
} Vector2;
typedef Vector2 v2;

typedef struct Vector3 {
	union {float x, r;};
	union {float y, g;};
	union {float z, b;};
} Vector3;
typedef Vector3 v3;

typedef struct Vector4 {
	union {float x, r;};
	union {float y, g;};
	union {float z, b;};
	union {float w, a;};
} Vector4;
typedef Vector4 v4;

// #Simd #Speed

Vector2 v2_add(Vector2 a, Vector2 b) {
	return (v2){a.x + b.x, a.y + b.y};
}
Vector3 v3_add(Vector3 a, Vector3 b) {
	return (v3){a.x + b.x, a.y + b.y, a.z + b.z};
}
Vector4 v4_add(Vector4 a, Vector4 b) {
	return (v4){a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};
}
Vector2 v2_sub(Vector2 a, Vector2 b) {
	return (v2){a.x - b.x, a.y - b.y};
}
Vector3 v3_sub(Vector3 a, Vector3 b) {
	return (v3){a.x - b.x, a.y - b.y, a.z - b.z};
}
Vector4 v4_sub(Vector4 a, Vector4 b) {
	return (v4){a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w};
}
Vector2 v2_mul(Vector2 a, Vector2 b) {
	return (v2){a.x * b.x, a.y * b.y};
}
Vector3 v3_mul(Vector3 a, Vector3 b) {
	return (v3){a.x * b.x, a.y * b.y, a.z * b.z};
}
Vector4 v4_mul(Vector4 a, Vector4 b) {
	return (v4){a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w};
}
Vector2 v2_div(Vector2 a, Vector2 b) {
	return (v2){a.x / b.x, a.y / b.y};
}
Vector3 v3_div(Vector3 a, Vector3 b) {
	return (v3){a.x / b.x, a.y / b.y, a.z / b.z};
}
Vector4 v4_div(Vector4 a, Vector4 b) {
	return (v4){a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w};
}