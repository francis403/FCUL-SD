#include "primary_backup-private.h"

int hello (Server *server) {
	if (server == NULL || server->isConnected == 0)
		return -1;
	Message *h = malloc(sizeof(Message));
	if (h == NULL)
		return -1;
	h->opcode = OC_HELLO;
	h->c_type = CT_RESULT;
	h->content.result = 0;
	Message *r = send_receive(server, h);//network_client.c
	free_message(h);
	if (r == NULL)
		return -1;
	int result = r->content.result;
	free_message(r);
	return result;
}

int update_state(Server *server) {
	if (server == NULL || server->isConnected == 0)
		return -1;
	Message *pedido = malloc(sizeof(Message));
	if (pedido == NULL)
		return -1;
	char *key = malloc(2);
	if (key == NULL)
		return -1;
	strcpy(key, "*");
	pedido->opcode = OC_UPDATE_STATE;
	pedido->c_type = CT_KEY;
	pedido->content.key = key;
	Message *respostaKeys = send_receive(server, pedido);
	free_message(pedido);
	if (respostaKeys == NULL || respostaKeys->opcode != OC_GET + 1)
		return -1;
	char **keys = respostaKeys->content.keys;
	int i;
	Message *resposta = NULL;
	for(i = 0; keys[i] != NULL; i++) {
		Data *data = table_skel_get(keys[i]);
		if (data == NULL) {
			pedido = malloc(sizeof(Message));
			if (pedido == NULL)
				continue;
			key = malloc(strlen(keys[i]) + 1);
			int len = strlen(keys[i]) + 1;
			if (key == NULL) {
				free_message(pedido);
				continue;
			}
			memcpy(key, keys[i], len);
			pedido->opcode = OC_GET;
			pedido->c_type = CT_KEY;
			pedido->content.key = key;
			resposta = send_receive(server, pedido);
			if (resposta != NULL && resposta->opcode == OC_GET + 1)
				table_skel_put(key, resposta->content.data);
			free_message(pedido);
			free_message(resposta);
		}
		data_destroy(data);
	}
	free_message(respostaKeys);
	pedido = malloc(sizeof(Message));
	if (pedido == NULL)
		return -1;
	pedido->opcode = OC_UPDATE_STATE + 1;
	pedido->c_type = CT_RESULT;
	pedido->content.result = 0;
	resposta = send_receive(server, pedido);
	free_message(pedido);
	if (resposta == NULL || resposta->opcode != OC_UPDATE_STATE + 1) {
		free_message(resposta);
		return -1;
	}
	free_message(resposta);
	return 0;
}
