#ifndef _PRIMARY_BACKUP_H
#define _PRIMARY_BACKUP_H


struct server_t;

int hello (struct server_t *server);

int update_state(struct server_t *server);

#endif
