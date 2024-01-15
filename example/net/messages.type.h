#ifndef FIXME_H
#define FIXME_H
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>

typedef struct
{
	int32_t type_; // Struct metadata, DO NOT REMOVE!
	char name[32];
	int some_option;
	int some_secret_key;
} MsgConnect; //message 0, total index: 0

typedef struct
{
	int32_t type_; // Struct metadata, DO NOT REMOVE!
	char message[256];
} MsgText; //message 1, total index: 1

typedef struct
{
	int32_t type_; // Struct metadata, DO NOT REMOVE!
	char reason[32];
} MsgDisconnect; //message 2, total index: 2

typedef struct
{
	int32_t type_; // Struct metadata, DO NOT REMOVE!
	int16 source_port;
	int32 sequence_number;
	int32 acknowledge_number;
	int32 time;
	int32 message_count;
} Packet; //message 3, total index: 3

typedef enum
{
	k_ETypeMsgConnect = 0,
	k_ETypeMsgText = 1,
	k_ETypeMsgDisconnect = 2,
	k_ETypePacket = 3,
} k_EType;

typedef struct
{
	int (*visit_int)(void *ctx, const char *key, int *value, size_t num_elements);
	int (*visit_char)(void *ctx, const char *key, char *value, size_t num_elements);
	int (*visit_short)(void *ctx, const char *key, short *value, size_t num_elements);
	int (*visit_long)(void *ctx, const char *key, long *value, size_t num_elements);
	int (*visit_int8)(void *ctx, const char *key, int8 *value, size_t num_elements);
	int (*visit_int16)(void *ctx, const char *key, int16 *value, size_t num_elements);
	int (*visit_int32)(void *ctx, const char *key, int32 *value, size_t num_elements);
	int (*visit_int64)(void *ctx, const char *key, int64 *value, size_t num_elements);
	int (*visit_uint8)(void *ctx, const char *key, uint8 *value, size_t num_elements);
	int (*visit_uint16)(void *ctx, const char *key, uint16 *value, size_t num_elements);
	int (*visit_uint32)(void *ctx, const char *key, uint32 *value, size_t num_elements);
	int (*visit_uint64)(void *ctx, const char *key, uint64 *value, size_t num_elements);
	int (*visit_float)(void *ctx, const char *key, float *value, size_t num_elements);
	int (*visit_float32)(void *ctx, const char *key, float32 *value, size_t num_elements);
	int (*visit_float64)(void *ctx, const char *key, float64 *value, size_t num_elements);
	int (*visit_double)(void *ctx, const char *key, double *value, size_t num_elements);
	int (*visit_vec2)(void *ctx, const char *key, vec2 *value, size_t num_elements);
	int (*visit_vec3)(void *ctx, const char *key, vec3 *value, size_t num_elements);
	int (*visit_vec4)(void *ctx, const char *key, vec4 *value, size_t num_elements);
	int (*visit_quat)(void *ctx, const char *key, quat *value, size_t num_elements);
	int (*visit_mat3)(void *ctx, const char *key, mat3 *value, size_t num_elements);
	int (*visit_mat4)(void *ctx, const char *key, mat4 *value, size_t num_elements);
	int (*visit_string)(void *ctx, const char *key, string *value, size_t num_elements);
	int (*visit_hash)(void *ctx, const char *key, hash *value, size_t num_elements);
	int (*visit)(void *ctx, const char *key, void *value, size_t size, size_t num_elements); // Fallback in case visit_X is NULL
} TypeFieldVisitor;

static void vt_fn_initialize_type_0_(void *data)
{
	MsgConnect *inst = (MsgConnect*)data;
	inst->type_ = 0;
}

static int vt_fn_visit_type_0_(void *data, TypeFieldVisitor *visitor, void *ctx)
{
	MsgConnect *inst = (MsgConnect*)data;
	int changed_count = 0;
	if(visitor->visit_char)
	{
		changed_count += visitor->visit_char(ctx, "name", (char*)&inst->name[0], 32);
	}
	else if(visitor->visit)
	{
		changed_count += visitor->visit(ctx, "name", (void*)&inst->name[0], sizeof(char), 32);
	}
	if(visitor->visit_int)
	{
		changed_count += visitor->visit_int(ctx, NULL, (int*)&inst->some_option, 1);
	}
	else if(visitor->visit)
	{
		changed_count += visitor->visit(ctx, NULL, (void*)&inst->some_option, sizeof(int), 1);
	}
	return changed_count;
}

static void vt_fn_initialize_type_1_(void *data)
{
	MsgText *inst = (MsgText*)data;
	inst->type_ = 1;
}

static int vt_fn_visit_type_1_(void *data, TypeFieldVisitor *visitor, void *ctx)
{
	MsgText *inst = (MsgText*)data;
	int changed_count = 0;
	return changed_count;
}

