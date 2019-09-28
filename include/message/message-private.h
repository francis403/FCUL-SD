/*
* Grupo 13
* Francisco de Almeida Araujo, 45701
* Nuno Burnay, 46406
* Robin Vassantlal, 46408
*/

#ifndef _MESSAGE_PRIVATE_H
#define _MESSAGE_PRIVATE_H

#define _SHORT 2
#define _INT 4

#define OC_RT_ERROR 0
#define OC_HELLO 100
#define OC_UPDATE_STATE 60

#include "table-private.h" /* For table_free_keys() */
#include "message.h"
#include "data-private.h"

typedef struct message_t Message;

/*
 * Returns size in bytes according to type of message.
 * 0 if msg == NULL
*/
int get_byte_quantity(Message *msg);

/*
 * Copy contents of message (msg) to ptr;
*/
void copy_content(Message *msg, char *ptr);

/*
* Recupera a mensagem, passa os bytes do union para o union 
* que esta na  struct message_t
* Retornar 0 (OK) ou -1
*/
int recover_message(char *msg_buf, Message *msg);

#endif
