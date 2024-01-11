#ifndef UTIL_H
#define UTIL_H
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
#endif