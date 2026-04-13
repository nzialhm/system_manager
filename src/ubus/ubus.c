// ubus.c
#include "../std.h"
#include "ubus.h"

struct ubus_context *ubus_get_ctx(void)
{
    return ctx;
}

static int get_status(struct ubus_context *ctx,
                      struct ubus_object *obj,
                      struct ubus_request_data *req,
                      const char *method,
                      struct blob_attr *msg)
{
    struct blob_buf b = {};
    blob_buf_init(&b, 0);

    blobmsg_add_string(&b, "status", "running");

    ubus_send_reply(ctx, req, b.head);
    blob_buf_free(&b);

    return 0;
}

static const struct ubus_method methods[] = {
    UBUS_METHOD_NOARG("status", get_status),
};

static struct ubus_object_type obj_type =
    UBUS_OBJECT_TYPE("system_manager", methods);

static struct ubus_object obj = {
    .name = "system_manager",
    .type = &obj_type,
    .methods = methods,
    .n_methods = 1,
};

void ubus_init(void)
{
    ctx = ubus_connect(NULL);
    ubus_add_object(ctx, &obj);
}
