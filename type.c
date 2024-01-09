#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "arena.h"
#include "lexer.h"
#include "hash.h"

typedef struct
{
	const char *mode;
} CompilerOptions;

int read_file(const char *path, Arena *arena, char **buffer_out)
{
	FILE *fp = fopen(path, "rb");
	if(!fp)
		return 1;
	
	fseek(fp, 0, SEEK_END);
	int n = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	
	char *buffer = new(arena, char, n);
	fread(buffer, n, 1, fp);
	buffer[n] = 0;
	fclose(fp);
	*buffer_out = buffer;
	return 0;
}

//TODO: use a stream block altough I wrote a quick implementation and it seemed slower
typedef struct
{
	FILE *fp;
	s32 index;
	int cached;
	char data[2048];
} StreamBlock;

int ls_read(void* ctx, u8* b, s32 n)
{
	FILE *fp = ctx;
	return fread(b, n, 1, fp);
}
int ls_seek(void* ctx, s32 n)
{
	FILE *fp = ctx;
	return fseek(fp, n, SEEK_SET);
}
s32 ls_tell(void* ctx)
{
	FILE *fp = ctx;
	return ftell(fp);
}

void test(const char *path, Arena *arena)
{
	//printf("script = '%s'\n", script);
	
	//const char *script = "\"test\" struct A {}";
	
	FILE *fp = fopen(path, "rb");
	if(!fp)
		return;
	
	Lexer lexer = {0};
	
	LexerStream ls =
	{
		.ctx = fp,
		.read = ls_read,
		.seek = ls_seek,
		.tell = ls_tell
	};
	lexer_init(&lexer, arena, &ls);
	
	char type_str[32];
	Token t;
	u64 needle = fnv1a_64("Test");
	while(!lexer_step(&lexer, &t))
	{
		const char *type = token_type_to_string(t.token_type, type_str, sizeof(type_str));
		//printf("%s %s\n", type, it->string_value);
		if(t.position != -1 && t.length > 0)
		{
			int verbose = 0;
			//printf("%s %llu\n", type, t.hash);
			if(verbose || t.hash == needle)
			{
				s32 pos = ls.tell(ls.ctx);
				ls.seek(ls.ctx, t.position);
				char temp[64];
				s32 n = sizeof(temp) - 1;
				if(t.length < n)
					n = t.length;
				ls.read(ls.ctx, temp, n);
				temp[n] = 0;
				printf("%s '%s'\n", type, temp);
				ls.seek(ls.ctx, pos);
			}
			//printf("%s %.*s\n", type, t.length, &script[t.position]);
		}
	}
	#if 0
	Token *it = lexer_parse(&lexer);
	while(it)
	{
		const char *type = token_type_to_string(it->token_type, type_str, sizeof(type_str));
		//printf("%s %s\n", type, it->string_value);
		if(it->position != -1 && it->length > 0)
		{
			//printf("%s %.*s\n", type, it->length, &script[it->position]);
		}
		it = it->next;
	}
	#endif
	fclose(fp);
}

int main(int argc, const char **argv, char **envp)
{	
	//char temp[8000000]; //8MB
	//char temp[100000]; //100KB
	Arena scratch = {0};
	//char temp[16000]; //16KB
	//arena_init(&scratch, temp, sizeof(temp));
	size_t bytes = 64000000; //64MB
	char *temp = malloc(bytes);
	arena_init(&scratch, temp, bytes);
	
	//getchar();
	CompilerOptions opts = {0};
	
	for(int i = 0; i < argc; ++i)
	{
		if(!strcmp(argv[i], "-mode"))
		{
			opts.mode = argv[++i];
		} else if(!strcmp(argv[i], "-script"))
		{
			const char *path = argv[++i];
			test(path, &scratch);
			#if 0
			char *buffer;
			if(!read_file(path, &scratch, &buffer))
			{
				test(buffer, &scratch);
			}
			#endif
		}
	}
	printf("memory used: %d MB\n", (bytes - (scratch.end - scratch.beg)) / 1000 / 1000);
	//printf("mode = %s\n", opts.mode);
	return 0;
}