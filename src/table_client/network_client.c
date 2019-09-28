/*
* Grupo 13
* Francisco de Almeida Araujo, 45701
* Nuno Burnay, 46406
* Robin Vassantlal, 46408
*/
#include "network_client-private.h"

#include <stdlib.h>

Server *server_bk = NULL;

int write_all(int sock, char *buf, int len){
	int bufsize = len;
	while(len > 0) {
		int res;
		if (len > MAX_MSG)
			res = write(sock, buf, MAX_MSG);
		else
			res = write(sock, buf, len);
		if(res < 0) {
			if(errno == EINTR) 
				continue;
			perror("write failed:");
			return res;
		}
		buf += res;
		len -= res;
	}
	return bufsize;
}

int read_all(int sock, char *buf, int len){
	int bufsize = len;
	while(len>0) {
		int res = read(sock, buf, len);
		if (res == 0)
			return bufsize - len;
		if(res<0) {
			if(errno==EINTR) 
				continue;
			perror("read failed:");
			return res;
		}
		buf += res;
		len -= res;
	}
	return bufsize;
}

Server *network_connect(const char *address_port){
	Server *server = malloc(sizeof(Server));

	/* Verificar parâmetro da função e alocação de memória */
	if (server == NULL)
		return NULL;
	if (address_port == NULL) {
		free(server);
		return NULL;
	}
	int separator_i = strchr(address_port, ':') - address_port;
	if (separator_i < 0) {
		free(server);
		return NULL;
	}
	int port_size = strlen(address_port) - separator_i - 1;
	char *port = malloc(sizeof(char *) * port_size + 1);
	if (port == NULL) {
		free(server);
		return NULL;
	}
	memcpy(port, address_port + separator_i + 1, port_size);
	port[port_size] = '\0';
	struct sockaddr_in server_soc;
	server_soc.sin_family = AF_INET;
	server_soc.sin_port = htons(atoi(port));

	free(port);
	char *ip = malloc(sizeof(char *) * separator_i + 1);
	if (ip == NULL) {
		free(server);
		return NULL;
	}
	memcpy(ip, address_port, separator_i);
	ip[separator_i] = '\0';
	if (inet_pton(AF_INET, ip, &server_soc.sin_addr) < 1) {
		free(server);
		free(ip);
		return NULL;
	}
	free(ip);
	int sockfd;
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		free(server);
		return NULL;
	}
	
	/* Se a ligação não foi estabelecida, retornar NULL */
	if (connect(sockfd, (struct sockaddr *)&server_soc, sizeof(server_soc)) < 0) {
		free(server);
		close(sockfd);
		return NULL;
	}
	int len = strlen(address_port);
	char *cp = malloc(len + 1);
	memcpy(cp, address_port, len);
	cp[len] = '\0';
	server->sockfd = sockfd;
	server->addr_port = cp;
	server->isConnected = 1;
	return server;
}

void set_backup_server(const char *address_port) {
	if (address_port == NULL)
		return;
	server_bk = malloc(sizeof(Server));
	if (server_bk == NULL)
		return;
	int len = strlen(address_port);
	char *cp = malloc(len + 1);
	memcpy(cp, address_port, len);
	cp[len] = '\0';
	server_bk->addr_port = cp;
	server_bk->isConnected = 0;
}

Message *network_send_receive(Server *server, Message *msg){
	if (server == NULL || msg == NULL)
		return NULL;
	Message *result;

	result = send_once(server, msg);
	if (result == NULL) {
		int temp = RETRY_TIME;
		while((temp = sleep(temp)) != 0);
		char *ad = server->addr_port;
		free(server);
		server = network_connect(ad);
		free(ad);
		result = send_once(server, msg);
	}

	return result;
}

Message *send_once(Server *server, Message *msg) {
	if (server == NULL || msg == NULL)
		return NULL;
	Message *result = NULL;
	if (server->isConnected)
		result = send_receive(server, msg);
	if (result == NULL) {
		if (server_bk->isConnected == 0) {
			char *ad = server_bk->addr_port;
			free(server_bk);
			server_bk = network_connect(ad);
			free(ad);
		}
		result = send_receive(server_bk, msg);
	}
	return result;
}

Message *send_receive(Server *server, Message *msg) {
	char *message_out;
	int message_size, msg_size, result;
	struct message_t *msg_resposta;

	/* Verificar parâmetros de entrada */
	if (server == NULL || msg == NULL)
		return NULL;
	/* Serializar a mensagem recebida */
	message_size = message_to_buffer(msg, &message_out);

	/* Verificar se a serialização teve sucesso */
	if (message_size == -1) {
		free(message_out);
		return NULL;
	}
	/* Enviar ao servidor o tamanho da mensagem que será enviada
	   logo de seguida
	*/
	msg_size = htonl(message_size);
 	result = write_all(server->sockfd, (char *) &msg_size, _INT);

	/* Verificar se o envio teve sucesso */
	
	if (result != _INT) {
		server->isConnected = 0;
		printf("%d %d\n", result, _INT);
		free(message_out);
		return  NULL;
	}
	/* Enviar a mensagem que foi previamente serializada */

	result = write_all(server->sockfd, message_out, message_size);

	/* Verificar se o envio teve sucesso */
	if (result != message_size) {
		server->isConnected = 0;
		free(message_out);
		return NULL;
	}
	free(message_out);
	/* De seguida vamos receber a resposta do servidor:
		Com a função read_all, receber num inteiro o tamanho da 
		mensagem de resposta.
		Alocar memória para receber o número de bytes da
		mensagem de resposta.
		Com a função read_all, receber a mensagem de resposta.
		
	*/
	result = read_all(server->sockfd, (char *) &msg_size, _INT);
	if (result != _INT) {
		server->isConnected = 0;
		return NULL;
	}
	message_size = ntohl(msg_size);
	message_out = malloc(sizeof(char *) * message_size);
	if (message_out == NULL)
		return NULL;
	result = read_all(server->sockfd, message_out, message_size);
	if (result != message_size) {
		server->isConnected = 0;
		free(message_out);
		return NULL;
	}
	/* Desserializar a mensagem de resposta */
	msg_resposta = buffer_to_message( message_out, message_size);

	/* Verificar se a desserialização teve sucesso */
	if (msg_resposta == NULL) {
		free(message_out);
		return NULL;
	}
	/* Libertar memória */
	free(message_out);
	return msg_resposta;
}

void close_backup_server() {
	if (server_bk == NULL)
		return;
	network_close(server_bk);
}

int network_close(Server *server){
	/* Verificar parâmetros de entrada */
	if (server == NULL)
		return -1;
	/* Terminar ligação ao servidor */
	if (server->isConnected)
		close(server->sockfd);
	/* Libertar memória */
	free(server->addr_port);
	free(server);
	return 0;
}