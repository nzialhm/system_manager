// alive_client.h

#ifndef __ALIVE_CLIENT_H
#define __ALIVE_CLIENT_H

#include <arpa/inet.h>
#include <unistd.h>
#include <libubox/uloop.h>

#define SLAVE_ALIVECHECK_TIME 1000

static struct uloop_timeout alive_timer;

void send_alive(void);
void make_alivesocket(void);
void close_alivesocket(void);
void alive_cb(struct uloop_timeout *t);

#endif //__ALIVE_CLIENT_H
