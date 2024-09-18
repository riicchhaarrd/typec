#ifndef PARSE_TYPE_H
#define PARSE_TYPE_H
#include <stdbool.h>
#include <inttypes.h>
#include <ctype.h>
#include "../stream_file.h"
#include "../stream_buffer.h"

// TODO
// - When encoding do the same as protobuf and have a key/index for each field first that maps to the location of the field in the file. With a small value indicating what type it is, e.g variable length/varint for future compatibility.
// - Implement ability to set display names / labels for fields.
// - Initial values for fields, it's why I opted to use '%' for specifying field order instead of using '='.
// - Pass through "native" types e.g HWND
// - Error checking for duplicate field and type names, indices etc.
// - Add varint and zigzag encoding
// - For fixed size arrays, maybe encode them as variable length but add a check whether that amount of data is actually present when decoded.
// - More agressive compression / packing for structs of type 'Message'
// - More padding/alignment fixing for structs of type 'Component'
// - Maybe a visit2/visit_general, that goes through each field but only visits general types like int, float, string and passes the number of bits, whether the type is signed or unsigned. (For now just map all specific visitors to a more general visitor in serialize.c)
// - Maybe apply a Transformer/Operator type function in order or with flags to fields so before serializing when compression/encryption is enabled it's first passed through these functions.
// - Bitfields or specifying bit count for types, e.g int7, int3 maybe with options to pad up to the next byte for next fields.
//   Variable length vs fixed bit fields might be tricky to implement, basically kinda like a buffer, char* again.
// - Add metadata to fields like this:
//   [Note("This is a note.")]
//	 int32 max_value;
//   Altough I'm not sure about how to apply it to multiple fields. Maybe use : like public, private and protected do but would be easy to miss and accidently apply metadata to a bunch of fields.
/*
	if(bit_count % 8 != 0 || bit_index % 8 != 0)
		write_bitstream(...)
	else
		memcpy(...)
*/

/*
	Maybe try to make the usage of iterating types like:
	
	for(int i = 0; i < type_member_count(&type); ++i)
	{
		void *mem_ptr = type_member_get(&type, i);
		bool b = type_member_info(&type, i, &mem_info)
		int n = type_member_to_string(&type, i, str, sizeof(str));
		int err = type_member_set_from_string(&type, i, str);
	}
*/

// Enable generic visitor to build absolute key path and fix keys for arrays

typedef enum
{
	k_EVisibilityPrivate,
	k_EVisibilityProtected,
	k_EVisibilityPublic
} k_EVisibility;
typedef enum
{
	k_EFieldDataTypePrimitive,
	k_EFieldDataTypeCustom,
	k_EFieldDataTypeForward //TODO: forward declared type, e.g HWND
} k_EFieldDataType;

typedef struct Field_s
{
	k_EFieldDataType data_type;
	char type[64];
	char name[64];
	char metadata[64];
	u64 type_hash;
	u64 name_hash;
	int replicated;
	s32 index;
	k_EVisibility visibility;
	struct Field_s *next;
	s32 element_count; //e.g vec3 has 3 elements, default 1, -1 variable length
	char *initial_value;
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

typedef struct TypeEntry_s
{
	struct TypeEntry_s *next;
	k_ETypeEntryType entry_type;
	char name[256];
	k_EVisibility visibility;
	s32 index;
	Field *fields;
	s32 type_index;
	bool referenced_as_field;
	u32 name_hash;
} TypeEntry;

typedef enum
{
	k_EDataTypeCustom = 1,
	k_EDataTypeInt,
	k_EDataTypeUInt,
	k_EDataTypeFloat,
	k_EDataTypeString,
	k_EDataTypeChar
	//k_EDataTypeHash
} k_EDataType;

typedef struct DataType_s
{
	const char *name;
	u64 hash;
	k_EDataType type;
	s32 bit_count; //e.g vec3 (float) is 32 bits, 0 means compiler/architecture specific
	s32 element_count; //e.g vec3 has 3 elements, default 1, -1 variable length
	void (*serialize)(struct DataType_s *dt, TypeEntry *entry, Field *field);
	void (*deserialize)(struct DataType_s *dt, TypeEntry *entry, Field *field);
	void (*definition)(struct DataType_s *dt, TypeEntry *entry, Field *field);
} DataType;

typedef struct ForwardedType_s
{
	char name[64];
	struct ForwardedType_s *next;
} ForwardedType;

#define ITERATE_LIST(type, head, iterator) \
    for (type* iterator = head; iterator != NULL; iterator = iterator->next)

static size_t entry_visit_count(TypeEntry *entry)
{
	size_t n = 0;
	ITERATE_LIST(Field, entry->fields, it)
	{
		if(it->visibility != k_EVisibilityPrivate)
		{
			++n;
		}
	}
	return n;
}

static char type_prefix[128];

static void string_constant_case(char *str, char *out, size_t n)
{
	size_t i = 0;
	for(char *p = str; *p; ++p)
	{
		if(i + 1 >= n)
			break;
		if(p != str && (*p >= 'A' && *p <='Z') && !isupper(*(p + 1)))
		{
			out[i++] = '_';
		}
		if(i + 1 >= n)
			break;
		out[i++] = toupper(*p);
	}
	out[i] = 0;
}

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
}

