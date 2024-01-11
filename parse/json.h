#ifndef JSON_H
#define JSON_H

typedef enum
{
	k_EJsonElementTypeNull,
	k_EJsonElementTypeTrue,
	k_EJsonElementTypeFalse,
	k_EJsonElementTypeString,
	k_EJsonElementTypeNumber,
	k_EJsonElementTypeObject,
	k_EJsonElementTypeArray
} k_EJsonElementType;

typedef struct JsonElement_s
{
	s32 text_start;
	s32 text_end;
	k_EJsonElementType element_type;
} JsonElement;

typedef struct JsonParser_s
{
	void *ctx;
	void (*on_object)(struct JsonParser_s *parser, void *ctx, const char *key);
	void (*on_array)(struct JsonParser_s *parser, void *ctx, const char *key);
} JsonParser;

void parse_json_element(Lexer *lexer, JsonParser *parser)
{
	Token t;
	if(lexer_step(lexer, &t))
		return;
	char temp[64];
	u64 id_false = fnv1a_64("false");
	u64 id_true = fnv1a_64("true");
	u64 id_null = fnv1a_64("null");
	switch(t.token_type)
	{
		case '{':
		{
			if(lexer_accept(lexer, '}', NULL))
			{
				do
				{
					lexer_expect(lexer, k_ETokenTypeString, &t);
					//lexer_token_read_string(lexer, &t, temp, sizeof(temp));
					//printf("key:%s\n",temp);
					lexer_expect(lexer, ':', NULL);
					parse_json_element(lexer, parser);

				} while(!lexer_accept(lexer, ',', NULL));
				
				lexer_expect(lexer, '}', NULL);
			}
		} break;
		case '[':
		{
			if(lexer_accept(lexer, ']', NULL))
			{
				do
				{
					parse_json_element(lexer, parser);
				} while(!lexer_accept(lexer, ',', NULL));
				
				lexer_expect(lexer, ']', NULL);
			}
		} break;
		
		case k_ETokenTypeNumber:
		break;
		case k_ETokenTypeString:
		break;
		case k_ETokenTypeIdentifier:
		{
			if(t.hash == id_false)
			{
				
			} else if(t.hash == id_true)
			{
				
			} else if(t.hash == id_null)
			{
				
			} else {
				lexer_error(lexer, "Expected true, false or null");
			}
		} break;
		
		default:
			token_type_to_string(t.token_type, temp, sizeof(temp));
			printf("%s\n", temp);
			lexer_error(lexer, "Expected JSON value");
		break;
	}
}

void parse_json(const char *path, Arena *arena)
{
	FILE *fp = fopen(path, "rb");
	if(!fp)
		return;

	Lexer lexer = { 0 };

	LexerStream ls = { 0 };
	lexer_stream_init_file(&ls, fp);
	lexer_init(&lexer, arena, &ls);

	parse_json_element(&lexer, 0);
	fclose(fp);
}
#endif