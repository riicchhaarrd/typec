#pragma once
#include <stdint.h>

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef int16_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef float float32;
typedef double float64;
typedef char* string;
typedef uint64_t hash;
typedef struct
{
	float x, y, z;
	int32_t padding; //aligned
} vec3;
typedef struct
{
	float x, y, z, w;
} vec4;
typedef struct
{
	float w, x, y, z;
} quat;
typedef struct
{
	float m[16];
} mat4;
typedef struct
{
	float m[9];
} mat3;
typedef struct
{
	float x, y;
} vec2;