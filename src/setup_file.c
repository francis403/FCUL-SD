#include "setup_file.h"

#include <string.h>
#include <stdio.h>

FILE *file;

int open_file(const char *name) {
	file = fopen(name, "a+");
	return (file != NULL) - 1;
}

int close_file() {
	if (file != NULL && fclose(file) == 0)
		return 0;
	return -1;
}

int write_to_file(const char *key, const char *value) {
	if (file == NULL || key == NULL || value == NULL)
		return -1;
	char data[strlen(key) + strlen(value) + 3];
	strcpy(data, key);
	strcat(data, " ");
	strcat(data, value);
	strcat(data, "\n");
	if (fputs(data, file) >= 0)
		return 0;
	else
		return -1;
}

int read_from_file(const char *key, char *result) {
	if (file == NULL || key == NULL)
		return -1;
	rewind(file);
	char buff_key[100];
	buff_key[0] = '\0';
	while(strcmp(buff_key, key) != 0)
		if (fscanf(file, "%s%s", buff_key, result) == EOF)
			return -1;
	return 0;
}
