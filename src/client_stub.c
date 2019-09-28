/*
* Grupo 13
* Francisco de Almeida Araujo, 45701
* Nuno Burnay, 46406
* Robin Vassantlal, 46408
*/

#include "client_stub-private.h"

RemoteTable *rtable_bind(const char *address_port) {
	if (address_port == NULL)
		return NULL;
	Server *server = network_connect(address_port);
	if (server == NULL)
		return NULL;
	RemoteTable *rtable = malloc(sizeof(RemoteTable));
	if (rtable == NULL) {
		network_close(server);
		return NULL;
	}
	rtable->server = server;
	return rtable;
}

int rtable_unbind(RemoteTable *rtable) {
	if (rtable == NULL)
		return -1;
	network_close(rtable->server);
	free(rtable);
	return 0;
}

int rtable_put(RemoteTable *rtable, char *key, Data *value) {
	if (rtable == NULL || key == NULL || value == NULL)
		return -1;
	if (key[0] == '*') //chaves nao podem começar com *
		return -1;
	Message *msg_out = malloc(sizeof(Message));
	if (msg_out == NULL)
		return -1;
	msg_out->opcode = OC_PUT;
	msg_out->c_type = CT_ENTRY;
	
	Entry *e = entry_create(key, value);
	if (e == NULL) {
		free_message(msg_out);
		return -1;
	}
	msg_out->content.entry = e;

	Message *msg_in = network_send_receive(rtable->server, msg_out);
	free_message(msg_out);
	
	if (msg_in == NULL)
		return -1;
	if (msg_in->opcode == OC_PUT + 1 && msg_in->content.result == 0) {
		free_message(msg_in);
		return 0;
	}
	free_message(msg_in);
	return -1;
}

int rtable_update(RemoteTable *rtable, char *key, Data *value) {
	if (rtable == NULL || key == NULL || value == NULL)
		return -1;

	Message *msg_out = malloc(sizeof(Message));
	if (msg_out == NULL)
		return -1;
	msg_out->opcode = OC_UPDATE;
	msg_out->c_type = CT_ENTRY;
	
	Entry *e = entry_create(key, value);
	if (e == NULL) {
		free_message(msg_out);
		return -1;
	}
	msg_out->content.entry = e;

	Message *msg_in = network_send_receive(rtable->server, msg_out);
	free_message(msg_out);
	
	if (msg_in == NULL)
		return -1;
	if (msg_in->opcode == OC_UPDATE + 1 && msg_in->content.result == 0) {
		free_message(msg_in);
		return 0;
	}
	free_message(msg_in);
	return -1;
}

Data *rtable_get(RemoteTable *rtable, char *key) {
	if (rtable == NULL || key == NULL)
		return NULL;
	Message *msg_out = malloc(sizeof(Message));
	if (msg_out == NULL)
		return NULL;
	
	char *k = malloc(sizeof(char *) * strlen(key) + 1);
	if (k == NULL) {
		free_message(msg_out);
		return NULL;
	}
	memcpy(k, key, strlen(key) + 1);
	msg_out->opcode = OC_GET;
	msg_out->c_type = CT_KEY;
	msg_out->content.key = k;

	Message *msg_in = network_send_receive(rtable->server, msg_out);
	free_message(msg_out);

	if (msg_in == NULL)
		return NULL;
	if (msg_in->opcode == OC_GET + 1) {
		Data *d;
		if (msg_in->content.data->datasize != 0)
			d = data_dup(msg_in->content.data);
		else {
			d = malloc(sizeof(Data));
			if (d == NULL) {
				free_message(msg_in);
				return NULL;
			}
			d->datasize = 0;
			d->data = NULL;
		}
		free_message(msg_in);
		return d;
	}
	free_message(msg_in);
	return NULL;
}

int rtable_del(RemoteTable *rtable, char *key) {
	if (rtable == NULL || key == NULL)
		return -1;
	Message *msg_out = malloc(sizeof(Message));
	if (msg_out == NULL)
		return -1;
	char *k = malloc(sizeof(char *) * strlen(key) + 1);
	if (k == NULL) {
		free_message(msg_out);
		return -1;
	}
	memcpy(k, key, strlen(key) + 1);
	msg_out->opcode = OC_DEL;
	msg_out->c_type = CT_KEY;
	msg_out->content.key = k;

	Message *msg_in = network_send_receive(rtable->server, msg_out);
	free_message(msg_out);

	if (msg_in == NULL)
		return -1;
	if (msg_in->opcode == OC_DEL + 1 && msg_in->content.result == 0) {
		free_message(msg_in);
		return 0;
	}
	free_message(msg_in);
	return -1;
}

int rtable_size(RemoteTable *rtable) {
	if (rtable == NULL)
		return -1;
	Message *msg_out = malloc(sizeof(Message));
	if (msg_out == NULL)
		return -1;
	msg_out->opcode = OC_SIZE;
	msg_out->c_type = 0;

	Message *msg_in = network_send_receive(rtable->server, msg_out);
	free_message(msg_out);

	if (msg_in == NULL)
		return -1;
	if (msg_in->opcode == OC_SIZE + 1) {
		int size = msg_in->content.result;
		free_message(msg_in);
		return size;
	}
	free_message(msg_in);
	return -1;
}

char **rtable_get_keys(RemoteTable *rtable) {
	if (rtable == NULL)
		return NULL;
	Message *msg_out = malloc(sizeof(Message));
	if (msg_out == NULL)
		return NULL;
	char *key = malloc(sizeof(char *) + 1);
	if (key == NULL) {
		free_message(msg_out);
		return NULL;
	}
	memcpy(key, "*", 1);
	key[1] = '\0';
	msg_out->opcode = OC_GET;
	msg_out->c_type = CT_KEY;
	msg_out->content.key = key;

	Message *msg_in = network_send_receive(rtable->server, msg_out);
	free_message(msg_out);

	if (msg_in == NULL)
		return NULL;
	if (msg_in->opcode == OC_GET + 1 && msg_in->content.keys != NULL) {
		char **keys = copy_keys(msg_in->content.keys);
		free_message(msg_in);
		return keys;
	}
	free_message(msg_in);
	return NULL;
}

void rtable_free_keys(char **keys) {
	table_free_keys(keys);
}

char **copy_keys(char **keys) {
	int countKeys = 0;
	while (keys[countKeys++] != NULL);

	char **result = malloc(countKeys * sizeof(char*) 
			+ sizeof(NULL));
	if (result == NULL)
		return NULL;

	int i = 0;
	bool b = true;
	while(keys[i] != NULL && b) {
		int len = strlen(keys[i]) + 1;
		result[i] = malloc(len);
		b = result[i] != NULL;
		if (b) {
			memcpy(result[i], keys[i], len);
			i++;
		}
	}
	if (!b) {
		while (i >= 0)
			free(result[i--]);
		free(result);
		return NULL;
	}
	result[i] = NULL;
	return result;
}
