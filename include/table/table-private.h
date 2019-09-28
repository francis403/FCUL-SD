/*
* Grupo 13
* Francisco de Almeida Araujo, 45701
* Nuno Burnay, 46406
* Robin Vassantlal, 46408
*/

#ifndef _TABLE_PRIVATE_H
#define _TABLE_PRIVATE_H

#include "table.h"
#include <string.h>

struct table_t{
	List **table;
	int size; /* Dimensão da tabela */
	int num_key_values;//Quantidades de pares chave-valor
};

typedef struct table_t Table;

int key_hash(char *key, int l);


#endif
