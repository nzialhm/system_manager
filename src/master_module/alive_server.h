//alive_servcer.h
#ifndef __ALIVE_SERVER_H
#define __ALIVE_SERVER_H

#include <arpa/inet.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>

#include <libubox/uloop.h>
#include <libubox/list.h>


#define MASTER_ALIVECHECK_TIME 500

struct device_info {
    char serial[64];
    char model[64];
    char cert_id[64];
    char type[64];

    char ip[32];

    double lat;
    double lon;

    char height_type[8];
    float height;

    int slave_key;

    time_t last_seen;

    int online;   // 1: online, 0: offline

    struct list_head list;
};

/* ------------------------------
 * 
 * ------------------------------ */
extern int image_server_init(void);
extern int ubus_init(void);


/* ------------------------------
 * 전역
 * ------------------------------ */
static struct uloop_fd sock_fd;
static struct uloop_timeout cleanup_timer;

static LIST_HEAD(device_list);

/* 외부에서 호출 */
int alive_server_start(void);

/* device 업데이트 (외부에서 쓸 수도 있어서 export) */
// void update_device(struct sockaddr_in *cli, char *msg);

void cleanup_devices_cb(struct uloop_timeout *t);

#endif //__ALIVE_SERVER_H
