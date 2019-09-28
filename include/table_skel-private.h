/*
* Grupo 13
* Francisco de Almeida Araujo, 45701
* Nuno Burnay, 46406
* Robin Vassantlal, 46408
*/

#ifndef _TABLE_SKEL_PRIVATE_H
#define _TABLE_SKEL_PRIVATE_H

#include "table_skel.h"
#include "message-private.h"
#include "table-private.h"

char **table_skel_get_keys();

int table_skel_put(char * key, Data *value);

Data *table_skel_get(char *key);

void table_skel_free_keys(char **keys);

#endif
