
// randy: we might wanna remove the f by default, similar to the Vector2 ?
// I know that we'll have a Range2i at some point, so maybe it's better to be explicit for less confusion?
// I'll leave this decision up to u charlie just delete this whenever u see it

// charlie: 
// Is this range stuff really necessary?
// Why not just:
// typedef Vector2 Range1f;
// typedef Vector4 Range2f;
// Vector4 also already have alias for x1, y1, x2, y2 and we could add an alias for min & max vectors (see linmath.c)
// This feels like introducing unnecessary complexity and vocabulary when it's really just
// another way to say Vector2 and Vector4.

typedef struct Range1f {
  float min;
  float max;
} Range1f;
// ...

typedef struct Range2f {
  Vector2 min;
  Vector2 max;
} Range2f;

inline Range2f range2f_make(Vector2 min, Vector2 max) { return (Range2f) { min, max }; }

Range2f range2f_shift(Range2f r, Vector2 shift) {
  r.min = v2_add(r.min, shift);
  r.max = v2_add(r.max, shift);
  return r;
}

Range2f range2f_make_bottom_center(Vector2 size) {
  Range2f range = {0};
  range.max = size;
  range = range2f_shift(range, v2(size.x * -0.5, 0.0));
  return range;
}

Vector2 range2f_size(Range2f range) {
  Vector2 size = {0};
  size = v2_sub(range.min, range.max);
  size.x = fabsf(size.x);
  size.y = fabsf(size.y);
  return size;
}

bool range2f_contains(Range2f range, Vector2 v) {
  return v.x >= range.min.x && v.x <= range.max.x && v.y >= range.min.y && v.y <= range.max.y;
}