/*
* Grupo 13
* Francisco de Almeida Araujo, 45701
* Nuno Burnay, 46406
* Robin Vassantlal, 46408
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>

#include "message-private.h"

void free_message(Message *msg){

  /* Verificar se msg é NULL */
	if (msg == NULL)
		return;
  /* Se msg->c_type for:
      VALOR, libertar msg->content.data
      ENTRY, libertar msg->content.entry_create
      CHAVES, libertar msg->content.keys
      CHAVE, libertar msg->content.key
  */
	switch(msg->c_type) {
		case CT_VALUE:
			data_destroy(msg->content.data);
			break;
		case CT_ENTRY:
			entry_destroy(msg->content.entry);
			break;
		case CT_KEYS:
			table_free_keys(msg->content.keys);
			break;
		case CT_KEY:
			free(msg->content.key);
			break;
	}
  /* libertar msg */
	free(msg);
}

int message_to_buffer(Message *msg, char **msg_buf){
  /* Verificar se msg é NULL */
	if(msg == NULL)
		return -1;
  /* Consoante o msg->c_type, determinar o tamanho do vetor de bytes
     que tem de ser alocado antes de serializar msg
  */
	int buffer_size = 4 + get_byte_quantity(msg);
  /* Alocar quantidade de memória determinada antes 
     *msg_buf = ....
  */
	*msg_buf = malloc(buffer_size);
	if (*msg_buf == NULL)
		return -1;
  /* Inicializar ponteiro auxiliar com o endereço da memória alocada */
	char *ptr;
	ptr = *msg_buf;
	
	short short_value;
	short_value = htons(msg->opcode);
	memcpy(ptr, &short_value, _SHORT);
	ptr += _SHORT;
	
	short_value = htons(msg->c_type);
	memcpy(ptr, &short_value, _SHORT);
	ptr += _SHORT;
  /* Consoante o conteúdo da mensagem, continuar a serialização da mesma */
	copy_content(msg, ptr);
  return buffer_size;
}

int get_byte_quantity(Message *msg) {
	if (msg == NULL)
		return 0;
	int size = 0;
	switch(msg->c_type) {
		case CT_RESULT:
			size = _INT;
			break;
		case CT_VALUE:
			size = _INT + msg->content.data->datasize;
			break;
		case CT_ENTRY:
			size = _SHORT + strlen(msg->content.entry->key) + _INT + msg->content.entry->value->datasize;
			break;
		case CT_KEYS:
			size = _INT; //numero de keys
			char **keys = msg->content.keys;
			int i = 0;
			while(keys[i] != NULL) {
				size += _SHORT + strlen(keys[i]);
				i++;
			}
			break;
		case CT_KEY:
			size = _SHORT + strlen(msg->content.key);
			break;
	}
	return size;
}

void copy_content(Message *msg, char *ptr) {
	if (msg == NULL || ptr == NULL)
		return;
	int int_value, i;
	short key_size, short_value;
	char *temp;
	switch(msg->c_type) {
		case CT_RESULT:
			int_value = htonl(msg->content.result);
			memcpy(ptr, &int_value, _INT);
			break;
		case CT_VALUE:
			int_value = htonl(msg->content.data->datasize);
			memcpy(ptr, &int_value, _INT);
			ptr += _INT;
			
			if (msg->content.data->datasize != 0)
				memcpy(ptr, msg->content.data->data, msg->content.data->datasize);
			break;
		case CT_ENTRY:
			key_size = strlen(msg->content.entry->key);
			short_value = htons(key_size);
			memcpy(ptr, &short_value, _SHORT);
			ptr += _SHORT;
			memcpy(ptr, msg->content.entry->key, key_size);
			ptr += key_size;
			
			int_value = htonl(msg->content.entry->value->datasize);
			memcpy(ptr, &int_value, _INT);
			ptr += _INT;
			memcpy(ptr, msg->content.entry->value->data, msg->content.entry->value->datasize);
			break;
		case CT_KEYS:
			temp = ptr;
			ptr += _INT;
			i = 0;
			while (msg->content.keys[i] != NULL) {
				short_value = htons(strlen(msg->content.keys[i]));
				memcpy(ptr, &short_value, _SHORT);
				ptr += _SHORT;
				memcpy(ptr, msg->content.keys[i], strlen(msg->content.keys[i]));
				ptr += strlen(msg->content.keys[i]);
				i++;
			}
			int_value = htonl(i);
			memcpy(temp, &int_value, _INT);
			break;
		case CT_KEY:
			int_value = htons(strlen(msg->content.key));
			memcpy(ptr, &int_value, _SHORT);
			ptr += _SHORT;
			memcpy(ptr, msg->content.key, strlen(msg->content.key));
			break;
	}
}

