/*
* Grupo 13
* Francisco de Almeida Araujo, 45701
* Nuno Burnay, 46406
* Robin Vassantlal, 46408
*/
#ifndef _CLIENT_STUB_PRIVATE_H
#define _CLIENT_STUB_PRIVATE_H

#include "data-private.h"
#include "client_stub.h"
#include "network_client-private.h"
#include "message-private.h"

struct rtable_t {
	Server *server;
};

typedef struct rtable_t RemoteTable;

char **copy_keys(char **keys);

#endif