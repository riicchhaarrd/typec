#ifndef PARSE_TYPE_H
#define PARSE_TYPE_H

/*
	struct Player
	{
		int32 health;
	};
	
	enum DataType
	{
		Integer,
		Float,
		String
	};
	
	bitflags Mode
	{
		Read,
		Write,
		Execute
	};
*/

typedef struct Field_s
{
	char type[64];
	char name[64];
	char metadata[64];
	u64 type_hash;
	u64 name_hash;
	int replicated;
	s32 index;
	struct Field_s *next;
} Field;

typedef enum
{
	k_ETypeEntryTypeStruct,
	k_ETypeEntryTypeMessage,
	k_ETypeEntryTypeComponent,
	k_ETypeEntryTypeEnum,
	k_ETypeEntryTypeFlags,
	k_ETypeEntryTypeMax
} k_ETypeEntryType;


int entry_type_is_structure(k_ETypeEntryType entry_type)
{
	switch(entry_type)
	{		
		case k_ETypeEntryTypeStruct:
		case k_ETypeEntryTypeMessage:
		case k_ETypeEntryTypeComponent:
			return 1;
	}
	return 0;
}

static const char *entry_type_to_string(k_ETypeEntryType type)
{
	switch(type)
	{
		case k_ETypeEntryTypeStruct: return "struct";
		case k_ETypeEntryTypeMessage: return "message";
		case k_ETypeEntryTypeComponent: return "component";
		case k_ETypeEntryTypeEnum: return "enum";
		case k_ETypeEntryTypeFlags: return "flags";
	}
	return "?";
}

typedef enum
{
	k_ETypeEntryVisibilityPrivate,
	k_ETypeEntryVisibilityPublic
} k_ETypeEntryVisibility;

typedef struct TypeEntry_s
{
	struct TypeEntry_s *next;
	k_ETypeEntryType entry_type;
	char name[64];
	k_ETypeEntryVisibility visibility;
	s32 index;
	Field *fields;
	s32 type_index;
} TypeEntry;

/*
Too much work for now, maybe later
*/
/*
typedef struct
{
	k_EDataTypeInt,
	k_EDataTypeFloat,
	k_EDataTypeString,
	k_EDataTypeHash
} k_EDataType;
*/

typedef struct DataType_s
{
	const char *name;
	u64 hash;
	//k_EDataType type;
	//s32 bit_count; //e.g vec3 (float) is 32 bits, 0 means compiler/architecture specific
	//s32 element_count; //e.g vec3 has 3 elements, default 1, -1 variable length
	void (*serialize)(struct DataType_s *dt, TypeEntry *entry, Field *field);
	void (*deserialize)(struct DataType_s *dt, TypeEntry *entry, Field *field);
	void (*definition)(struct DataType_s *dt, TypeEntry *entry, Field *field);
} DataType;

void data_type_string_definition(struct DataType_s *dt, TypeEntry *entry, Field *field)
{
	printf("\tchar *%s", field->name);
	printf(";\n\tint32 %s_length", field->name);
}
void data_type_string_serialize(struct DataType_s *dt, TypeEntry *entry, Field *field)
{
}
void data_type_string_deserialize(struct DataType_s *dt, TypeEntry *entry, Field *field)
{
}

void data_type_primitive_definition(struct DataType_s *dt, TypeEntry *entry, Field *field)
{
	printf("\t%s %s", dt->name, field->name);
}
void data_type_primitive_serialize(struct DataType_s *dt, TypeEntry *entry, Field *field)
{
}
void data_type_primitive_deserialize(struct DataType_s *dt, TypeEntry *entry, Field *field)
{
}

