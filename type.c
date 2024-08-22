#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "arena.h"
#include "lexer.h"
#include "hash.h"
#include "util.h"
//#include "parse/cstruct.h"
#include "opts.h"
#include "parse/type.h"

void error(const char *fmt)
{
	printf("%s\n", fmt);
	exit(0);
}

static void pathinfo(const char *path,
					 char *directory, size_t directory_max_length,
					 char *basename, size_t basename_max_length,
					 char *extension, size_t extension_max_length, char *sep)
{

	size_t offset = 0;
	const char *it = path;
	while(*it)
	{
		if(*it == '/' || *it == '\\')
		{
			if(sep)
				*sep = *it;
			offset = it - path;
		}
		++it;
	}
	directory[0] = 0;
	snprintf(directory, directory_max_length, "%.*s", offset, path);
	const char *filename = path + offset;
	
	if(*filename == '/' || *filename == '\\')
		++filename;

	char *delim = strrchr(filename, '.');
	basename[0] = 0;
	extension[0] = 0;
	if(!delim)
	{
		snprintf(basename, basename_max_length, "%s", filename);
	} else
	{
		snprintf(basename, basename_max_length, "%.*s", delim - filename, filename);
		snprintf(extension, extension_max_length, "%s", delim + 1);
	}
}

int main(int argc, const char **argv, char **envp)
{
	char directory[256] = {0};
	char basename[256] = {0};
	char extension[256] = {0};
	char sep = 0;

	Arena scratch = { 0 };
	char temp[160000]; //160KB
	size_t bytes = sizeof(temp);
	
	//size_t bytes = 64000000; // 64MB
	//char *temp = malloc(bytes);
	arena_init(&scratch, temp, bytes);

	// getchar();
	CompilerOptions opts = {
		.mode = "c11", // _Generic
		.output = "none",
		.prefix = NULL
	};
	const char *path = NULL;

	for(int i = 0; i < argc; ++i)
	{
		if(!strcmp(argv[i], "-mode"))
		{
			opts.mode = argv[++i];
		} else if(!strcmp(argv[i], "-o"))
		{
			opts.output = argv[++i];
		}
		else if(!strcmp(argv[i], "-f"))
		{
			path = argv[++i];
			pathinfo(path,
						directory,
						sizeof(directory),
						basename,
						sizeof(basename),
						extension,
						sizeof(extension),
						&sep);
			opts.prefix = basename;
#if 0
			char *buffer;
			if(!read_file(path, &scratch, &buffer))
			{
				test(buffer, &scratch);
			}
#endif
		}
	}
	if(path && opts.prefix)
	{
		parse_type_file(path, &scratch, &opts);
	}
	// printf("memory used: %d KB\n", (bytes - (scratch.end - scratch.beg)) / 1000);
	// printf("mode = %s\n", opts.mode);
	return 0;
}
