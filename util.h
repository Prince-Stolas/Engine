#ifndef UTIL_H
#define UTIL_H

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define SIGN(x) ((x) > 0 ? 1 : ((x) < 0 ? -1 : 0))

float cross2d(vec2_t v1, vec2_t v2);

#endif