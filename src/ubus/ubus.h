// ubus.h

#ifndef __UBUS_H
#define __UBUS_H
#include <libubus.h>
#include <libubox/blobmsg_json.h>

static struct ubus_context *ctx;
extern void ubus_init(void);

#endif //__UBUS_H