static void vt_fn_initialize_type_2_(void *data)
{
	MsgDisconnect *inst = (MsgDisconnect*)data;
	inst->type_ = 2;
}

static int vt_fn_visit_type_2_(void *data, TypeFieldVisitor *visitor, void *ctx)
{
	MsgDisconnect *inst = (MsgDisconnect*)data;
	int changed_count = 0;
	return changed_count;
}

static void vt_fn_initialize_type_3_(void *data)
{
	Packet *inst = (Packet*)data;
	inst->type_ = 3;
}

static int vt_fn_visit_type_3_(void *data, TypeFieldVisitor *visitor, void *ctx)
{
	Packet *inst = (Packet*)data;
	int changed_count = 0;
	if(visitor->visit_int16)
	{
		changed_count += visitor->visit_int16(ctx, NULL, (int16*)&inst->source_port, 1);
	}
	else if(visitor->visit)
	{
		changed_count += visitor->visit(ctx, NULL, (void*)&inst->source_port, sizeof(int16), 1);
	}
	if(visitor->visit_int32)
	{
		changed_count += visitor->visit_int32(ctx, NULL, (int32*)&inst->sequence_number, 1);
	}
	else if(visitor->visit)
	{
		changed_count += visitor->visit(ctx, NULL, (void*)&inst->sequence_number, sizeof(int32), 1);
	}
	if(visitor->visit_int32)
	{
		changed_count += visitor->visit_int32(ctx, NULL, (int32*)&inst->acknowledge_number, 1);
	}
	else if(visitor->visit)
	{
		changed_count += visitor->visit(ctx, NULL, (void*)&inst->acknowledge_number, sizeof(int32), 1);
	}
	if(visitor->visit_int32)
	{
		changed_count += visitor->visit_int32(ctx, NULL, (int32*)&inst->time, 1);
	}
	else if(visitor->visit)
	{
		changed_count += visitor->visit(ctx, NULL, (void*)&inst->time, sizeof(int32), 1);
	}
	if(visitor->visit_int32)
	{
		changed_count += visitor->visit_int32(ctx, NULL, (int32*)&inst->message_count, 1);
	}
	else if(visitor->visit)
	{
		changed_count += visitor->visit(ctx, NULL, (void*)&inst->message_count, sizeof(int32), 1);
	}
	return changed_count;
}


typedef int (*VisitFn)(void *instance, TypeFieldVisitor *visitor, void *ctx);
typedef void (*InitializeFn)(void *instance);
typedef struct
{
	const char *name;
	VisitFn visit;
	InitializeFn initialize;
	size_t size;
} VTableEntry;

static const VTableEntry vtable_entries[] = {
	{"MsgConnect", (VisitFn)vt_fn_visit_type_0_, (InitializeFn)vt_fn_initialize_type_0_, sizeof(MsgConnect)},
	{"MsgText", (VisitFn)vt_fn_visit_type_1_, (InitializeFn)vt_fn_initialize_type_1_, sizeof(MsgText)},
	{"MsgDisconnect", (VisitFn)vt_fn_visit_type_2_, (InitializeFn)vt_fn_initialize_type_2_, sizeof(MsgDisconnect)},
	{NULL, (VisitFn)vt_fn_visit_type_3_, (InitializeFn)vt_fn_initialize_type_3_, sizeof(Packet)},
	{0, 0, 0}
};

static int visit(void *data, TypeFieldVisitor *visitor, void *ctx)
{
	int32_t type_index = *(int32_t*)data;
	int32_t max_entries = sizeof(vtable_entries) / sizeof(vtable_entries[0]);
	if(type_index < 0 || type_index >= max_entries)
		return 0;
	return vtable_entries[type_index].visit(data, visitor, ctx);
}
static int type_visit(void *data, TypeFieldVisitor *visitor, void *ctx, int32_t type_index)
{
	int32_t max_entries = sizeof(vtable_entries) / sizeof(vtable_entries[0]);
	if(type_index < 0 || type_index >= max_entries)
		return 0;
	return vtable_entries[type_index].visit(data, visitor, ctx);
}
static size_t type_sizeof(int32_t type_index)
{
	int32_t max_entries = sizeof(vtable_entries) / sizeof(vtable_entries[0]);
	if(type_index < 0 || type_index >= max_entries)
		return 0;
	return vtable_entries[type_index].size;
}
static const char* type_name(int32_t type_index)
{
	int32_t max_entries = sizeof(vtable_entries) / sizeof(vtable_entries[0]);
	if(type_index < 0 || type_index >= max_entries)
		return NULL;
	return vtable_entries[type_index].name;
}
static int type_init(void *data, int32_t type_index)
{
	int32_t max_entries = sizeof(vtable_entries) / sizeof(vtable_entries[0]);
	if(type_index < 0 || type_index >= max_entries)
		return 1;
	vtable_entries[type_index].initialize(data);
	return 0;
}
#endif
