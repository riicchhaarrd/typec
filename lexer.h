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
	k_ETokenTypeComment,
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
	static const char *type_strings[] = {"identifier", "string", "number", "comment"};
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

// TODO: use a stream block altough I wrote a quick implementation and it seemed slower
typedef struct
{
	FILE *fp;
	s32 index;
	int cached;
	char data[2048];
} StreamBlock;

int lexer_stream_read_file(void *ctx, u8 *b, s32 n)
{
	FILE *fp = ctx;
	return fread(b, n, 1, fp);
}
int lexer_stream_seek_file(void *ctx, s32 n)
{
	FILE *fp = ctx;
	return fseek(fp, n, SEEK_SET);
}
s32 lexer_stream_tell_file(void *ctx)
{
	FILE *fp = ctx;
	return ftell(fp);
}

void lexer_stream_init_file(LexerStream *ls, FILE *fp)
{
	ls->ctx = fp;
	ls->read = lexer_stream_read_file;
	ls->seek = lexer_stream_seek_file;
	ls->tell = lexer_stream_tell_file;
}

typedef enum
{
	k_ELexerFlagNone = 0,
	k_ELexerFlagSkipComments = 1
} k_ELexerFlags;

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
	int flags;
} Lexer;
#pragma pack(pop)

void lexer_init(Lexer *l, Arena *arena, LexerStream *stream)
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
	if(setjmp(l->jmp_error))
	{
		exit(1);
	}
}

void lexer_token_read_string(Lexer *lexer, Token *t, char *temp, s32 max_temp_size)
{
	LexerStream *ls = lexer->stream;
	s32 pos = ls->tell(ls->ctx);
	ls->seek(ls->ctx, t->position);
	s32 n = max_temp_size - 1;
	if(t->length < n)
		n = t->length;
	ls->read(ls->ctx, temp, n);
	temp[n] = 0;
	ls->seek(ls->ctx, pos);
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


Token* lexer_read_string(Lexer *lexer, Token *t)
{
	// https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
	u64 prime = 0x00000100000001B3;
	u64 offset = 0xcbf29ce484222325;

	u64 hash = offset;

	t->token_type = k_ETokenTypeString;
	//t->position = lexer->index;
	t->position = lexer->stream->tell(lexer->stream->ctx);
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
	return !(ch >= 'a' && ch <= 'z') && !(ch >= 'A' && ch <= 'Z') && ch != '_' && !(ch >= '0' && ch <= '9');
}
int cond_single_line_comment(u8 ch, int *undo)
{
	*undo = 1;
	//\0 is implicitly handled by the if(!ch) check in lexer_read_characters
	return ch == '\r' || ch == '\n';
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

int lexer_accept(Lexer *lexer, k_ETokenType tt, Token *t)
{
	Token _;
	if(!t)
		t = &_;
	s32 pos = lexer->stream->tell(lexer->stream->ctx);
	if(lexer_step(lexer, t))
	{
		// Unexpected EOF
		longjmp(lexer->jmp_error, 1);
	}
	if(tt != t->token_type)
	{
		// Undo
		lexer->stream->seek(lexer->stream->ctx, pos);
		return 1;
	}
	return 0;
}

void lexer_expect(Lexer *lexer, k_ETokenType tt, Token *t)
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

int lexer_step(Lexer *lexer, Token *t)
{
	s32 index;
	
	t->next = NULL;
	t->length = 1;

	u8 ch = 0;
repeat:
	index = lexer->stream->tell(lexer->stream->ctx);
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
		
		case '\t':
		case ' ':
		case '\r':
		case '\n':
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
			lexer_read_characters(lexer, t, k_ETokenTypeComment, cond_single_line_comment);
			if(lexer->flags & k_ELexerFlagSkipComments)
				goto repeat;
		} break;
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