void write_field_definition(Field *field)
{
	if(field->element_count == 1)
	{
		printf("\t%s %s", field->type, field->name);
	} else if(field->element_count == -1)
	{
		printf("\tsize_t num%s;\n", field->name);
		printf("\t%s *%s", field->type, field->name);
	} else if(field->element_count == -2)
	{
		printf("\t%s *%s", field->type, field->name);
	} else {
		printf("\t%s %s[%d]", field->type, field->name, field->element_count);
	}
}
void data_type_primitive_serialize(struct DataType_s *dt, TypeEntry *entry, Field *field)
{
}
void data_type_primitive_deserialize(struct DataType_s *dt, TypeEntry *entry, Field *field)
{
}
/*
	https://c-faq.com/charstring/wchar.html
	
	Well... I think that for 99% of the cases that a char/byte is 8-bits.
	And at the moment I'm just simply writing some C meta/type/reflection helper functions.
	I don't really wanna bother with it, so char is a exception which we'll define as a type instead of "forwarding" along.
	We'll just use UTF-8 like any other _sane_ person would do and say a char is always 8-bits.
*/
static DataType data_types[] = {
	//{"int", 0, k_EDataTypeInt, 0, 1, data_type_primitive_serialize, data_type_primitive_deserialize, data_type_primitive_definition},
	//{"size_t", 0, k_EDataTypeInt, 0, 1, data_type_primitive_serialize, data_type_primitive_deserialize, data_type_primitive_definition},
	//{"ptrdiff_t", 0, k_EDataTypeInt, 0, 1, data_type_primitive_serialize, data_type_primitive_deserialize, data_type_primitive_definition},
	//{"uintptr_t", 0, k_EDataTypeInt, 0, 1, data_type_primitive_serialize, data_type_primitive_deserialize, data_type_primitive_definition},
	//{"intptr_t", 0, k_EDataTypeInt, 0, 1, data_type_primitive_serialize, data_type_primitive_deserialize, data_type_primitive_definition},
	//{"intmax_t", 0, k_EDataTypeInt, 0, 1, data_type_primitive_serialize, data_type_primitive_deserialize, data_type_primitive_definition},
	{"char", 0, k_EDataTypeChar, 0, 1, data_type_primitive_serialize, data_type_primitive_deserialize, data_type_primitive_definition},
	//{"short", 0, k_EDataTypeInt, 0, 1, data_type_primitive_serialize, data_type_primitive_deserialize, data_type_primitive_definition},
	//{"long", 0, k_EDataTypeInt, 0, 1, data_type_primitive_serialize, data_type_primitive_deserialize, data_type_primitive_definition},
	//{"uint", 0, k_EDataTypeInt, 0, 1, data_type_primitive_serialize, data_type_primitive_deserialize, data_type_primitive_definition},
	//{"ushort", 0, k_EDataTypeUInt, 0, 1, data_type_primitive_serialize, data_type_primitive_deserialize, data_type_primitive_definition},
	//{"ulong", 0, k_EDataTypeUInt, 0, 1, data_type_primitive_serialize, data_type_primitive_deserialize, data_type_primitive_definition},
	{"int8", 0, k_EDataTypeInt, 8, 1, data_type_primitive_serialize, data_type_primitive_deserialize, data_type_primitive_definition},
	{"int16", 0, k_EDataTypeInt, 16, 1, data_type_primitive_serialize, data_type_primitive_deserialize, data_type_primitive_definition},
	{"int32", 0, k_EDataTypeInt, 32, 1, data_type_primitive_serialize, data_type_primitive_deserialize, data_type_primitive_definition},
	{"int64", 0, k_EDataTypeInt, 64, 1, data_type_primitive_serialize, data_type_primitive_deserialize, data_type_primitive_definition},
	{"uint8", 0, k_EDataTypeUInt, 8, 1, data_type_primitive_serialize, data_type_primitive_deserialize, data_type_primitive_definition},
	{"uint16", 0, k_EDataTypeUInt, 16, 1, data_type_primitive_serialize, data_type_primitive_deserialize, data_type_primitive_definition},
	{"uint32", 0, k_EDataTypeUInt, 32, 1, data_type_primitive_serialize, data_type_primitive_deserialize, data_type_primitive_definition},
	{"uint64", 0, k_EDataTypeUInt, 64, 1, data_type_primitive_serialize, data_type_primitive_deserialize, data_type_primitive_definition},
	/*
		C doesn't say anything about exact-width fixed-representation types for floats.
		So... float and double are a exception and we'll just assume sizeof(float) == 4 and sizeof(double) == 8
	*/
	{"float", 0, k_EDataTypeFloat, 0, 1, data_type_primitive_serialize, data_type_primitive_deserialize, data_type_primitive_definition},
	{"double", 0, k_EDataTypeFloat, 0, 1, data_type_primitive_serialize, data_type_primitive_deserialize, data_type_primitive_definition},
	//{"float32", 0, k_EDataTypeFloat, 32, 1, data_type_primitive_serialize, data_type_primitive_deserialize, data_type_primitive_definition},
	//{"float64", 0, k_EDataTypeFloat, 64, 1, data_type_primitive_serialize, data_type_primitive_deserialize, data_type_primitive_definition},
	/*
		I could name them vec3f and vec3fa (aligned to 4 so it fits better in cache line), but I kinda wanna keep it short and can always change this later I guess.
		Maybe add specific types like vec2f vec2d later.
		
		Just assume vecN is a 32-bit float.
		Most of the time it's already promoted to a double anyways but for storing it to disk / network traffic bandwidth usually unnecessary.
	*/
	//{"vec2", 0, k_EDataTypeFloat, 0, 2, data_type_primitive_serialize, data_type_primitive_deserialize, data_type_primitive_definition},
	//{"vec3", 0, k_EDataTypeFloat, 0, 3, data_type_primitive_serialize, data_type_primitive_deserialize, data_type_primitive_definition},
	//{"vec4", 0, k_EDataTypeFloat, 0, 4, data_type_primitive_serialize, data_type_primitive_deserialize, data_type_primitive_definition},
	//{"quat", 0, k_EDataTypeFloat, 0, 3, data_type_primitive_serialize, data_type_primitive_deserialize, data_type_primitive_definition},
	//{"mat3", 0, k_EDataTypeFloat, 0, 9, data_type_primitive_serialize, data_type_primitive_deserialize, data_type_primitive_definition},
	//{"mat4", 0, k_EDataTypeFloat, 0, 16, data_type_primitive_serialize, data_type_primitive_deserialize, data_type_primitive_definition},
	//{"string", 0, k_EDataTypeString, 0, -1, data_type_string_serialize, data_type_string_deserialize, data_type_string_definition},
	//{"hash", 0, k_EDataTypeUInt, 0, 1, data_type_primitive_serialize, data_type_primitive_deserialize, data_type_primitive_definition},
	{NULL, 0, 0, 0, 0, 0, 0, 0}
};
uint32_t used_data_type_flags = 0;
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

