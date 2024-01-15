#include <string.h>

#include "serialize.h"
#include "types.h"
#include "messages.type.h"

/* Serializing to FILE* */
typedef struct
{
	FILE *fp;
	int writing;
	// TODO: check result of fread/fwrite and longjmp
	//jmp_buf jmp_error;
} SerializeContext;

int type_serialize_visit_field(void *ctx_, const char *key, void *value, size_t element_size, size_t num_elements)
{
	SerializeContext *ctx = (SerializeContext*)ctx_;
	if(ctx->writing)
	{
		fwrite(value, element_size, num_elements, ctx->fp);
		return 0;
	}
	fread(value, element_size, num_elements, ctx->fp);
	return 1;
}

void serialize_to_file(FILE *fp, void *data)
{
	SerializeContext ctx = {0};
	ctx.fp = fp;
	ctx.writing = 1;
	TypeFieldVisitor visitor = {0};
	visitor.visit = type_serialize_visit_field;
	visit(data, &visitor, &ctx);
}

void deserialize_from_file(FILE *fp, void *data)
{
	SerializeContext ctx = {0};
	ctx.fp = fp;
	ctx.writing = 0;
	TypeFieldVisitor visitor = {0};
	visitor.visit = type_serialize_visit_field;
	visit(data, &visitor, &ctx);
}

/* Serializing to buffer */

typedef struct Buffer_s
{
	char *data;
	size_t size;
	//size_t capacity;
	//size_t (*resize)(Buffer_s*, size_t);
} Buffer;

typedef struct
{
	Buffer buffer;
	size_t position;
	int writing;
} SerializeBufferContext;

int type_serialize_buffer_visit_field(void *ctx_, const char *key, void *value, size_t element_size, size_t num_elements)
{
	SerializeBufferContext *ctx = (SerializeBufferContext*)ctx_;
	size_t size = element_size * num_elements;
	if(ctx->writing)
	{
		if(ctx->position + size > ctx->buffer.size)
		{
			//Error: Out of bounds
			return 0;
		}
		memcpy(&ctx->buffer.data[ctx->position], value, size);
		ctx->position += size;
		return 0;
	}
	if(ctx->position + size > ctx->buffer.size)
	{
		//Error: Reading past EOF
		memset(value, 0, size);
		return 1;
	}
	memcpy(value, &ctx->buffer.data[ctx->position], size);
	ctx->position += size;
	return 1;
}

void serialize_to_buffer(char *buffer, int buffer_size, size_t *buffer_index, void *data_to_serialize, int include_type_field)
{
	if(buffer_size <= 0)
		return;
	SerializeBufferContext ctx = {0};
	ctx.buffer.data = buffer;
	ctx.buffer.size = buffer_size;
	ctx.writing = 1;
	ctx.position = *buffer_index;
	TypeFieldVisitor visitor = {0};
	visitor.visit = type_serialize_buffer_visit_field;
	if(include_type_field)
	{
		type_serialize_buffer_visit_field(&ctx, NULL, data_to_serialize, sizeof(int32_t), 1);
	}
	visit(data_to_serialize, &visitor, &ctx);
	*buffer_index = ctx.position;
}

void deserialize_from_buffer(char *buffer, int buffer_size, size_t *buffer_index, void *data_to_serialize, int32_t type_index, int include_type_field)
{
	if(buffer_size <= 0)
		return;
	SerializeBufferContext ctx = {0};
	ctx.buffer.data = buffer;
	ctx.buffer.size = buffer_size;
	ctx.writing = 0;
	ctx.position = *buffer_index;
	TypeFieldVisitor visitor = {0};
	visitor.visit = type_serialize_buffer_visit_field;
	if(include_type_field)
	{
		type_serialize_buffer_visit_field(&ctx, NULL, data_to_serialize, sizeof(int32_t), 1);
	}
	type_visit(data_to_serialize, &visitor, &ctx, type_index);
	*buffer_index = ctx.position;
}