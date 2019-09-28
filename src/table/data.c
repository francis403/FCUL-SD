/*Grupo 13
* Francisco de Almeida Araujo, 45701
* Nuno Burnay, 46406
* Robin Vassantlal, 46408
*/
#include "data-private.h"

Data *data_create(int size) {
	if (size <= 0)
		return NULL;

	Data *result = malloc(sizeof(Data));
	
	if (result == NULL)
		return NULL;

	result->datasize = size;
	result->data = malloc(size);
	if (result->data == NULL) {
		free(result);
		return NULL;
	}
	return result;
}

Data *data_create2(int size, void *data) {
	if (data == NULL)
		return NULL;
	Data *result = data_create(size);

	if (result == NULL)
		return NULL;
	memcpy(result->data, data, size);
	
	return result;
}

void data_destroy(Data *data) {
	if (data == NULL)
		return;
	free(data->data);
	free(data);
}

Data *data_dup(Data *data) {
	if (data == NULL)
		return NULL;

	return data_create2(data -> datasize, data -> data);
}