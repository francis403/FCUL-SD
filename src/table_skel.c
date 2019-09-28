/*
* Grupo 13
* Francisco de Almeida Araujo, 45701
* Nuno Burnay, 46406
* Robin Vassantlal, 46408
*/

#include "table_skel-private.h"
#include <stdio.h>

Table *table;

int table_skel_init(int n_lists) {
	if (n_lists <= 0)
		return -1;
	table = table_create(n_lists);
	if (table == NULL)
		return -1;
	return 0;
}

int table_skel_destroy() {
	if (table == NULL)
		return -1;
	table_destroy(table);
	return 0;
}

Message *invoke(Message *msg_in) {
	if (msg_in == NULL || table == NULL)
		return NULL;
	Message *msg_out;
	
	/* Preparar mensagem de resposta */
	msg_out = malloc(sizeof(Message));	
	if (msg_out == NULL) {
		return NULL;
	}
	msg_out->opcode = OC_RT_ERROR;
	msg_out->c_type = CT_RESULT;
	msg_out->content.result = -1;
	
	/* Verificar parâmetros de entrada */
	if (msg_in == NULL || table == NULL)
		return msg_out;
	/* Verificar opcode e c_type na mensagem de pedido */
	int op = msg_in->opcode;
	int type = msg_in->c_type;
	if (op % 10 != 0 || op < 10 || op > 60 || type % 10 != 0
			|| type < 0 || type > 50)
		return msg_out;
		
	/* Aplicar operação na tabela */
	switch(op) {
		case OC_PUT:
			msg_out->opcode = OC_PUT + 1;
			msg_out->c_type = CT_RESULT;
			msg_out->content.result = table_put(table, msg_in->content.entry->key, msg_in->content.entry->value);
			break;
		case OC_GET:
			if (msg_in->content.key[0] == '*') {
				char **keys = table_get_keys(table);
				if (keys == NULL)
					break;
				msg_out->c_type = CT_KEYS;
				msg_out->content.keys = keys;
				msg_out->opcode = OC_GET + 1;
				break;
			}
			Data *d = table_get(table, msg_in->content.key);
			if (d == NULL) {
				d = malloc(sizeof(Data));
				if (d == NULL)
					break;
				d->datasize = 0;
				d->data = NULL;
				msg_out->c_type = CT_VALUE;
				msg_out->content.data = d;
			}
			else {
				msg_out->c_type = CT_VALUE;
				msg_out->content.data = d;
			}
			msg_out->opcode = OC_GET + 1;
			break;
		case OC_DEL:
			msg_out->opcode = OC_DEL + 1;
			msg_out->c_type = CT_RESULT;
			msg_out->content.result = table_del(table, msg_in->content.key);
			break;
		case OC_UPDATE:
			msg_out->opcode = OC_UPDATE + 1;
			msg_out->c_type = CT_RESULT;
			msg_out->content.result = table_update(table, msg_in->content.entry->key, msg_in->content.entry->value);
			break;
		case OC_SIZE:
			msg_out->opcode = OC_SIZE + 1;
			msg_out->c_type = CT_RESULT;
			msg_out->content.result = table->num_key_values;
			break;
	}

	return msg_out;
}

char **table_skel_get_keys() {
	if (table == NULL)
		return NULL;
	return table_get_keys(table);
}

int table_skel_put(char * key, Data *value) {
	return table_put(table, key, value);
}

Data *table_skel_get(char *key) {
	if (table == NULL || key == NULL)
		return NULL;
	return table_get(table, key);
}

void table_skel_free_keys(char **keys) {
	table_free_keys(keys);
}