Message *buffer_to_message(char *msg_buf, int msg_size){

  /* Verificar se msg_buf é NULL */
	if (msg_buf == NULL)
		return  NULL;
  /* msg_size tem tamanho mínimo ? */
	if (msg_size < 2 * _SHORT)
		return NULL;

  /* Alocar memória para uma struct message_t */
	Message *msg = malloc(sizeof(Message));
	if (msg == NULL)
		return NULL;
  /* Recuperar o opcode e c_type */
  msg->opcode = ntohs(*(short *) msg_buf++);
  msg->c_type = ntohs(*(short *) ++msg_buf);
  msg_buf += _SHORT;

  /* O opcode e c_type são válidos? */
  int op = msg->opcode;
	if (op % 10 > 1 || op > 101 || op < 0 || msg->c_type % 10 != 0 || msg->c_type > 50 || msg->c_type < 0) {
		free(msg);
		return NULL;
	}
  /* Consoante o c_type, continuar a recuperação da mensagem original */
	int result = recover_message(msg_buf, msg);
	if (result == -1) {
		free(msg);
		return NULL;
	}
  return msg;
}

int recover_message(char *msg_buf, Message *msg) {
	if (msg_buf == NULL || msg == NULL)
		return -1;
	Data *data;
	short key_size, short_value;
	char *key, **keys;
	int n_keys, i;
	Entry *entry;
	
	switch(msg->c_type) {
		case CT_RESULT:
			memcpy(&i, msg_buf, _INT);
			msg->content.result = ntohl(i);
			break;
		case CT_VALUE:
			memcpy(&i, msg_buf, _INT);
			msg_buf += _INT;
			i = ntohl(i);
			if (i == 0) {
				data = malloc(sizeof(Data));
				if (data == NULL)
					return -1;
				data->datasize = 0;
				data->data = NULL;
			}
			else {
				data = data_create(i);
				if (data == NULL)
					return -1;
				
				memcpy(data->data, msg_buf, data->datasize);
			}
			msg->content.data = data;
			break;
		case CT_ENTRY:
			memcpy(&short_value, msg_buf, _SHORT);
			msg_buf += _SHORT;
			key_size = ntohs(short_value);
			
			key = malloc(key_size + 1);
			if (key == NULL)
				return -1;
			memcpy(key, msg_buf, key_size);
			msg_buf += key_size;
			key[key_size] = '\0';

			memcpy(&i, msg_buf, _INT);
			msg_buf += _INT;
			data = data_create(ntohl(i));
			if (data == NULL) {
				free(key);
				return -1;
			}
			memcpy(data->data, msg_buf, data->datasize);
			
			entry = entry_create(key, data);
			if (entry == NULL) {
				free(key);
				data_destroy(data);
				return -1;
			}
			free(key);
			data_destroy(data);
			msg->content.entry = entry;
			break;
		case CT_KEYS:
			memcpy(&i, msg_buf, _INT);
			msg_buf += _INT;
			n_keys = ntohl(i);
			keys = malloc(n_keys * sizeof(char *) + sizeof(NULL));
			if (keys == NULL)
				return -1;
			i = 0;
			while (n_keys-- > 0) {
				memcpy(&short_value, msg_buf, _SHORT);
				msg_buf += _SHORT;
				key_size = ntohs(short_value);
				key = malloc(key_size + 1);
				if (key == NULL)
					break;
				memcpy(key, msg_buf, key_size);
				msg_buf += key_size;
				key[key_size] = '\0';
				keys[i] = key;
				i++;
			}
			//nao foram lidas todas as keys
			if (n_keys != -1) {
				while(--i >= 0)
					free(keys[i]);
				free(keys);
				return -1;
			}
			keys[i] = NULL;
			msg->content.keys = keys;
			break;
		case CT_KEY:
			memcpy(&short_value, msg_buf, _SHORT);
			msg_buf += _SHORT;
			key_size = ntohs(short_value);
			key = malloc(key_size + 1);
			if (key == NULL)
				return -1;
			memcpy(key, msg_buf, key_size);
			key[key_size] = '\0';
			msg->content.key = key;
			break;
	}
	return 0;
}
