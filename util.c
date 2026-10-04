#include "typedef.h"

float cross2d(vec2_t v1, vec2_t v2) {
    return v1.x * v2.y - v1.y * v2.x;
}