/*Grupo 13
* Francisco de Almeida Araujo, 45701
* Nuno Burnay, 46406
* Robin Vassantlal, 46408
*/
#include "list-private.h"

List *list_create() {
	List *result = malloc(sizeof(List));
	if(result == NULL)
		return NULL;
	
	result->root = NULL;
	result->size = 0;
	return result;
}

void list_destroy(List *list) {
	if (list == NULL)
		return;
	freeNodes(list->root);
	free(list);
}

void freeNodes(Node *n) {
	if (n != NULL) {
		freeNodes(n->next);
		entry_destroy(n->data);
		free(n);
	}
}

int list_add(List *list, Entry *entry) {
	if (list == NULL || entry == NULL)
		return -1;
	Node *n = list->root;
	Node *preN = NULL;
	while (n != NULL && strcmp(n->data->key, entry->key) > 0) {
		preN = n;
		n = n->next;
	}
	Entry *entry_temp = entry_dup(entry);
	if (entry_temp == NULL)
		return -1;
	if (n != NULL && strcmp(n->data->key, entry->key) == 0) {
		entry_destroy(n->data);
		n->data = entry_temp;
	}
	else {
		Node *newNode;
		newNode = malloc(sizeof(Node));
		if (newNode == NULL) {
			free(entry_temp);
			return -1;
		}
		newNode->data = entry_temp;
		newNode->next = n;
		
		if (preN != NULL)
			preN->next = newNode;
		else
			list->root = newNode;
		list->size = list->size + 1;
	}
	
	return 0;
}

int list_remove(List *list, char *key) {
	if (list == NULL || key == NULL)
		return -1;
	Node *n = list->root;
	Node *preN = NULL;
	
	while (n != NULL && strcmp(n->data->key, key) > 0) {
		preN = n;
		n = n->next;
	}
	if (n != NULL && strcmp(n->data->key, key) == 0) {
		if (preN == NULL)
			list->root = n->next;
		else
			preN->next = n->next;
		list->size = list->size - 1;
		entry_destroy(n->data);
		free(n);
		return 0;
	}
	else
		return -1;
}

Entry *list_get(List *list, char *key) {
	if (list == NULL || key == NULL)
		return NULL;
	Node *n = list->root;
	
	while (n != NULL && strcmp(n->data->key, key) > 0)
		n = n->next;
	if (n != NULL && strcmp(n->data->key, key) == 0)
		return n->data;
	else
		return NULL;
}

int list_size(List *list) {
	if (list == NULL)
		return -1;
	return list->size;
}

char **list_get_keys(List *list) {
	if (list == NULL)
		return NULL;
	char **vetor = malloc(list->size * sizeof(char*) 
			+ sizeof(NULL));
	if (vetor == NULL)
		return NULL;
	Node *n = list->root;
	int i = 0;
	bool b = true;
	while(n != NULL && b) {
		int len = strlen(n->data->key) + 1;
		vetor[i] = malloc(len);
		b = vetor[i] != NULL;
		if (b) {
			memcpy(vetor[i], n->data->key, len);
			i++;
			n = n->next;
		}
	}
	if (!b) {
		while (i >= 0)
			free(vetor[i--]);
		free(vetor);
		return NULL;
	}
	vetor[i] = NULL;
	return vetor;
}

void list_free_keys(char **keys) {
	if (keys == NULL)
		return;
	int i = 0;
	while(keys[i] != NULL)
		free(keys[i++]);
	free(keys[i]);
	free(keys);
}