static DataType data_types[] = {
	{"int", 0, data_type_primitive_serialize, data_type_primitive_deserialize, data_type_primitive_definition},
	{"char", 0, data_type_primitive_serialize, data_type_primitive_deserialize, data_type_primitive_definition},
	{"short", 0, data_type_primitive_serialize, data_type_primitive_deserialize, data_type_primitive_definition},
	{"long", 0, data_type_primitive_serialize, data_type_primitive_deserialize, data_type_primitive_definition},
	{"int8", 0, data_type_primitive_serialize, data_type_primitive_deserialize, data_type_primitive_definition},
	{"int16", 0, data_type_primitive_serialize, data_type_primitive_deserialize, data_type_primitive_definition},
	{"int32", 0, data_type_primitive_serialize, data_type_primitive_deserialize, data_type_primitive_definition},
	{"int64", 0, data_type_primitive_serialize, data_type_primitive_deserialize, data_type_primitive_definition},
	{"uint8", 0, data_type_primitive_serialize, data_type_primitive_deserialize, data_type_primitive_definition},
	{"uint16", 0, data_type_primitive_serialize, data_type_primitive_deserialize, data_type_primitive_definition},
	{"uint32", 0, data_type_primitive_serialize, data_type_primitive_deserialize, data_type_primitive_definition},
	{"uint64", 0, data_type_primitive_serialize, data_type_primitive_deserialize, data_type_primitive_definition},
	{"float", 0, data_type_primitive_serialize, data_type_primitive_deserialize, data_type_primitive_definition},
	{"float32", 0, data_type_primitive_serialize, data_type_primitive_deserialize, data_type_primitive_definition},
	{"float64", 0, data_type_primitive_serialize, data_type_primitive_deserialize, data_type_primitive_definition},
	{"double", 0, data_type_primitive_serialize, data_type_primitive_deserialize, data_type_primitive_definition},
	{"vec2", 0, data_type_primitive_serialize, data_type_primitive_deserialize, data_type_primitive_definition},
	{"vec3", 0, data_type_primitive_serialize, data_type_primitive_deserialize, data_type_primitive_definition},
	{"vec4", 0, data_type_primitive_serialize, data_type_primitive_deserialize, data_type_primitive_definition},
	{"quat", 0, data_type_primitive_serialize, data_type_primitive_deserialize, data_type_primitive_definition},
	{"mat3", 0, data_type_primitive_serialize, data_type_primitive_deserialize, data_type_primitive_definition},
	{"mat4", 0, data_type_primitive_serialize, data_type_primitive_deserialize, data_type_primitive_definition},
	{"string", 0, data_type_string_serialize, data_type_string_deserialize, data_type_string_definition},
	{"hash", 0, data_type_primitive_serialize, data_type_primitive_deserialize, data_type_primitive_definition},
	{NULL, 0, 0, 0, 0}
};
/*
static const char *data_types[] = {
	"int", "char", "short", "long",
	// "byte",
	"int8", "int16", "int32", "int64", "uint8", "uint16", "uint32", "uint64",
	// "i8", "i16", "i32", "i64", "u8", "u16", "u32", "u64",
	"float", "double",
	"vec2", "vec3", "vec4", "quat", "mat3", "mat4",
	// "f32", "f64",
	"string", "hash",
	NULL
};
*/

//static u64 data_type_hashes[sizeof(data_types) / sizeof(data_types[0])];

void initialize_data_type_hashes()
{
	for(s32 i = 0; data_types[i].name; ++i)
	{
		data_types[i].hash = fnv1a_64(data_types[i].name);
	}
}

int data_type_index_by_hash(u64 hash)
{
	for(s32 i = 0; data_types[i].name; ++i)
	{
		if(data_types[i].hash == hash)
			return i;
	}
	return -1;
}

