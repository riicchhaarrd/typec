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
	struct Field_s *next;
} Field;

typedef enum
{
	k_ETypeEntryTypeStruct,
	k_ETypeEntryTypeEnum,
	k_ETypeEntryTypeFlags
} k_ETypeEntryType;

typedef struct TypeEntry_s
{
	struct TypeEntry_s *next;
	k_ETypeEntryType entry_type;
	char name[64];
	Field *fields;
} TypeEntry;

void parse_type(Lexer *lexer, TypeEntry **entries_out)
{
	Token t;
	
	u64 id_enum = fnv1a_64("enum");
	u64 id_struct = fnv1a_64("struct");
	u64 id_bitflags = fnv1a_64("bitflags");
	//char temp[64];
	TypeEntry *entries = NULL;
	while(!lexer_step(lexer, &t))
	{
		switch(t.token_type)
		{
			default:
				lexer_error(lexer, "Expected identifier");
			break;
			case k_ETokenTypeIdentifier:
			{
				TypeEntry *entry = new(lexer->arena, TypeEntry, 1);
				if(!entries)
				{
					*entries_out = entries = entry;
				} else {
					entries->next = entry;
					entries = entry;
				}
				
				entry->next = NULL;
				entry->name[0] = 0;
				entry->fields = NULL;
				
				u64 id_hash = t.hash;
				Token name;
				lexer_expect(lexer, k_ETokenTypeIdentifier, &name);
				lexer_expect(lexer, '{', &t);
				lexer_token_read_string(lexer, &name, entry->name, sizeof(entry->name));
				//printf("%s\n", temp);
				if(id_hash == id_struct)
				{
					entry->entry_type = k_ETypeEntryTypeStruct;
					Field *fields = NULL;
					while(1)
					{
						if(!lexer_accept(lexer, '}', NULL))
							break;
						Field *field = new(lexer->arena, Field, 1);
						field->next = NULL;

						Token field_type, field_name;
						lexer_expect(lexer, k_ETokenTypeIdentifier, &field_type);
						lexer_expect(lexer, k_ETokenTypeIdentifier, &field_name);
						lexer_token_read_string(lexer, &field_type, field->type, sizeof(field->type));
						//printf("field_type: %s\n", field->type);
						lexer_token_read_string(lexer, &field_name, field->name, sizeof(field->name));
						//printf("field_name: %s\n", field->name);
						
						if(!fields)
						{
							entry->fields = fields = field;
						} else {
							fields->next = field;
							fields = field;
						}
						lexer_expect(lexer, ';', NULL);
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
				lexer_accept(lexer, ';', NULL);
			} break;
		}
	}
}

void parse_type_file(const char *path, Arena *arena)
{
	FILE *fp = fopen(path, "rb");
	if(!fp)
		return;

	Lexer lexer = { 0 };

	LexerStream ls = { 0 };
	lexer_stream_init_file(&ls, fp);
	lexer_init(&lexer, arena, &ls);

	TypeEntry *entries;
	parse_type(&lexer, &entries);
	while(entries)
	{
		switch(entries->entry_type)
		{
			case k_ETypeEntryTypeEnum:
			case k_ETypeEntryTypeFlags:
				printf("enum k_E%s\n", entries->name);
			break;
			case k_ETypeEntryTypeStruct:
				printf("struct %s\n", entries->name);
			break;
		}
		printf("{\n");
		Field *fields = entries->fields;
		s32 field_index = 0;
		while(fields)
		{
			switch(entries->entry_type)
			{
				case k_ETypeEntryTypeEnum:
					printf("\tk_E%s%s", entries->name, fields->name);
				break;
				case k_ETypeEntryTypeFlags:
					printf("\tk_E%s%s = 0x%02X", entries->name, fields->name, (1 << field_index));
				break;
				case k_ETypeEntryTypeStruct:
					printf("\t%s", fields->name);
				break;
			}
			if(fields->next)
				printf(",\n");
			else
				printf("\n");
			++field_index;
			fields = fields->next;
		}
		printf("};\n");
		entries = entries->next;
	}
	fclose(fp);
}
#endif