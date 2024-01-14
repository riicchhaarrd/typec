
#include <stdint.h>
typedef uint32_t uint32;
typedef int32_t int32;
typedef struct
{
	float x, y, z;
} vec3;

#include "test.type.h"

int visitor(void *ctx, const char *key, void *value)
{
	printf("%s = %d\n", key ? key : "", *(int32_t*)value);
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
	printf("health=%d,origin=%f,%f,%f\n",p.health,p.origin.x,p.origin.y,p.origin.z);
	
	//visit(&p, visitor, NULL);
	fclose(fp);
	return 0;
}