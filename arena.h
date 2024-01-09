#ifndef ARENA_H
#define ARENA_H
#include <setjmp.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
	char *beg;
	char *end;
    jmp_buf jmp_oom;
} Arena;

void *arena_allocate_memory_(Arena *a, ptrdiff_t size, ptrdiff_t align, ptrdiff_t count)
{
    ptrdiff_t padding = -(uintptr_t)a->beg & (align - 1);
    ptrdiff_t available = a->end - a->beg - padding;
    if(available < 0 || count > available / size)
	{
		longjmp(a->jmp_oom, 1);
    }
    void *p = a->beg + padding;
    a->beg += padding + count * size;
    return memset(p, 0, count * size);
}
#define new(a, t, n)  (t *)arena_allocate_memory_(a, sizeof(t), _Alignof(t), n)

void arena_init(Arena *a, char *buffer, size_t size)
{
	a->beg = buffer;
	a->end = buffer + size;
	if(setjmp(a->jmp_oom))
	{
		printf("Out of memory!\n");
		exit(1);
	}
}

#endif