bool is_identifier_reserved(const char *ident)
{
	for(s32 i = 0; data_types[i].name; ++i)
	{
		if(!strcmp(data_types[i].name, ident))
			return true;
	}
	return false;
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

TypeEntry *type_entry_by_name(TypeEntry **entries, const char *s)
{
	TypeEntry *it = (*entries);
	while(it)
	{
		if(!strcmp(it->name, s))
			return it;
		it = it->next;
	}
	return NULL;
}

void forward_type(Arena *arena, ForwardedType **types, const char *name)
{
	ForwardedType *ft = new(arena, ForwardedType, 1);
	snprintf(ft->name, sizeof(ft->name), "%s", name);
	if(*types)
	{
		ft->next = (*types);
	}
	*types = ft;
}

bool is_forwarded_type(ForwardedType **types, const char *name)
{
	for(ForwardedType *it = *types; it; it = it->next)
	{
		if(!strcmp(it->name, name))
			return true;
	}
	return false;
}

void parse_type(Lexer *lexer, TypeEntry **entries_out, ForwardedType **forwarded_types)
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
	u64 id_protected = fnv1a_64("protected");
	u64 id_component = fnv1a_64("component");
	u64 id_message = fnv1a_64("message");
	u64 id_extern = fnv1a_64("extern");
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
			case '#':
				lexer_expect(lexer, TOKEN_TYPE_IDENTIFIER, &t);
				char directive[64];
				lexer_token_read_string(lexer, &t, directive, sizeof(directive));
				if(!strcmp(directive, "prefix"))
				{
					lexer_expect(lexer, TOKEN_TYPE_STRING, &t);
					lexer_token_read_string(lexer, &t, type_prefix, sizeof(type_prefix));
				}
				else
				{
					lexer_error(lexer, "Invalid directive");
				}
			break;
			case TOKEN_TYPE_IDENTIFIER:
			{
				if(t.hash == id_extern)
				{
					lexer_expect(lexer, TOKEN_TYPE_IDENTIFIER, &t);
					// TODO: Fix types that have more than 1 identifier like 'unsigned int'.
					// I would have to fix some other code to convert whitespace to underscores and I don't wanna deal with it right now though.
					char fwd_name[64];
					lexer_token_read_string(lexer, &t, fwd_name, sizeof(fwd_name));
					forward_type(lexer->arena, forwarded_types, fwd_name);
					lexer_expect(lexer, ';', NULL);
				} else 
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
					entry->name_hash = 0;
					entry->referenced_as_field = false;
					entry->index = type_index++;
					entry->next = NULL;
					entry->visibility = k_EVisibilityPrivate;
					entry->name[0] = 0;
					entry->fields = NULL;
					
					if(t.hash == id_public || t.hash == id_private || t.hash == id_protected)
					{
						if(t.hash == id_private)
						{
							entry->visibility = k_EVisibilityPrivate;
						} else if(t.hash == id_public)
						{
							entry->visibility = k_EVisibilityPublic;
						} else if(t.hash == id_protected)
						{
							entry->visibility = k_EVisibilityProtected;
						}
						lexer_expect(lexer, TOKEN_TYPE_IDENTIFIER, &t);
					}
					k_EVisibility field_visibility = k_EVisibilityPrivate;
					s32 field_index = 0;
					u64 id_hash = t.hash;
					Token name;
					lexer_expect(lexer, TOKEN_TYPE_IDENTIFIER, &name);
					lexer_expect(lexer, '{', NULL);
					char name_str[256];
					lexer_token_read_string(lexer, &name, name_str, sizeof(name_str));
					snprintf(entry->name, sizeof(entry->name), "%s%s", type_prefix, name_str);
					entry->name_hash = fnv1a_32(entry->name);
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
							lexer_expect(lexer, TOKEN_TYPE_IDENTIFIER, &field_type);
							if(field_type.hash == id_replicated)
							{
								replicated = 1;
								lexer_expect(lexer, ':', NULL);
							} else if(field_type.hash == id_private)
							{
								field_visibility = k_EVisibilityPrivate;
								lexer_expect(lexer, ':', NULL);
							} else if(field_type.hash == id_public)
							{
								field_visibility = k_EVisibilityPublic;
								lexer_expect(lexer, ':', NULL);
							} else if(field_type.hash == id_protected)
							{
								field_visibility = k_EVisibilityProtected;
								lexer_expect(lexer, ':', NULL);
							} else {
								Field *field = new(lexer->arena, Field, 1);
								field->initial_value = NULL;
								lexer_token_read_string(lexer, &field_type, field->type, sizeof(field->type));
								field->next = NULL;
								field->data_type = k_EFieldDataTypePrimitive;
								field->index = field_index;
								field->visibility = field_visibility;
								field->element_count = 1;
								field->replicated = replicated;
								field->type_hash = field_type.hash;
								field->metadata[0] = 0;
								s32 dt_index = data_type_index_by_hash(field_type.hash);
								if(dt_index == -1)
								{
									if(is_forwarded_type(forwarded_types, field->type))
									{										
										field->data_type = k_EFieldDataTypeForward;
									} else
									{
										TypeEntry *fnd = type_entry_by_name(entries_out, field->type);
										if(!fnd)
										{
											for(s32 k = 0; data_types[k].name; ++k)
											{
												printf("%s, ", data_types[k].name);
											}
											printf("\n");
											printf("Field type: %s\n", field->type);
											lexer_error(lexer, "Field type must be one of the following above or a user-defined type.");
										}
										else
										{
											fnd->referenced_as_field = true;
											// Not sure if this is the right place to automatically coerce enum types to a primitive type.
											// TODO: Get the max value of all entries of enum and choose best type to use e.g u8, u16, u32 or u64
											if(fnd->entry_type == k_ETypeEntryTypeEnum || fnd->entry_type == k_ETypeEntryTypeFlags)
											{
												snprintf(field->type, sizeof(field->type), "uint32");
												field->type_hash = fnv1a_64("uint32");
											}
											else
											{
												field->data_type = k_EFieldDataTypeCustom;
											}
										}
									}
								} else
								{
									used_data_type_flags |= (1 << dt_index);
								}
								lexer_expect(lexer, TOKEN_TYPE_IDENTIFIER, &field_name);
								field->name_hash = field_name.hash;
								//printf("field_type: %s\n", field->type);
								lexer_token_read_string(lexer, &field_name, field->name, sizeof(field->name));
								if(is_identifier_reserved(field->name))
									lexer_error(lexer, "Field name cannot be a reserved identifier.");
								//printf("field_name: %s\n", field->name);
								
								if(!lexer_accept(lexer, '[', NULL))
								{
									if(!lexer_accept(lexer, '?', NULL))
									{
										field->element_count = -1;
									} else if(!lexer_accept(lexer, '.', NULL) && !lexer_accept(lexer, '.', NULL) && !lexer_accept(lexer, '.', NULL))
									{
										field->element_count = -2;
									}
									else
									{
										Token num_of_elements_tk;
										lexer_expect(lexer, TOKEN_TYPE_NUMBER, &num_of_elements_tk);
										lexer_token_read_string(lexer, &num_of_elements_tk, index_str, sizeof(index_str));
										field->element_count = atoi(index_str);
									}
									lexer_expect(lexer, ']', NULL);
								}
								
								if(!lexer_accept(lexer, '@', NULL))
								{
									Token keyword;
									lexer_expect(lexer, TOKEN_TYPE_STRING, &keyword);
									lexer_token_read_string(lexer, &keyword, field->metadata, sizeof(field->metadata));
								}
								if(!lexer_accept(lexer, '%', NULL))
								{
									Token index_token;
									lexer_expect(lexer, TOKEN_TYPE_NUMBER, &index_token);
									lexer_token_read_string(lexer, &index_token, index_str, sizeof(index_str));
									field_index = field->index = atoi(index_str);
								}
								if(!lexer_accept(lexer, '=', NULL))
								{
									Stream *ls = lexer->stream;
									s64 pos = ls->tell(ls);
									// Read till EOL
									while(1)
									{
										if(!lexer_accept(lexer, ';', NULL))
										{
											lexer_unget(lexer);
											break;
										}
										Token _;
										lexer_step(lexer, &_);
									}
									s64 nchars = ls->tell(ls) - pos;
									ls->seek(ls, pos, SEEK_SET);
									char* initial_value = new(lexer->arena, char, nchars + 1);

									ls->read(ls, initial_value, 1, nchars);
									initial_value[nchars] = 0;

									field->initial_value = initial_value;
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
							field->initial_value = NULL;
							field->data_type = k_EFieldDataTypePrimitive;
							field->next = NULL;
							field->index = -1;
							field->visibility = field_visibility;
							field->element_count = 1;
							field->replicated = replicated;
							field->type[0] = 0;
							Token enum_value;
							lexer_expect(lexer, TOKEN_TYPE_IDENTIFIER, &enum_value);
							lexer_token_read_string(lexer, &enum_value, field->name, sizeof(field->name));
							//printf("enum value: %s\n", temp);
							
							if(!fields)
							{
								entry->fields = fields = field;
							} else {
								fields->next = field;
								fields = field;
							}
							
							// TODO: Initial value for enum values.
							
							if(!lexer_accept(lexer, '}', NULL))
								break;
							lexer_expect(lexer, ',', NULL);
						}
					} else {
						lexer_error(lexer, "Expected struct, enum or bitflags");
					}
					entry->type_index = entry_type_index[entry->entry_type]++;
					lexer_accept(lexer, ';', NULL);
				}
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
				if(entries->visibility != k_EVisibilityPrivate)
				{
					printf("\tconst void *type_info_;\n");
					// printf("\tuint32_t type_; // fnv1a_32 of struct name\n");
				}
			break;
		}
		Field *fields = entries->fields;
		s32 field_index = 0;
		while(fields)
		{
			switch(entries->entry_type)
			{
				case k_ETypeEntryTypeEnum:
				{
					char concatenated[256] = { 0 };
					char constant[256] = { 0 };
					snprintf(concatenated, sizeof(concatenated), "%s%s", entries->name, fields->name);
					string_constant_case(concatenated, constant, sizeof(constant));
					printf("\t%s", constant);
					if(fields->next)
						printf(",\n");
					else
						printf("\n");
				} break;
				case k_ETypeEntryTypeFlags:
				{
					char concatenated[256] = { 0 };
					char constant[256] = { 0 };
					snprintf(concatenated, sizeof(concatenated), "%s%s", entries->name, fields->name);
					string_constant_case(concatenated, constant, sizeof(constant));
					printf("\t%s = 0x%02X", constant, (1 << field_index));
					if(fields->next)
						printf(",\n");
					else
						printf("\n");
				} break;
				default:
				{
					if(entry_type_is_structure(entries->entry_type))
					{
						write_field_definition(fields);
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
				printf("} %s;\n\n", entries->name);
			break;
			default:
				if(entry_type_is_structure(entries->entry_type))
				{
					printf("} %s;\n\n", entries->name);
				}
			break;
		}
		entries = entries->next;
	}
}

void write_visitor_entry_field(TypeEntry **entries, ForwardedType **forwarded_types, Field *field)
{
	if(field->visibility == k_EVisibilityPrivate)
		return;
	char field_name[256] = {0};
	if(field->visibility == k_EVisibilityProtected)
	{
		snprintf(field_name, sizeof(field_name), "NULL");
	} else {
		
		snprintf(field_name, sizeof(field_name), "\"%s\"", field->metadata[0] != 0 ? field->metadata : field->name);
	}
	if(field->element_count > 1)
	{
		printf("\tif(visitor->pre_visit(visitor, %s, 0x%x, (void**)&inst->%s, NULL, sizeof(inst->%s[0])))\n\t{\n", field_name, field->name_hash, field->name, field->name);
	} else if(field->element_count == -2)
	{
		printf("\tif(visitor->pre_visit(visitor, %s, 0x%x, (void**)&inst->%s, &n, sizeof(inst->%s[0])) && n > 0 && inst->%s)\n\t{\n", field_name, field->name_hash, field->name, field->name, field->name);
	} else if(field->element_count == -1)
	{
		printf("\tif(visitor->pre_visit(visitor, %s, 0x%x, (void**)&inst->%s, &inst->num%s, sizeof(inst->%s[0])) && inst->%s && inst->num%s > 0)\n\t{\n", field_name, field->name_hash, field->name, field->name, field->name, field->name, field->name);
	}
	else
	{
		printf("\tif(visitor->pre_visit(visitor, %s, 0x%x, NULL, NULL, sizeof(inst->%s)))\n\t{\n", field_name, field->name_hash, field->name);
	}
	if(field->element_count > 1)
	{
		printf("\t\tchanged_count += visitor->visit_%s(visitor, %s, (%s*)&inst->%s[0], %d, sizeof(inst->%s[0]));\n", field->type, field_name, field->type, field->name, field->element_count, field->name);
	}
	else if(field->element_count == -1)
	{
		// printf("\t\tvisitor->visit_field_count(ctx, %s, (void**)&inst->%s, sizeof(inst->%s[0]), &inst->num%s);\n", field_name, field->name, field->name, field->name);
		printf("\t\tchanged_count += visitor->visit_%s(visitor, %s, (%s*)&inst->%s[0], inst->num%s, sizeof(inst->%s[0]));\n", field->type, field_name, field->type, field->name, field->name, field->name);
	} else if(field->element_count == -2)
	{
		// printf("\t\tn = 0;\n");
		// printf("\t\tvisitor->visit_field_count(ctx, %s, (void**)&inst->%s, sizeof(inst->%s[0]), &n);\n", field_name, field->name, field->name);
		printf("\t\tchanged_count += visitor->visit_%s(visitor, %s, (%s*)&inst->%s[0], n, sizeof(inst->%s[0])));\n", field->type, field_name, field->type, field->name, field->name);
	}
	else
	{
		printf("\t\tchanged_count += visitor->visit_%s(visitor, %s, (%s*)&inst->%s, 1, sizeof(inst->%s));\n", field->type, field_name, field->type, field->name, field->name);
	}
	printf("\t}\n");
}
void write_visitor_entry(const char *type_field_visitor_name, Arena *arena, TypeEntry **entries, ForwardedType **forwarded_types, TypeEntry *it)
{
	if(entry_type_is_structure(it->entry_type))
	{
		Field *fields;
		fields = it->fields;
		printf("static void %s_init(%s *inst)\n{\n", it->name, it->name);
		
		// printf("\t%s *inst = (%s*)data;\n", it->name, it->name);
		if(it->visibility != k_EVisibilityPrivate)
		{
			printf("\tstatic const StructTypeInfo type_info_ = {.name = \"%s\", .hash = 0x%x, .size = sizeof(%s), .alignment = alignof(%s), .initialize = (void(*)(void*))%s_init, .clone = NULL, .visitor = (size_t (*)(void *visitor, const char *key, void *value, size_t nmemb, size_t size))%s_visit};\n",
				it->visibility == k_EVisibilityPublic ? it->name : "",
				it->name_hash,
				it->name,
				it->name,
				it->name,
				it->name
			);
			printf("\tmemset(inst, 0, sizeof(*inst));\n");
			// printf("\tinst->type_ = %" PRIu32 ";\n", it->name_hash);
			printf("\tinst->type_info_ = &type_info_;\n");
		}
		while(fields)
		{
			if(fields->data_type == k_EFieldDataTypeCustom)
			{
				TypeEntry *fnd = type_entry_by_name(entries, fields->type);
				if(fnd)// && fnd->visibility != k_EVisibilityPrivate) // If the type visibility is set to private, we can't infer the type because it doesn't have a type_ field.
				{
					if(fields->element_count == 1)
						printf("\t%s_init(&inst->%s);\n", fnd->name, fields->name);
					else if(fields->element_count < 0)
					{
						if(fields->element_count == -1)
							printf("\tinst->num%s = 0;\n", fields->name);
						printf("\tinst->%s = NULL;\n", fields->name);
					}
				}
			} else
			{
				// TODO: Add check whether initial value can be copied to the type of this field.
				
				if(fields->initial_value)
				{
					Stream s = {0};
					StreamBuffer sb = {0};
					init_stream_from_buffer(&s, &sb, fields->initial_value, strlen(fields->initial_value) + 1);

					Lexer l = { 0 };
					lexer_init(&l, arena, &s);
					l.flags |= k_ELexerFlagSkipComments;
					
					Token _;
					if(!lexer_accept(&l, '{', &_) || !lexer_accept(&l, '[', &_))
					{
						// List of values
						int c = 0;
						do
						{
							lexer_step(&l, &_);
							char str[256];
							str[0] = 0;
							lexer_token_read_string(&l, &_, str, sizeof(str));
							printf("\tinst->%s[%d] = %s;\n", fields->name, c++, str);
						} while (!lexer_accept(&l, ',', NULL));
						
					} else if(!lexer_accept(&l, TOKEN_TYPE_STRING, &_))
					{
						// String
						printf("\tsnprintf(inst->%s, sizeof(inst->%s),%s);\n", fields->name, fields->name, fields->initial_value);
					} else
					{
						// TODO: Lexer doesn't support hexadecimal values at the moment.
						printf("\tinst->%s =%s;\n", fields->name, fields->initial_value);
					}
				} else
				{
					// if(fields->element_count < 0)
					// {
					// 	if(fields->element_count == -1)
					// 		printf("\tinst->num%s = 0;\n", fields->name);
					// 	printf("\tinst->%s = NULL;\n", fields->name);
					// }
				}
			}
			fields = fields->next;
		}
		printf("}\n\n");
	}
}
void write_visitor_entry_visitor(const char *type_field_visitor_name, Arena *arena, TypeEntry **entries, ForwardedType **forwarded_types, TypeEntry *it)
{
	if(entry_type_is_structure(it->entry_type))
	{
		if(entry_visit_count(it) > 0)
		{
			printf("static size_t %s_visit(%s *visitor, const char *key, %s *inst, size_t nmemb, size_t size)\n{\n", it->name, type_field_visitor_name, it->name);
			printf("\tsize_t changed_count = 0;\n");
			printf("\tsize_t n = 0;\n");
			ITERATE_LIST(Field, it->fields, field)
			{
				if(field->visibility != k_EVisibilityPrivate)
				{
					write_visitor_entry_field(entries, forwarded_types, field);
				}
			}
			printf("\treturn changed_count;\n}\n\n");
		}
	}
}

size_t type_entry_field_count(TypeEntry *it)
{
	size_t n = 0;
	
	Field *fields = it->fields;
	while(fields)
	{
		if(fields->visibility != k_EVisibilityPrivate)
		{
			++n;
		}
		fields = fields->next;
	}
	return n;
}

void write_vtable(CompilerOptions *opts, TypeEntry *entries)
{
	// https://stackoverflow.com/questions/24743520/incompatible-pointer-types-passing-in-generic-macro
	// All branches of a _Generic primary expression must be a valid expressions, and thus valid under all circumstances.
	// The fact that only one of the branches will ever be evaluated, is not related to this.
	// printf("#define type_%s_init(x) _Generic((x), \\\n", opts->prefix);
	// ITERATE_LIST(TypeEntry, entries, it)
	// {
	// 	if(entry_type_is_structure(it->entry_type))// && entries->visibility != k_EVisibilityPrivate)
	// 	{
	// 		// printf("\t%s*: %s_init(x), \\\n", entries->name, entries->name);
	// 		printf("\t%s: %s_init", it->name, it->name);
	// 		if(it->next)
	// 		{
	// 			printf(",");
	// 			printf(" \\");
	// 		} else
	// 		{
	// 			printf(")(&(x))");
	// 		}
	// 		printf("\n");
	// 	}
	// }
printf(R"(
#ifndef type_info
static StructTypeInfo* type_info_(StructTypeInfo **ptr)
{
	return *ptr;
}
#define type_info(ptr) type_info_((StructTypeInfo**)ptr)
#endif

#ifndef type_init
#define type_init(type, name) \
    type name = (type){ 0 }; \
    type##_init(&name)
#endif

)");

/*
#define type_init(type, name, ...) \
    type name = (type){__VA_ARGS__, 0}; \
    name##_init(&name);
*/
// ##__VA_ARGS__ is GCC extension
// __VA_OPT__ is C23

	// printf("#define type_%s_visit(visitor, x) _Generic((x), \\\n", opts->prefix);
	// ITERATE_LIST(TypeEntry, entries, it)
	// {
	// 	if(entry_type_is_structure(it->entry_type) && entry_visit_count(it) > 0)// && entries->visibility != k_EVisibilityPrivate)
	// 	{
	// 		// printf("\t%s*: %s_init(x), \\\n", entries->name, entries->name);
	// 		printf("\t%s: %s_visit", it->name, it->name);
	// 		if(it->next)
	// 		{
	// 			printf(",");
	// 			printf(" \\");
	// 		} else
	// 		{
	// 			printf(")(visitor, NULL, &(x), 1, sizeof(x))");
	// 		}
	// 		printf("\n");
	// 	}
	// }
}

int sort_fields_compare(Field *a, Field *b)
{
	return a->index - b->index;
}

void sort_fields(Field **fields, int (*compare)(Field *a, Field *b))
{
	if(!*fields)
		return; // Empty
	
	if(!(*fields)->next)
		return; // Single field
	
	int swapped;
	do
	{
		Field **it = fields;
		swapped = 0;
		while((*it)->next)
		{
			int result = compare((*it), (*it)->next);
			/*
				a = 1
				b = 2
				return a - b; // Returns < 0, do nothing.
			*/
			if(result > 0)
			{
				Field *tmp = (*it); // Save current
				*it = (*it)->next; // Set current to next
				// Swap next to point to eachother
				tmp->next = (*it)->next;
				(*it)->next = tmp;
				swapped = 1;
			}
			it = &((*it)->next);
		}
	} while(swapped);
}
ForwardedType *forwarded_types = NULL;

static void print_types()
{
printf(R"(
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

)");
}

static void print_reflection_info()
{
printf(R"(
typedef enum
{
	k_EDataTypeCustom = 1,
	k_EDataTypeInt,
	k_EDataTypeUInt,
	k_EDataTypeFloat,
	k_EDataTypeString,
	k_EDataTypeChar
	//k_EDataTypeHash,
	//k_EDataTypeBuffer
} k_EDataType;

typedef int (*RTFI_ToStringFn)(void *value, int num_elements, char *buf, size_t bufsz);

typedef struct
{
	const char *name;
	size_t offset;
	size_t size;
	size_t element_count;
	size_t bits;
	void *field;
	k_EDataType data_type;
	//TODO: Pass special properties that are set for this field?
	//const char *tag;
	//int flags;
	RTFI_ToStringFn to_string;
} ReflectionFieldTypeInfo;
)");

	{
		{
			ForwardedType *fwd_it = forwarded_types;
			while(fwd_it)
			{
				printf("extern int data_type_%s_to_string(%s *value, int num_elements, char *buf, size_t bufsz);\n", fwd_it->name, fwd_it->name);
				fwd_it = fwd_it->next;
			}
			printf("\n");
		}
	}
	{
		// https://stackoverflow.com/questions/7899119/what-does-s-mean-in-printf
		// It should be noted that the str_len argument must have type int (or narrower integral type, which would be promoted to int). It would be a bug to pass long, size_t.
		
		for(s32 i = 0; data_types[i].name; ++i)
		{
			printf("static int data_type_%s_to_string(%s *value, int num_elements, char *buf, size_t bufsz)\n{\n", data_types[i].name, data_types[i].name);
			
			//TODO: If I do add bit types like int3 then convert them up to the nearest integer e.g int8 or int16
			size_t bc = data_types[i].bit_count;
			switch(data_types[i].type)
			{
				/*
					(C99, 6.3.1.1p2) " The following may be used in an expression wherever an int or unsigned int may be used:
					— An object or expression with an integer type whose integer conversion rank is less than or equal to the rank of int and unsigned int.
					— A bit-field of type _Bool, int, signed int, or unsigned int.
					If an int can represent all values of the original type, the value is converted to an int; otherwise, it is converted to an unsigned int. These are called the integer promotions.48) All other types are unchanged by the integer promotions."
				*/
				case k_EDataTypeUInt:
				if(data_types[i].bit_count != 0)
				{
					printf("\treturn snprintf(buf, bufsz, \"%%\" PRIu%d, (uint%d_t)(*value));\n", bc, bc);
				} else
				{
					printf("\treturn snprintf(buf, bufsz, \"%%\" PRIuMAX, (uintmax_t)(*value));\n");
				}
				break;
				
				case k_EDataTypeInt:
				if(data_types[i].bit_count != 0)
				{
					printf("\treturn snprintf(buf, bufsz, \"%%\" PRIu%d, (int%d_t)(*value));\n", bc, bc);
				} else
				{
					printf("\treturn snprintf(buf, bufsz, \"%%\" PRIiMAX, (intmax_t)(*value));\n");
				}
				break;
				/*
					Float will be promoted to double.
					That means for printf the format %f and %lf are identical, since the argument will be a double.
				*/
				case k_EDataTypeFloat:
				if(data_types[i].element_count == 1)
				{
					printf("\treturn snprintf(buf, bufsz, \"%%lf\", (double)(*value));\n");
				} else if(data_types[i].element_count == 2)
				{
					printf("\treturn snprintf(buf, bufsz, \"%%lf %%lf\", (double)(*value)[0], (double)(*value)[1]);\n");
				} else if(data_types[i].element_count == 3)
				{
					printf("\treturn snprintf(buf, bufsz, \"%%lf %%lf %%lf\", (double)(*value)[0], (double)(*value)[1], (double)(*value)[2]);\n");
				} else if(data_types[i].element_count == 4)
				{
					printf("\treturn snprintf(buf, bufsz, \"%%lf %%lf %%lf %%lf\", (double)(*value)[0], (double)(*value)[1], (double)(*value)[2], (double)(*value)[3]);\n");
				} else if(data_types[i].element_count == 9)
				{
					printf("\treturn snprintf(buf, bufsz, \"%%lf %%lf %%lf %%lf %%lf %%lf %%lf %%lf %%lf\", (double)(*value)[0], (double)(*value)[1], (double)(*value)[2], (double)(*value)[3], (double)(*value)[4], (double)(*value)[5], (double)(*value)[6], (double)(*value)[7], (double)(*value)[8]);\n");
				} else if(data_types[i].element_count == 16)
				{
					printf("\treturn snprintf(buf, bufsz, \"%%lf %%lf %%lf %%lf %%lf %%lf %%lf %%lf %%lf %%lf %%lf %%lf %%lf %%lf %%lf %%lf\", (double)(*value)[0], (double)(*value)[1], (double)(*value)[2], (double)(*value)[3], (double)(*value)[4], (double)(*value)[5], (double)(*value)[6], (double)(*value)[7], (double)(*value)[8], (double)(*value)[9], (double)(*value)[10], (double)(*value)[11], (double)(*value)[12], (double)(*value)[13], (double)(*value)[14], (double)(*value)[15]);\n");
				} else
				{
					printf("\t#error \"Unimplemented variable length\"\n");
				}
				break;
				
				case k_EDataTypeChar:
					printf("\treturn snprintf(buf, bufsz, \"%%.*s\", num_elements, value);\n");
				break;
				
				case k_EDataTypeString:
					printf("\treturn snprintf(buf, bufsz, \"%%s\", *value);\n");
				break;
			}
			printf("}\n\n");
		}
	}
}

static void print_visitor_info(CompilerOptions *opts, Arena *arena, TypeEntry *entries)
{
	char type_field_visitor_name[256]={0};
	snprintf(type_field_visitor_name,
			 sizeof(type_field_visitor_name),
			 "%c%sVisitor",
			 toupper(opts->prefix[0]),
			 &opts->prefix[1]);
	printf("typedef struct %s_s %s;\n", type_field_visitor_name, type_field_visitor_name);
	printf("typedef size_t (*%sFn)(%s *visitor, const char *key, void *value, size_t nmemb, size_t size);\n\n", type_field_visitor_name, type_field_visitor_name);

	printf("struct %s_s\n{\n", type_field_visitor_name);
	printf("\tvoid *ctx;\n\n");
	
	for(s32 i = 0; data_types[i].name; ++i)
	{
		if(used_data_type_flags & (1 << i))
		{
			printf("\t%sFn visit_%s;\n", type_field_visitor_name, data_types[i].name);
		}
	}
	ITERATE_LIST(ForwardedType, forwarded_types, fwd_it)
	{
		printf("\t%sFn visit_%s;\n", type_field_visitor_name, fwd_it->name);
	}
	ITERATE_LIST(TypeEntry, entries, it)
	{
		if(entry_type_is_structure(it->entry_type))// && entries->visibility != k_EVisibilityPrivate)
		{
			if(it->referenced_as_field)
			{
				printf("\t%sFn visit_%s;\n", type_field_visitor_name, it->name);
			}
		}
	}
	// printf("\t%sFn visit; // Fallback in case visit_X is NULL\n", type_field_visitor_name);
	printf("\tbool (*pre_visit)(%s *visitor, const char *key, uint32_t hashed_key, void **value, size_t *nmemb, size_t size);\n", type_field_visitor_name);
	// printf("\tbool (*visit_field_count)(struct %s_s *visitor, const char *key, void **value, size_t element_size, size_t *num_elements, size_t *changed_count);\n", type_field_visitor_name);
	printf("};\n\n");
	printf("// ============              VISITORS            ==================\n\n");
	ITERATE_LIST(TypeEntry, entries, it)
	{
		write_visitor_entry_visitor(type_field_visitor_name, arena, &entries, &forwarded_types, it);
	}
	printf("// ================================================================\n\n");
	printf("static size_t %s_visitor_dummy_(%s *visitor, const char *key, void *value, size_t nmemb, size_t size) { return 0; }\n", opts->prefix, type_field_visitor_name);
	printf("static bool %s_pre_visit_dummy_(%s *visitor, const char *key, uint32_t hashed_key, void **value, size_t *nmemb, size_t size) { return true; }\n", opts->prefix, type_field_visitor_name);
	printf("static void type_%s_visitor_init(%s *v, void *ctx)\n{\n", opts->prefix, type_field_visitor_name);
	printf("\tv->ctx = ctx;\n");
	printf("\tv->pre_visit = %s_pre_visit_dummy_;\n", opts->prefix);
	for(s32 i = 0; data_types[i].name; ++i)
	{
		if(used_data_type_flags & (1 << i))
		{
			printf("\tv->visit_%s = %s_visitor_dummy_;\n", data_types[i].name, opts->prefix);
		}
	}
	
	ITERATE_LIST(ForwardedType, forwarded_types, fwd_it)
	{
		printf("\tv->visit_%s = %s_visitor_dummy_;\n", fwd_it->name, opts->prefix);
	}
	ITERATE_LIST(TypeEntry, entries, it)
	{
		if(entry_type_is_structure(it->entry_type))// && entries->visibility != k_EVisibilityPrivate)
		{
			if(it->referenced_as_field)
			{
				if(entry_visit_count(it) > 0)
				{
					printf("\tv->visit_%s = (%sFn)%s_visit;\n", it->name, type_field_visitor_name, it->name);
				} else
				{
					printf("\tv->visit_%s = %s_visitor_dummy_;\n", it->name, opts->prefix);
				}
			}
		}
	}
	printf("}\n");
	printf("// ============           INITIALIZATION         ==================\n\n");
	ITERATE_LIST(TypeEntry, entries, it)
	{
		write_visitor_entry(type_field_visitor_name, arena, &entries, &forwarded_types, it);
	}
	printf("// ================================================================\n\n");
}

void parse_type_file(const char *path, Arena *arena, CompilerOptions *opts)
{
	
	initialize_data_type_hashes();

	Lexer lexer = { 0 };

	Stream ls = { 0 };
	stream_open_file(&ls, path, "rb");
	lexer_init(&lexer, arena, &ls);
	
	lexer.flags |= k_ELexerFlagSkipComments;
	if(setjmp(lexer.jmp_error))
	{
		fprintf(stderr, "Lexer error");
		exit(-1);
	}

	TypeEntry *entries;
	parse_type(&lexer, &entries, &forwarded_types);
	
	{
		TypeEntry *it = entries;
		while(it)
		{
			sort_fields(&it->fields, sort_fields_compare);
			it = it->next;
		}
	}

	if(strcmp(opts->mode, "c11"))
	{
		fprintf(stderr, "C11 is required for _Generic support");
		exit(-1);
	}
	char uppercase[256] = { 0 };
	for(size_t i = 0; i < sizeof(uppercase) - 1; ++i)
	{
		if(!opts->prefix[i])
			break;
		uppercase[i] = toupper(opts->prefix[i]);
	}
	
	// printf("#ifdef TYPE_IMPLEMENTATION\n\n");
	// printf("#include <stdbool.h>\n");
	// printf("#include <inttypes.h>\n");
	// printf("#include <stddef.h>\n");
	// printf("#include <stdio.h>\n");

	// printf("#ifdef DEFINE_%s_X_MACRO\n", uppercase);

	printf("#ifndef %s_X_MACRO\n", uppercase);
	char tmp[256] = { 0 };
	printf("#define %s_X_MACRO(X) \\\n", uppercase);
	ITERATE_LIST(TypeEntry, entries, it)
	{
		if(entry_type_is_structure(it->entry_type))
		{
			string_constant_case(it->name, tmp, sizeof(tmp));
			char tmp_lower[256] = { 0 };
			for(size_t k = 0; tmp[k]; ++k)
				tmp_lower[k] = tolower(tmp[k]);
			if(it->next)
				printf("\tX(%s, %s, %s) \\\n", tmp, it->name, tmp_lower);
			else
				printf("\tX(%s, %s, %s)\n", tmp, it->name, tmp_lower);
		}
	}
	printf("#endif\n");
	// printf("#endif\n");

	printf("#ifndef %s_TYPE_HEADER_INCLUDE_GUARD\n", uppercase);
	printf("#define %s_TYPE_HEADER_INCLUDE_GUARD\n", uppercase);
	print_types();
	write_definitions(entries);
	printf("#ifdef %s_VISITOR_IMPLEMENTATION\n", uppercase);
printf(R"(
#ifndef STRUCT_TYPE_INFO_DEFINED
#define STRUCT_TYPE_INFO_DEFINED

typedef struct
{
	const char *name;
	uint32_t hash;
	size_t size;
	size_t alignment;
	void (*initialize)(void*);
	void* (*clone)(void*);
	size_t (*visitor)(void *visitor, const char *key, void *value, size_t nmemb, size_t size);
} StructTypeInfo;

#endif

// #include <stdalign.h>

#if __STDC_VERSION__ >= 202311L // C23
    // Already supported
#elif __STDC_VERSION__ >= 201112L // C11
    #define alignof _Alignof
#else
    #define alignof(x) __alignof__(x) // Try GCC extension
#endif

)");

	print_visitor_info(opts, arena, entries);
	// {
	// 	TypeEntry *it = entries;

	// 	while(it)
	// 	{
	// 		printf("static const StructTypeInfo type_info_%s_ = {.hash = 0x%x, .size = sizeof(%s), .initialize = %s_init};\n",
	// 			it->name,
	// 			it->name_hash,
	// 			it->name,
	// 			it->name
	// 		);
	// 		it = it->next;
	// 	}
	// 	printf("\n");
	// }
	write_vtable(opts, entries);
	printf("#endif\n");
	printf("#endif\n");
	// {
	// 	TypeEntry *it = entries;

	// 	printf("typedef enum\n{\n");
	// 	while(it)
	// 	{
	// 		printf("\tk_EType%s = %d,\n", it->name, it->index);
	// 		it = it->next;
	// 	}
	// 	printf("} k_EType;\n\n"); //TODO: Append header filename to prevent collision between multiple header files.
	// }
	// printf("#endif\n");
	stream_close_file(&ls);
}
#endif