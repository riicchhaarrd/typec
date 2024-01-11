#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "arena.h"
#include "lexer.h"
#include "hash.h"
#include "util.h"
//#include "parse/cstruct.h"
#include "parse/type.h"

typedef struct
{
	const char *mode;
} CompilerOptions;

void error(const char *fmt)
{
	printf("%s\n", fmt);
	exit(0);
}

int main(int argc, const char **argv, char **envp)
{
	Arena scratch = { 0 };
	char temp[16000]; //16KB
	size_t bytes = sizeof(temp);
	
	//size_t bytes = 64000000; // 64MB
	//char *temp = malloc(bytes);
	arena_init(&scratch, temp, bytes);

	// getchar();
	CompilerOptions opts = { 0 };

	for(int i = 0; i < argc; ++i)
	{
		if(!strcmp(argv[i], "-mode"))
		{
			opts.mode = argv[++i];
		}
		else if(!strcmp(argv[i], "-script"))
		{
			const char *path = argv[++i];
			parse_type_file(path, &scratch);
#if 0
			char *buffer;
			if(!read_file(path, &scratch, &buffer))
			{
				test(buffer, &scratch);
			}
#endif
		}
	}
	printf("memory used: %d KB\n", (bytes - (scratch.end - scratch.beg)) / 1000);
	// printf("mode = %s\n", opts.mode);
	return 0;
}