void parse_type(Lexer *lexer, TypeEntry **entries_out)
{
	
	s32 entry_type_index[k_ETypeEntryTypeMax] = {0};
	
	Token t;
	s32 type_index = 0;
	u64 id_enum = fnv1a_64("enum");
	u64 id_struct = fnv1a_64("struct");
	u64 id_bitflags = fnv1a_64("bitflags");
	u64 id_replicated = fnv1a_64("replicated");
	u64 id_public = fnv1a_64("public");
	u64 id_private = fnv1a_64("private");
	u64 id_component = fnv1a_64("component");
	u64 id_message = fnv1a_64("message");
	//char temp[64];
	TypeEntry *entries = NULL;
	char index_str[64];
	while(!lexer_step(lexer, &t))
	{
		switch(t.token_type)
		{
			default:
				lexer_error(lexer, "Expected identifier");
			break;
			case k_ETokenTypeIdentifier:
			{
				int replicated = 0;
				
				TypeEntry *entry = new(lexer->arena, TypeEntry, 1);
				if(!entries)
				{
					*entries_out = entries = entry;
				} else {
					entries->next = entry;
					entries = entry;
				}
				entry->index = type_index++;
				entry->next = NULL;
				entry->visibility = k_ETypeEntryVisibilityPrivate;
				entry->name[0] = 0;
				entry->fields = NULL;
				
				if(t.hash == id_public || t.hash == id_private)
				{
					entry->visibility = t.hash == id_public ? k_ETypeEntryVisibilityPublic : k_ETypeEntryVisibilityPrivate;
					lexer_expect(lexer, k_ETokenTypeIdentifier, &t);
				}
				
				u64 id_hash = t.hash;
				Token name;
				lexer_expect(lexer, k_ETokenTypeIdentifier, &name);
				lexer_expect(lexer, '{', NULL);
				lexer_token_read_string(lexer, &name, entry->name, sizeof(entry->name));
				//printf("%s\n", temp);
				if(id_hash == id_struct || id_hash == id_component || id_hash == id_message)
				{
					if(id_hash == id_struct)
					{
						entry->entry_type = k_ETypeEntryTypeStruct;
					}
					else if(id_hash == id_component)
					{
						entry->entry_type = k_ETypeEntryTypeComponent;
					} else if(id_hash == id_message)
					{
						entry->entry_type = k_ETypeEntryTypeMessage;
					}
					Field *fields = NULL;
					while(1)
					{
						if(!lexer_accept(lexer, '}', NULL))
							break;

						Token field_type, field_name;
						lexer_expect(lexer, k_ETokenTypeIdentifier, &field_type);
						if(field_type.hash == id_replicated)
						{
							replicated = 1;
							lexer_expect(lexer, ':', NULL);
						} else {
							Field *field = new(lexer->arena, Field, 1);
							field->next = NULL;
							field->index = -1;
							field->replicated = replicated;
							field->type_hash = field_type.hash;
							s32 dt_index = data_type_index_by_hash(field_type.hash);
							if(dt_index == -1)
							{
								for(s32 k = 0; data_types[k].name; ++k)
								{
									printf("%s, ", data_types[k].name);
								}
								printf("\n");
								lexer_error(lexer, "Field type must be one of the following above.");
							}
							lexer_expect(lexer, k_ETokenTypeIdentifier, &field_name);
							field->name_hash = field_name.hash;
							lexer_token_read_string(lexer, &field_type, field->type, sizeof(field->type));
							//printf("field_type: %s\n", field->type);
							lexer_token_read_string(lexer, &field_name, field->name, sizeof(field->name));
							//printf("field_name: %s\n", field->name);
							
							if(!lexer_accept(lexer, '@', NULL))
							{
								Token keyword;
								lexer_expect(lexer, k_ETokenTypeIdentifier, &keyword);
								lexer_token_read_string(lexer, &keyword, field->metadata, sizeof(field->metadata));
							}
							if(!lexer_accept(lexer, '%', NULL))
							{
								Token index_token;
								lexer_expect(lexer, k_ETokenTypeNumber, &index_token);
								lexer_token_read_string(lexer, &index_token, index_str, sizeof(index_str));
								field->index = atoi(index_str);
							}
							
							if(!fields)
							{
								entry->fields = fields = field;
							} else {
								fields->next = field;
								fields = field;
							}
							lexer_expect(lexer, ';', NULL);
						}
					}
				} else if(id_hash == id_enum || id_hash == id_bitflags)
				{
					entry->entry_type = id_hash == id_enum ? k_ETypeEntryTypeEnum : k_ETypeEntryTypeFlags;
					Field *fields = NULL;
					while(1)
					{
						if(!lexer_accept(lexer, '}', NULL))
							break;
						Field *field = new(lexer->arena, Field, 1);
						field->next = NULL;
						field->index = -1;
						field->replicated = replicated;
						field->type[0] = 0;
						Token enum_value;
						lexer_expect(lexer, k_ETokenTypeIdentifier, &enum_value);
						lexer_token_read_string(lexer, &enum_value, field->name, sizeof(field->name));
						//printf("enum value: %s\n", temp);
						
						if(!fields)
						{
							entry->fields = fields = field;
						} else {
							fields->next = field;
							fields = field;
						}
						
						if(!lexer_accept(lexer, '}', NULL))
							break;
						lexer_expect(lexer, ',', NULL);
					}
				} else {
					lexer_error(lexer, "Expected struct, enum or bitflags");
				}
				entry->type_index = entry_type_index[entry->entry_type]++;
				lexer_accept(lexer, ';', NULL);
			} break;
		}
	}
}

