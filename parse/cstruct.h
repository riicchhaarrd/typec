#ifndef CSTRUCT_H
#define CSTRUCT_H

#define MAX_STRUCT_FIELD_TOKENS (32)

typedef struct
{
	s32 token_count;
	Token tokens[MAX_STRUCT_FIELD_TOKENS];
	s32 name;
	s32 count; // For arrays, e.g data[4], count would be 4
} CStructField;

void parse_c_struct_field_name(CStructField *cf)
{
	s32 n = cf->token_count;
	// Find if there's a ], in that case it's a array and it may use a identifier e.g macro so skip that one
	s32 offset = 0;
	for(s32 i = 0; i < n; ++i)
	{
		s32 token_index = n - i - 1;
		Token *t = &cf->tokens[token_index];
		if(t->token_type == '[')
		{
			offset = i;
			break;
		}
	}
	// Set the index of the token that holds the string for the name of this field.
	
	for(s32 i = offset; i < n; ++i)
	{
		s32 token_index = n - i - 1;
		Token *t = &cf->tokens[token_index];
		if(t->token_type == k_ETokenTypeIdentifier)
		{
			cf->name = token_index;
			break;
		}
	}
}

k_ETokenType parse_c_struct_field(Lexer *lexer, CStructField *cf)
{
	Token *t = NULL;
	cf->token_count = 0;
	while(1)
	{
		if(cf->token_count >= MAX_STRUCT_FIELD_TOKENS)
		{
			error("cf->token_count >= MAX_STRUCT_FIELD_TOKENS");
		}
		t = &cf->tokens[cf->token_count++];
		int result = lexer_step(lexer, t);
		if(result)
		{
			error("Unexpected EOF");
		}
		if(t->token_type == '}' || t->token_type == ';' || t->token_type == ',')
			break;
	}
	
	if(t->token_type == ';' || t->token_type == ',')
	{
		parse_c_struct_field_name(cf);
	}
	
	return t->token_type;
}

void parse_struct(Lexer *lexer)
{
	char temp[64];
	Token t;
	lexer_step(lexer, &t);
	if(t.token_type == k_ETokenTypeIdentifier)
	{
		lexer_token_read_string(lexer, &t, temp, sizeof(temp));
		printf("struct type: %s\n", temp);
	}
	else if(t.token_type != '{')
	{
		error("Expected {\n");
	}
	CStructField cf = {0};
	char ident[64];
	while(1)
	{
		k_ETokenType tt = parse_c_struct_field(lexer, &cf);
		if(tt == '}')
		{
			break;
		}
		if(cf.name != -1)
		{
			lexer_token_read_string(lexer, &cf.tokens[cf.name], ident, sizeof(ident));
			printf("\tfield: %s\n", ident);
		}
	}
	lexer_step(lexer, &t);
	lexer_token_read_string(lexer, &t, temp, sizeof(temp));
	if(t.token_type != ';')
	{
		printf("struct type: %s\n", temp);
	}
}

void parse_c_structs(const char *path, Arena *arena)
{
	FILE *fp = fopen(path, "rb");
	if(!fp)
		return;

	Lexer lexer = { 0 };

	LexerStream ls = { 0 };
	lexer_stream_init_file(&ls, fp);
	lexer_init(&lexer, arena, &ls);

	char temp[64];
	Token t;
	u64 typedef_id = fnv1a_64("typedef");
	u64 struct_id = fnv1a_64("struct");
	u64 enum_id = fnv1a_64("enum");
	
	while(!lexer_step(&lexer, &t))
	{
		switch(t.token_type)
		{
			case k_ETokenTypeIdentifier:
			{
				if(t.hash == typedef_id)
				{
					lexer_step(&lexer, &t);
					if(t.hash == struct_id)
					{
						parse_struct(&lexer);
					}
				}
				else if(t.hash == struct_id)
				{
					parse_struct(&lexer);
				}
			}
			break;
		}
	}
	fclose(fp);
}
#endif