/*Grupo 13
* Francisco de Almeida Araujo, 45701
* Nuno Burnay, 46406
* Robin Vassantlal, 46408
*/
#include "entry-private.h"

Entry *entry_create(char *key, Data *data) {
	if (key == NULL || data == NULL)
		return NULL;

	Entry *result = malloc(sizeof(Entry));
	if (result == NULL)
		return NULL;
	char *key_temp = malloc(strlen(key) + 1);
	if (key_temp == NULL) {
		free(result);
		return NULL;
	}

	memcpy(key_temp, key, strlen(key) + 1);
	result->key = key_temp;

	Data *data_temp = data_dup(data);
	if (data_temp == NULL) {
		free(result->key);
		free(result);
		return NULL;
	}

	result->value = data_temp;
	
	return result;
}

void entry_destroy(Entry *entry) {
	if (entry == NULL)
		return;
	data_destroy(entry->value);
	free(entry->key);
	free(entry);
}

Entry *entry_dup(Entry *entry) {
	if (entry == NULL)
		return NULL;
	Entry *result = malloc(sizeof(Entry));
	if (result == NULL)
		return NULL;
	char *key_temp = malloc(strlen(entry->key) + 1);
	if (key_temp == NULL) {
		free(result);
		return NULL;
	}
	memcpy(key_temp, entry->key, strlen(entry->key) + 1);
	result->key = key_temp;
	Data *data_temp = data_dup(entry->value);
	if (data_temp == NULL) {
		free(result->key);
		free(result);
		return NULL;
	}	
	result->value = data_temp;
	
	return result;
}