void write_definitions(TypeEntry *entries)
{
	while(entries)
	{
		switch(entries->entry_type)
		{
			case k_ETypeEntryTypeEnum:
			case k_ETypeEntryTypeFlags:
				printf("typedef enum\n{\n");
			break;
			case k_ETypeEntryTypeStruct:
			case k_ETypeEntryTypeMessage:
			case k_ETypeEntryTypeComponent:
				printf("typedef struct\n{\n");
				printf("\t// BEGIN OF STRUCT META DATA\n");
				printf("\tint32_t type_;\n");
				//printf("\tuint64_t hash_;\n");
				printf("\t// END OF STRUCT META DATA\n");
			break;
		}
		Field *fields = entries->fields;
		s32 field_index = 0;
		while(fields)
		{
			switch(entries->entry_type)
			{
				case k_ETypeEntryTypeEnum:
					printf("\tk_E%s%s", entries->name, fields->name);
					if(fields->next)
						printf(",\n");
					else
						printf("\n");
				break;
				case k_ETypeEntryTypeFlags:
					printf("\tk_E%s%s = 0x%02X", entries->name, fields->name, (1 << field_index));
					if(fields->next)
						printf(",\n");
					else
						printf("\n");
				break;
				default:
				{
					if(entry_type_is_structure(entries->entry_type))
					{
						//printf("\t%s", fields->name);
						s32 dt_index = data_type_index_by_hash(fields->type_hash);
						DataType *dt = &data_types[dt_index];
						dt->definition(dt, entries, fields);
						printf(";\n");
					}
				} break;
			}
			++field_index;
			fields = fields->next;
		}
		switch(entries->entry_type)
		{
			case k_ETypeEntryTypeEnum:
			case k_ETypeEntryTypeFlags:
				printf("} k_E%s;\n", entries->name);
			break;
			default:
				if(entry_type_is_structure(entries->entry_type))
				{
					printf("} %s; //%s %d, total index: %d\n", entries->name, entry_type_to_string(entries->entry_type), entries->type_index, entries->index);
				}
			break;
		}
		entries = entries->next;
	}
}

