#ifndef PARSE_H
#define PARSE_H
#include "types.h"

#define MAX_TOKEN_STRING_LENGTH (64)

/*
	//TODO: Lexer Modes
	1. Read entire text into memory + allocate all tokens in memory
	2. Read entire text into memory + temporary buffer for 1 token which then gets parsed
	3. Stream text and temporary buffer for tokens
*/

typedef enum
{
	//ASCII table ...
	k_ETokenTypeIdentifier = 256,
	k_ETokenTypeString,
	k_ETokenTypeNumber,
	k_ETokenTypeMax
} k_ETokenType;

static const char *token_type_to_string(k_ETokenType token_type, char *string_out, int string_out_size)
{
	if(token_type >= k_ETokenTypeMax)
		return "?";
	if(string_out_size < 2)
		return "?";
	if(token_type >= 0x20 && token_type <= /*0x7e*/0xff) // Printable ASCII range
	{
		string_out[0] = token_type & 0xff;
		string_out[1] = 0;
		return string_out;
	}
	if(token_type < 256)
		return "?";
	static const char *type_strings[] = {"identifier", "string", "number"};
	return type_strings[token_type - 256];
}

#pragma pack(push, 1)
typedef struct Token_s
{
	struct Token_s *next;
	u32 position;
	u16 token_type;
	u64 hash;
	#if 0
	union
	{
		float numeric_value;
		char string_value[MAX_TOKEN_STRING_LENGTH];
	};
	#endif
	
	u16 length;
} Token;

typedef struct
{
	void *ctx;
	//int (*eof)(LexerStream*);
	int (*read)(void*, u8*, s32);
	int (*seek)(void*, s32);
	s32 (*tell)(void*);
	//StreamError? longjmp
} LexerStream;

typedef struct
{
	LexerStream *stream;
//	const char *input_stream;
	Token *tokens;
	Token *tokens_end;
	Arena *arena;
//	u32 index;
//	u32 size;
	jmp_buf jmp_error;
} Lexer;
#pragma pack(pop)

void lexer_init(Lexer *l, Arena *arena, LexerStream *stream)
{
	//l->index = 0;
	//l->size = strlen(s);
	//l->input_stream = s;
	l->stream = stream;
	l->arena = arena;
	Token *t = new(arena, Token, 1);
	{		
		t->next = NULL;
		t->position = -1;
		t->length = 0;
		t->token_type = 0;
		t->hash = 0;
	}
	l->tokens = t;
	l->tokens_end = l->tokens;
	if(setjmp(l->jmp_error))
	{
		exit(1);
	}
}

u8 lexer_read_and_advance(Lexer *l)
{
	u8 buf = 0;
	if(l->stream->read(l->stream->ctx, &buf, 1) != 1)
		return 0;
	return buf;
	//if(l->index < 0 || l->index >= l->size)
		//return 0;
	//return l->input_stream[l->index++];
}

void lexer_error(Lexer *l, const char *message)
{
	printf("Lexer error: %s\n", message);
	longjmp(l->jmp_error, 1);
}

void lexer_unget(Lexer *l)
{
	s32 current = l->stream->tell(l->stream->ctx);
	if(current == 0)
		return;
	l->stream->seek(l->stream->ctx, current - 1);
//	l->index--;
//	if(l->index < 0)
//		l->index = 0;
}

Token* lexer_read_characters(Lexer *lexer, Token *t, k_ETokenType token_type, int (*cond)(u8 ch, int* undo))
{
	// https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
	u64 prime = 0x00000100000001B3;
	u64 offset = 0xcbf29ce484222325;

	u64 hash = offset;

	t->token_type = token_type;
	//t->position = lexer->index;
	t->position = lexer->stream->tell(lexer->stream->ctx);
	int n = 0;
	while(1)
	{
		u8 ch = lexer_read_and_advance(lexer);
		if(!ch)
		{
			//lexer_error(lexer, "Unexpected EOF");
			break;
		}
		int undo = 0;
		if(cond(ch, &undo))
		{
			if(undo)
			{
				lexer_unget(lexer);
			}
			break;
		}
		//if(n >= sizeof(t->string_value) - 1)
			//lexer_error(lexer, "n >= sizeof(t->string_value) - 1");
		//t->string_value[n] = ch;
		++n;
		
		hash ^= ch;
		hash *= prime;
	}
	t->hash = hash;
	//t->string_value[n] = 0;
	t->length = n;
	return t;
}

int cond_string(u8 ch, int *undo)
{
	*undo = 0;
	return ch == '"';
}
int cond_numeric(u8 ch, int *undo)
{
	*undo = 1;
	return !(ch >= '0' && ch <= '9') && ch != '.' && ch != 'e';
}
int cond_ident(u8 ch, int *undo)
{
	*undo = 1;
	return !(ch >= 'a' && ch <= 'z') && !(ch >= 'A' && ch <= 'Z') && ch != '_';
}

void lexer_push_token(Lexer *lexer, Token *t)
{
	Token **list_end = &lexer->tokens_end;
	if(list_end)
	{
		(*list_end)->next = t;
		*list_end = t;
	}
}

int lexer_step(Lexer *lexer, Token *t)
{
	s32 index;
	
	t->next = NULL;
	t->length = 1;
	t->hash = 0;

	u8 ch = 0;
repeat:
	index = lexer->stream->tell(lexer->stream->ctx);
	t->position = index;
	
	ch = lexer_read_and_advance(lexer);
	if(!ch)
		return 1;
	t->token_type = ch;
	switch(ch)
	{
		case '"':
			lexer_read_characters(lexer, t, k_ETokenTypeString, cond_string);
		break;
		
		case '\t':
		case ' ':
		case '\r':
		case '\n':
			goto repeat;
		
		default:
		{
			if(ch >= '0' && ch <= '9')
			{
				lexer_unget(lexer);
				lexer_read_characters(lexer, t, k_ETokenTypeNumber, cond_numeric);
			} else if((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_')
			{
				lexer_unget(lexer);
				lexer_read_characters(lexer, t, k_ETokenTypeIdentifier, cond_ident);
			} else
			{
				//if(ch >= 0x20 && ch <= 0x7e)
				if(!(ch >= 0x20 && ch <= 0xff))
				{
					printf("%d\n", ch);
					lexer_error(lexer, "Unexpected character");
				}
			}
		} break;
	}
	return 0;
}

Token *lexer_parse(Lexer *lexer)
{
	while(1)
	{
		Token *t = new(lexer->arena, Token, 1);
		if(lexer_step(lexer, t))
			break;
		lexer_push_token(lexer, t);
	}
	return lexer->tokens->next;
}

#endif