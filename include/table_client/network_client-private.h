/*
* Grupo 13
* Francisco de Almeida Araujo, 45701
* Nuno Burnay, 46406
* Robin Vassantlal, 46408
*/

#ifndef _NETWORK_CLIENT_PRIVATE_H
#define _NETWORK_CLIENT_PRIVATE_H

#include "inet.h"
#include "network_client.h"

#include <errno.h>
#include <unistd.h>

#define RETRY_TIME 1 //em segundos

struct server_t{
	/* Atributos importantes para interagir com o servidor, */
	/* tanto antes da ligação estabelecida, como depois.    */
	int sockfd;
	char *addr_port;
	int isConnected;
};

typedef struct server_t Server;

/* Função que garante o envio de len bytes armazenados em buf,
   através da socket sock.
*/
int write_all(int sock, char *buf, int len);

/* Função que garante a receção de len bytes através da socket sock,
   armazenando-os em buf.
*/
int read_all(int sock, char *buf, int len);

void set_backup_server(const char *address_port);

Message *send_once(Server *server, Message *msg);

Message *send_receive(Server *server, Message *msg);

void close_backup_server();

#endif