void write_serialize_entry(TypeEntry *it)
{
	if(entry_type_is_structure(it->entry_type))
	{
		Field *fields;
		fields = it->fields;
		printf("void vt_fn_serialize_type_%d_(void *data, FILE *fp, int writing)\n{\n", it->index);
		printf("\t%s *inst = (%s*)data;\n", it->name, it->name);
		while(fields)
		{
			printf("\tif(writing)\n\t{\n");
			printf("\t\tfwrite(&inst->%s, sizeof(inst->%s), 1, fp);\n", fields->name, fields->name);
			printf("\t}\n\telse\n\t{\n");
			printf("\t\tfread(&inst->%s, sizeof(inst->%s), 1, fp);\n", fields->name, fields->name);
			printf("\t}\n");
			fields = fields->next;
		}
		printf("}\n");
		fields = it->fields;
		printf("void vt_fn_initialize_type_%d_(void *data)\n{\n", it->index);
		printf("\t%s *inst = (%s*)data;\n", it->name, it->name);
		printf("\tinst->type_ = %d;\n", it->index);
		printf("}\n");
		fields = it->fields;
		//printf("int vt_fn_visit_type_%d_(void *data, int (*visit_field)(void *ctx, const char *key, void *value, k_EPrimitiveType field_type), void *ctx)\n{\n", it->index);
		printf("int vt_fn_visit_type_%d_(void *data, TypeFieldVisitor *visitor, void *ctx)\n{\n", it->index);
		printf("\t%s *inst = (%s*)data;\n\tint changed_count = 0;\n", it->name, it->name);
		while(fields)
		{
			s32 dt_index = data_type_index_by_hash(fields->type_hash);
			if(dt_index != -1)
			{
				printf("\tif(visitor->visit_%s)\n\t{\n", data_types[dt_index].name);
				if(it->visibility == k_ETypeEntryVisibilityPrivate)
				{
					printf("\t\tchanged_count += visitor->visit_%s(ctx, NULL, (%s*)&inst->%s);\n", data_types[dt_index].name, data_types[dt_index].name, fields->name);
				} else {
					printf("\t\tchanged_count += visitor->visit_%s(ctx, \"%s\", (%s*)&inst->%s);\n", data_types[dt_index].name, fields->name, data_types[dt_index].name, fields->name);
				}
				printf("\t}\n\telse if(visitor->visit)\n\t{\n");
				if(it->visibility == k_ETypeEntryVisibilityPrivate)
				{
					printf("\t\tchanged_count += visitor->visit(ctx, NULL, (void*)&inst->%s, sizeof(%s));\n", fields->name, data_types[dt_index].name);
				} else {
					printf("\t\tchanged_count += visitor->visit(ctx, \"%s\", (void*)&inst->%s, sizeof(%s));\n", fields->name, fields->name, data_types[dt_index].name);
				}
				printf("\t}\n");
			}
			fields = fields->next;
		}
		printf("\treturn changed_count;\n}\n");
	}
}

void write_serialize_table(TypeEntry *entries)
{
printf(R"(

typedef void (*SerializeFn)(void *instance, FILE *fp, int writing);
//typedef int (*VisitFn)(void *instance, int (*visit_field)(void *ctx, const char *key, void *value), void *ctx);
typedef int (*VisitFn)(void *instance, TypeFieldVisitor *visitor, void *ctx);
typedef void (*InitializeFn)(void *instance);
typedef struct
{
	SerializeFn serialize;
	VisitFn visit;
	InitializeFn initialize;
	//void (*serialize)(void *instance, FILE *fp, int writing);
	//int (*visit)(void *instance, int (*visit_field)(void *ctx, const char *key, void *value), void *ctx);
	//void (*initialize)(void *instance);
	size_t size;
} VTableEntry;	
)");
	printf("static const VTableEntry vtable_entries[] = {\n");
	while(entries)
	{
		if(entry_type_is_structure(entries->entry_type))
		{
			printf("\t{(SerializeFn)vt_fn_serialize_type_%d_, (VisitFn)vt_fn_visit_type_%d_, (InitializeFn)vt_fn_initialize_type_%d_, sizeof(%s)},\n", entries->index, entries->index, entries->index, entries->name);
		} else {
			printf("\t{0, 0},\n");
		}
		entries = entries->next;
	}
	printf("\t0\n};\n");
	printf(R"(
void serialize(FILE *fp, void *data)
{
	int32_t type_index = *(int32_t*)data;
	vtable_entries[type_index].serialize(data, fp, 1);
}
void deserialize(FILE *fp, void *data)
{
	int32_t type_index = *(int32_t*)data;
	vtable_entries[type_index].serialize(data, fp, 0);
}
//int visit(void *data, int (*visit_field)(void *ctx, const char *key, void *value), void *ctx)
int visit(void *data, TypeFieldVisitor *visitor, void *ctx)
{
	int32_t type_index = *(int32_t*)data;
	return vtable_entries[type_index].visit(data, visitor, ctx);
}
size_t type_sizeof(int32_t type_index)
{
	return vtable_entries[type_index].size;
}
void type_init(void *data, int32_t type_index)
{
	vtable_entries[type_index].initialize(data);
}
)");
}
void write_reflection(TypeEntry *entries)
{
	
	for(s32 i = 0; data_types[i].name; ++i)
	{
		printf("int refl_field_visit_data_type_%s(void *ctx, const char *key, void *value);\n", data_types[i].name);
	}
	
	printf(R"(	
typedef struct ReflStructField_s
{
	const char *name;
	const char *type;
	size_t offset;
	int (*visit)(void *ctx, const char *key, void *value); // returns 1 if visitor changed value, returns 0 if nothing changed
} ReflStructField;

typedef struct ReflEnumValue_s
{
	const char *name;
	uint32_t value;
} ReflEnumValue;

)");
	TypeEntry *it = entries;

	while(it)
	{
		if(it->entry_type == k_ETypeEntryTypeStruct)
		{
			printf("static const ReflStructField %s_refl_fields[] = {\n", it->name);
			Field *fields = it->fields;
			s32 field_index = 0;
			while(fields)
			{
				printf("\t{\"%s\", \"%s\", offsetof(%s, %s), refl_field_visit_data_type_%s}",
					fields->name,
					fields->type,
					it->name, fields->name,
					fields->type,
					it->name,
					fields->name
				);
				if(fields->next)
					printf(",\n");
				else
					printf("\n");
				++field_index;
				fields = fields->next;
			}
			printf("};\n");
		} else if(it->entry_type == k_ETypeEntryTypeEnum || it->entry_type == k_ETypeEntryTypeFlags)
		{
			printf("static const ReflEnumValue %s_refl_values[] = {\n", it->name);
			Field *fields = it->fields;
			s32 field_index = 0;
			while(fields)
			{
				if(it->entry_type == k_ETypeEntryTypeEnum)
				{
					printf("\t{\"%s\", %d}",
						fields->name,
						field_index
					);
				} else if(it->entry_type == k_ETypeEntryTypeFlags)
				{
					printf("\t{\"%s\", %d}",
						fields->name,
						(1 << field_index)
					);
				}
				if(fields->next)
					printf(",\n");
				else
					printf("\n");
				++field_index;
				fields = fields->next;
			}
			printf("};\n");
		}
		it = it->next;
	}
}

