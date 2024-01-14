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

#include "test.type.h"

int visit_float(void *ctx, const char *key, float *f)
{
	printf("%s = %f\n", key ? key : "", *f);
	return 0;
}
int visit_(void *ctx, const char *key, void *value, size_t size)
{
	printf("%s = %02X\n", key ? key : "", value);
	return 0;
}

int main()
{
	//#define WRITE
	
	#ifdef WRITE
	FILE *fp = fopen("test.bin", "wb");
	#else
	FILE *fp = fopen("test.bin", "rb");
	#endif
	if(!fp)
		return 0;
	Player p;
	type_init(&p, k_ETypePlayer);
	
	#ifdef WRITE
	p.health = 123;
	p.origin.x = 3.14f;
	p.origin.y = 5.2f;
	p.origin.z = 10.123f;
	serialize(fp, &p);
	#else
	deserialize(fp, &p);
	#endif
	//printf("health=%d,origin=%f,%f,%f\n",p.health,p.origin.x,p.origin.y,p.origin.z);
	TypeFieldVisitor visitor = {0};
	visitor.visit = visit_;
	visitor.visit_float = visit_float;
	
	visit(&p, &visitor, NULL);
	fclose(fp);
	return 0;
}