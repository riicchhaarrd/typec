#pragma once
#include <stdio.h>
#include <stdint.h>

/* Data expects a 32-bit signed integer indicating the type of data. */

void deserialize_from_file(FILE *fp, void *data);
void serialize_to_file(FILE *fp, void *data);

void deserialize_from_buffer(char *buffer, int buffer_size, size_t *buffer_index, void *type_to_serialize, int32_t type_index, int include_type_field);
void serialize_to_buffer(char *buffer, int buffer_size, size_t *buffer_index, void *type_to_serialize, int include_type_field);