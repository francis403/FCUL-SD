#ifndef _SETUP_FILE_H
#define _SETUP_FILE_H

int open_file(const char *name);

int close_file();

int write_to_file(const char *key, const char *value);

int read_from_file(const char *key, char *result);

#endif
