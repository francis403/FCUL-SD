/*
* Grupo 13
* Francisco de Almeida Araujo, 45701
* Nuno Burnay, 46406
* Robin Vassantlal, 46408
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "table-private.h"

int key_hash(char *key, int l){

  /* Verificar se key é NULL */
	if (key == NULL)
		return -1;
  /* l tem valor válido? */
	if (l <= 0)
		return 0;
	int soma = 0;
	int i;
	int len = strlen(key);
	if (len <= 5)
		for(i = 0; i < len; i++)
			soma += key[i];
	else
		soma = key[0] + key[1] + key[2] + key[len - 2] 
				+ key[len - 1];
  return soma % l;
}

Table *table_create(int n) {

  /* n tem valor válido? */
	if (n <= 0)
		return NULL;
  /* Alocar memória para struct table_t */
	Table *new_table = malloc(sizeof(Table));
	if (new_table == NULL)
		return NULL;
  /* Alocar memória para array de listas com n entradas 
     que ficará referenciado na struct table_t alocada. 
     
     Inicializar listas.

     Inicializar atributos da tabela.
  */
	new_table->table = malloc(sizeof(List*) * n);
	if (new_table->table == NULL) {
		free(new_table);
		return NULL;
	}
	int i;
	for (i = 0; i < n; i++)
		new_table->table[i] = NULL;
	new_table->size = n;
	new_table->num_key_values = 0;
	return new_table;
}

void table_destroy(Table *table) {

  /* table é NULL? 

     Libertar memória das listas.

     Libertar memória da tabela.

  */
	if (table == NULL)
  		return;
  	int i;
  	for (i = 0; i < table->size; i++)
  		if (table->table[i] != NULL)
  			list_destroy(table->table[i]);
  	free(table->table);
  	free(table);
}

int table_put(Table *table, char * key, struct data_t *value) {

  /* Verificar valores de entrada */
	if (table == NULL || key == NULL || value == NULL)
		return -1;
  /* Criar entry com par chave/valor */
	Entry *new_entry = malloc(sizeof(Entry));
	if (new_entry == NULL)
		return -1;
	new_entry->key = key;
	new_entry->value = value;
  /* Executar hash para determinar onde inserir a entry na tabela */
	int h_code = key_hash(key, table->size);
	if (h_code == -1) {
		free(new_entry);
		return -1;
	}
  /* Inserir entry na tabela */
	if (table->table[h_code]  == NULL) {
		table->table[h_code] = list_create();
		if (table->table[h_code] == NULL) {
			free(new_entry);
			return -1;
		}
	}
	if (list_get(table->table[h_code], new_entry->key) != NULL) {
		free(new_entry);
		return -1;
	}
	if (list_add(table->table[h_code], new_entry) == -1) {
		free(new_entry);
		return -1;
	}
	free(new_entry);
	table->num_key_values = table->num_key_values + 1;
	return 0;
}

int table_update(Table *table, char * key, struct data_t *value) {
	if (table == NULL || key == NULL || value == NULL)
		return -1;
	
	Entry *new_entry = malloc(sizeof(Entry));
	if (new_entry == NULL)
		return -1;
	new_entry->key = key;
	new_entry->value = value;
	
	int h_code = key_hash(key, table->size);
	if (h_code == -1) {
		free(new_entry);
		return -1;
	}
	
	if (list_get(table->table[h_code], new_entry->key) == NULL) {
		free(new_entry);
		return -1;
	}
	int result = list_add(table->table[h_code], new_entry);
	free(new_entry);
	return result;
}

struct data_t *table_get(Table *table, char * key){
	if (table == NULL || key == NULL)
		return NULL;
	int h_code = key_hash(key, table->size);
	if (h_code == -1)
		return NULL;
	Entry *entry = list_get(table->table[h_code], key);
	if (entry == NULL)
		return NULL;
	return data_dup(entry->value);
}

int table_del(Table *table, char *key){
	if (table == NULL || key == NULL)
		return -1;
	int h_code = key_hash(key, table->size);
	if (h_code == -1)
		return -1;
	int r = list_remove(table->table[h_code], key);
	if (r == -1)
		return -1;
	table->num_key_values = table->num_key_values - 1;
	return 0;
}

/* Esta é dada! Ao estilo C! */
int table_size(Table *table) {
	return table == NULL ? -1 : table->num_key_values;
}

char **table_get_keys(Table *table) {
	if (table == NULL)
		return NULL;
	char **vetor = malloc(table->num_key_values * sizeof(char*) 
			+ sizeof(NULL));
	if (vetor == NULL)
		return NULL;
	int i, j, count;
	count = 0;
	bool b = true;
	for (i = 0; i < table->size && b; i++) {
		if (table->table[i] != NULL) {
			char **temp = list_get_keys(table->table[i]);
			b = temp != NULL;
			if (b) {
				j = -1;
				while (temp[++j] != NULL)
					vetor[count++] = temp[j];
				free(temp);
			}
		}
	}
	if (!b) {
		while (--count >= 0)
			free(vetor[count]);
		free(vetor);
		return NULL;
	}
	vetor[count] = NULL;
	
	return vetor;
}

void table_free_keys(char **keys) {
	if (keys == NULL)
		return;
	int i = 0;
	while(keys[i] != NULL)
		free(keys[i++]);
	free(keys[i]);
	free(keys);
}

