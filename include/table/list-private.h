/*
* Grupo 13
* Francisco de Almeida Araujo, 45701
* Nuno Burnay, 46406
* Robin Vassantlal, 46408
*/

#ifndef _LIST_PRIVATE_H
#define _LIST_PRIVATE_H

#include "list.h"
#include <string.h>
#include <stdlib.h>
#include <stdbool.h> //alteracoe para projeto 2

struct list_t {
	struct node *root;
	int size;
};

struct node {
	struct entry_t *data;
	struct node *next;
};

typedef struct list_t List;
typedef struct node Node;
typedef struct entry_t Entry;

void freeNodes(Node *n);

#endif