void parse_type_file(const char *path, Arena *arena, CompilerOptions *opts)
{
	initialize_data_type_hashes();
	
	FILE *fp = fopen(path, "rb");
	if(!fp)
		return;

	Lexer lexer = { 0 };

	LexerStream ls = { 0 };
	lexer_stream_init_file(&ls, fp);
	lexer_init(&lexer, arena, &ls);

	TypeEntry *entries;
	parse_type(&lexer, &entries);
	if(!strcmp(opts->mode, "c") || !strcmp(opts->mode, "c++"))
	{
		printf("#ifndef FIXME_H\n#define FIXME_H\n");
		printf("#include <stdint.h>\n");
		printf("#include <stdio.h>\n");
		printf("#include <stddef.h>\n");
		printf("\n");
		write_definitions(entries);
		//write_reflection(entries);
		
		{
			TypeEntry *it = entries;

			printf("typedef enum\n{\n");
			while(it)
			{
				printf("\tk_EType%s,\n", it->name);
				it = it->next;
			}
			printf("} k_EType;\n");
		}
		{
			printf("typedef struct\n{\n");
			for(s32 i = 0; data_types[i].name; ++i)
			{
				printf("\tint (*visit_%s)(void *ctx, const char *key, %s *value);\n", data_types[i].name, data_types[i].name);
			}
			printf("\tint (*visit)(void *ctx, const char *key, void *value, size_t size); // Fallback in case visit_X is NULL\n");
			printf("} TypeFieldVisitor;\n");
		}
		#if 0
		{
			printf("typedef enum\n{\n");
			for(s32 i = 0; data_types[i].name; ++i)
			{
				printf("\tk_EPrimitiveType%c%s,\n", data_types[i].name[0] - 'a' + 'A', data_types[i].name + 1);
			}
			printf("} k_EPrimitiveType;\n");
		}
		#endif
		{
			TypeEntry *it = entries;

			while(it)
			{
				write_serialize_entry(it);
				it = it->next;
			}
			write_serialize_table(entries);
		}
		printf("#endif\n");
	} else {
		printf("Unsupported mode '%s'\n", opts->mode);
	}
	fclose(fp);
}
#endif