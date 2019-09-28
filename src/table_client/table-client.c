/*
* Grupo 13
* Francisco de Almeida Araujo, 45701
* Nuno Burnay, 46406
* Robin Vassantlal, 46408
*/

#include <signal.h>

#include "client_stub-private.h"

RemoteTable *rtable;

char *get_key(int oplen, char *input);
Data *get_data(int offset, char *input);
int count_words(char *in);
int get_size(int offset, char *in);
int is_operation(char *input, char *op);
void show_result(int result);
void show_data(Data *data);
void show_keys(char **keys);
void catch_signal(int signal);
void test_input(int argc);
void process_input(char *input);

int main(int argc, char **argv){
	char input[81];
	rtable = NULL;

	/* Testar os argumentos de entrada */
	test_input(argc);
	
	rtable = rtable_bind(argv[1]);
	if (rtable == NULL) {
		printf("ERRO: Não foi possível conectar.\n");
		return -1;
	}

	set_backup_server(argv[2]);

	signal(SIGINT, catch_signal);
	signal(SIGPIPE, catch_signal);
	/* Fazer ciclo até que o utilizador resolva fazer "quit" */
 	while (true){
		printf(">>> "); // Mostrar a prompt para inserção de comando
		
		if (fgets(input, 81, stdin) == NULL)
			break;
		input[strlen(input) - 1] = '\0';
		/* Verificar se o comando foi "quit". Em caso afirmativo
		   não há mais nada a fazer a não ser terminar decentemente.
		 */
		 if (is_operation(input, "quit"))
		 	break;
		process_input(input);
		
	}
	int i = rtable_unbind(rtable);
	close_backup_server();
	if (i == 0)
		printf("Já não está conectado com servidor!\n");
  	return i;
}

void catch_signal (int signal) {
	if (signal == SIGPIPE)
		return;
	rtable_unbind(rtable);
	exit(signal);
}

void test_input(int argc) {
	if (argc != 3){
		printf("Uso: ./client <IP_primario:Porto_primario> <IP_secundario:Porto_secundario>\n");
		printf("Exemplo de uso: ./client 98.94.35.0:50000 98.12.56.65:50001\n");
		exit(1);
	}
}

char *get_key(int oplen, char *input) {
	int offset = oplen + 1;
	int size;
	char *key;
	size = get_size(offset, input);
	key = malloc(sizeof(char *) * size + 1);
	if (key == NULL) {
		printf("ERRO: Não foi possível guardar key!\n");
		return NULL;
	}
	memcpy(key, input + offset, size);
	key[size] = '\0';
	return key;
}

Data *get_data(int off, char *input) {
	int offset = off + 1;
	int size;
	void *d;
	size = get_size(offset, input);
	d = malloc(sizeof(void *) * size);
	if (d == NULL) {
		printf("ERRO: Não foi possível guardar dado!\n");
		return NULL;
	}
	memcpy(d, input + offset, size);
	Data *data = data_create2(size, d);
	free(d);
	if (data == NULL) {
		printf("ERRO: Não foi possível guardar dado!\n");
		return NULL;
	}
	return data;
}

int count_words(char *in) {
	int i, count = 1;
	for (i = 1; i < strlen(in); i++)
		count += in[i] == ' ' && in[i + 1] != ' ' && in[i + 1] != '\0';
	return count;
}

int get_size(int offset, char *in) {
	int i = offset;
	while(offset < strlen(in) && in[offset] == ' ')
		offset++;
	while (offset < strlen(in) && in[offset] != ' ' && in[offset] != '\0')
		offset++;
	return (offset - i);
}

int is_operation(char *input, char *op) {
	int i = 0;
	for (i = 0; i < strlen(op); i++)
		if (input[i] != op[i])
			return 0;
	return input[i] == '\0' || input[i] == ' ';
}

void show_result(int result) {
	if (result < 0)
		printf("Erro na efetuação da operação.\n");
	else
		printf("Operação teve sucesso. Result = %d\n", result);
	printf("\n");
}

void show_data(Data *data) {
	if (data == NULL)
		printf("Erro na efetuação da operação.\n");
	else
		printf("datasize: %d\n", data->datasize);
	printf("\n");
}

void show_keys(char **keys) {
	if (keys == NULL)
		printf("Erro na afetuação da operação.\n");
	else {
		int i;
		for(i = 0; keys[i] != NULL; i++) {
			printf("key[%d]: %s\n", i, keys[i]);
		}
	}
	printf("\n");
}

void process_input(char *input) {
	char *key = NULL;
	Data *data = NULL;

	int words = count_words(input);

	if (is_operation(input, "put") && words == 3) {
		key = get_key(3, input);
		data = get_data(4 + strlen(key), input);
		show_result(rtable_put(rtable, key, data));
		free(key);
		data_destroy(data);
	}
	else if (is_operation(input, "get") && words == 2) {
		key = get_key(3, input);
		if (key == NULL) {
			printf("Erro na efetuação da operação.\n");
			return;
		}
		if (key[0] != '*') {
			data = rtable_get(rtable, key);
			show_data(data);
			data_destroy(data);
		}
		else {
			char **keys = rtable_get_keys(rtable);
			show_keys(keys);
			rtable_free_keys(keys);
		}
		free(key);
	}
	else if (is_operation(input, "del") && words == 2) {
		key = get_key(3, input);
		show_result(rtable_del(rtable, key));
		free(key);
	}
	else if (is_operation(input, "update") && words == 3) {
		key = get_key(6, input);
		data = get_data(7 + strlen(key), input);
		show_result(rtable_update(rtable, key, data));
		free(key);
		data_destroy(data);
	}
	else if (is_operation(input, "size") && words == 1) {
		show_result(rtable_size(rtable));
	}
	else {
		printf("ERRO: Comando não é válido!\n");
	}
}
