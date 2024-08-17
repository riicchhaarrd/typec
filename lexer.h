#ifndef PARSE_H
#define PARSE_H
#include "stream.h"

#include "types.h"

#define LEXER_STATIC static

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
	TOKEN_TYPE_IDENTIFIER = 256,
	TOKEN_TYPE_STRING,
	TOKEN_TYPE_NUMBER,
	TOKEN_TYPE_COMMENT,
	TOKEN_TYPE_MAX
} TokenType;

LEXER_STATIC const char *token_type_to_string(TokenType token_type, char *string_out, int string_out_size)
{
	if(token_type >= TOKEN_TYPE_MAX)
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
	static const char *type_strings[] = {"identifier", "string", "number", "comment"};
	return type_strings[token_type - 256];
}

#pragma pack(push, 1)
typedef struct Token_s
{
	struct Token_s *next;
	s64 position;
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

typedef enum
{
	k_ELexerFlagNone = 0,
	k_ELexerFlagSkipComments = 1,
	k_ELexerFlagTokenizeNewlines = 2,
	k_ELexerFlagIdentifierIncludesHyphen = 4
} k_ELexerFlags;

typedef struct
{
	Stream *stream;
//	const char *input_stream;
	Token *tokens;
	Token *tokens_end;
	Arena *arena;
//	u32 index;
//	u32 size;
	jmp_buf jmp_error;
	int flags;
} Lexer;
#pragma pack(pop)
LEXER_STATIC int lexer_step(Lexer *lexer, Token *t);

LEXER_STATIC void lexer_init(Lexer *l, Arena *arena, Stream *stream)
{
	//l->index = 0;
	//l->size = strlen(s);
	//l->input_stream = s;
	l->stream = stream;
	l->arena = arena;
	l->flags = k_ELexerFlagNone;
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
	#if 0
	if(setjmp(l->jmp_error))
	{
		exit(1);
	}
	#endif
}

LEXER_STATIC void lexer_token_read_string(Lexer *lexer, Token *t, char *temp, s32 max_temp_size)
{
	Stream *ls = lexer->stream;
	s32 pos = ls->tell(ls);
	ls->seek(ls, t->position, SEEK_SET);
	s32 n = max_temp_size - 1;
	if(t->length < n)
		n = t->length;
	ls->read(ls, temp, 1, n);
	temp[n] = 0;
	ls->seek(ls, pos, SEEK_SET);
}

LEXER_STATIC u8 lexer_read_and_advance(Lexer *l)
{
	u8 buf = 0;
	if(l->stream->read(l->stream, &buf, 1, 1) != 1)
		return 0;
	return buf;
	//if(l->index < 0 || l->index >= l->size)
		//return 0;
	//return l->input_stream[l->index++];
}

LEXER_STATIC void lexer_error(Lexer *l, const char *message)
{
	printf("Lexer error: %s\n", message);
	longjmp(l->jmp_error, 1);
}

LEXER_STATIC void lexer_unget(Lexer *l)
{
	int64_t current = l->stream->tell(l->stream);
	if(current == 0)
		return;
	l->stream->seek(l->stream, current - 1, SEEK_SET);
//	l->index--;
//	if(l->index < 0)
//		l->index = 0;
}


LEXER_STATIC Token* lexer_read_string(Lexer *lexer, Token *t)
{
	// https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
	u64 prime = 0x00000100000001B3;
	u64 offset = 0xcbf29ce484222325;

	u64 hash = offset;

	t->token_type = TOKEN_TYPE_STRING;
	//t->position = lexer->index;
	t->position = lexer->stream->tell(lexer->stream);
	int n = 0;
	int escaped = 0;
	while(1)
	{
		u8 ch = lexer_read_and_advance(lexer);
		if(!ch)
		{
			//lexer_error(lexer, "Unexpected EOF");
			break;
		}
		if(ch == '"' && !escaped)
			break;
		escaped = (!escaped && ch == '\\');
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

LEXER_STATIC Token* lexer_read_characters(Lexer *lexer, Token *t, TokenType token_type, int (*cond)(u8 ch, int* undo))
{
	// https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
	u64 prime = 0x00000100000001B3;
	u64 offset = 0xcbf29ce484222325;

	u64 hash = offset;

	t->token_type = token_type;
	//t->position = lexer->index;
	t->position = lexer->stream->tell(lexer->stream);
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

LEXER_STATIC int cond_string(u8 ch, int *undo)
{
	*undo = 0;
	return ch == '"';
}
LEXER_STATIC int cond_numeric(u8 ch, int *undo)
{
	*undo = 1;
	return !(ch >= '0' && ch <= '9') && ch != '.' && ch != 'e' && ch != 'f';
}
LEXER_STATIC int cond_ident(u8 ch, int *undo)
{
	*undo = 1;
	return !(ch >= 'a' && ch <= 'z') && !(ch >= 'A' && ch <= 'Z') && ch != '_' && !(ch >= '0' && ch <= '9');
}
LEXER_STATIC int cond_single_line_comment(u8 ch, int *undo)
{
	*undo = 1;
	//\0 is implicitly handled by the if(!ch) check in lexer_read_characters
	return ch == '\r' || ch == '\n';
}

LEXER_STATIC void lexer_push_token(Lexer *lexer, Token *t)
{
	Token **list_end = &lexer->tokens_end;
	if(list_end)
	{
		(*list_end)->next = t;
		*list_end = t;
	}
}

LEXER_STATIC int lexer_accept(Lexer *lexer, TokenType tt, Token *t)
{
	Token _;
	if(!t)
		t = &_;
	s64 pos = lexer->stream->tell(lexer->stream);
	if(lexer_step(lexer, t))
	{
		// Unexpected EOF
		longjmp(lexer->jmp_error, 1);
	}
	if(tt != t->token_type)
	{
		// Undo
		lexer->stream->seek(lexer->stream, pos, SEEK_SET);
		return 1;
	}
	return 0;
}

LEXER_STATIC void lexer_expect(Lexer *lexer, TokenType tt, Token *t)
{
	Token _;
	if(!t)
		t = &_;
	if(lexer_accept(lexer, tt, t))
	{
		char expected[64];
		char got[64] = {0};
		char temp[64] = {0};
		token_type_to_string(tt, expected, sizeof(expected));
		token_type_to_string(t->token_type, got, sizeof(got));
		lexer_token_read_string(lexer, t, temp, sizeof(temp));
		printf("Expected '%s', got '%s' %d '%s'\n", expected, got, t->token_type, temp);
		longjmp(lexer->jmp_error, 1); // TODO: pass error enum type value
	}
}

LEXER_STATIC int lexer_step(Lexer *lexer, Token *t)
{
	int64_t index;
	
	t->next = NULL;
	t->length = 1;

	u8 ch = 0;
repeat:
	index = lexer->stream->tell(lexer->stream);
	t->position = index;
	
	ch = lexer_read_and_advance(lexer);
	if(!ch)
		return 1;
	t->hash = (0xcbf29ce484222325 ^ ch) * 0x00000100000001B3;
	t->token_type = ch;
	switch(ch)
	{
		case '"':
			lexer_read_string(lexer, t);
		break;
		
		case '\n':
		if(lexer->flags & k_ELexerFlagTokenizeNewlines)
			return 0;
		case '\t':
		case ' ':
		case '\r':
			goto repeat;
		case '/':
		{
			ch = lexer_read_and_advance(lexer);
			// TODO: handle multi-line comment /*
			if(!ch || ch != '/')
			{
				lexer_unget(lexer); // We'll get \0 the next time we call lexer_step
				return 0;
			}
			lexer_read_characters(lexer, t, TOKEN_TYPE_COMMENT, cond_single_line_comment);
			if(lexer->flags & k_ELexerFlagSkipComments)
				goto repeat;
		} break;
		default:
		{
			if(ch >= '0' && ch <= '9')
			{
				lexer_unget(lexer);
				lexer_read_characters(lexer, t, TOKEN_TYPE_NUMBER, cond_numeric);
			} else if((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_')
			{
				lexer_unget(lexer);
				lexer_read_characters(lexer, t, TOKEN_TYPE_IDENTIFIER, cond_ident);
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

LEXER_STATIC Token *lexer_parse(Lexer *lexer)
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
