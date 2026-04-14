// ubus.c
#include "../std.h"
#include "ubus.h"

static struct ubus_context *ctx;

struct ubus_context *ubus_get_ctx(void)
{
    return ctx;
}

/* =========================
 * 정책 정의 (파라미터 파싱용)
 * ========================= */
enum {
    ARG_CMD,
    ARG_VALUE,
    __ARG_MAX,
};

static const struct blobmsg_policy policy[__ARG_MAX] = {
    [ARG_CMD]   = { .name = "cmd",   .type = BLOBMSG_TYPE_STRING },
    [ARG_VALUE] = { .name = "value", .type = BLOBMSG_TYPE_STRING },
};

/* =========================
 * 공통 처리 함수
 * ========================= */
static int handle_request(struct ubus_context *ctx,
                          struct ubus_object *obj,
                          struct ubus_request_data *req,
                          const char *method,
                          struct blob_attr *msg)
{
    struct blob_attr *tb[__ARG_MAX];
    struct blob_buf b = {};

    blobmsg_parse(policy, __ARG_MAX, tb, blob_data(msg), blob_len(msg));

    const char *cmd   = tb[ARG_CMD]   ? blobmsg_get_string(tb[ARG_CMD])   : "";
    const char *value = tb[ARG_VALUE] ? blobmsg_get_string(tb[ARG_VALUE]) : "";

    blob_buf_init(&b, 0);

    /* =========================
     * method 기준 분기
     * ========================= */
    if (!strcmp(method, "status")) {
        blobmsg_add_string(&b, "status", "running");
    }
    else if (!strcmp(method, "start")) {
        blobmsg_add_string(&b, "result", "started");
    }
    else if (!strcmp(method, "stop")) {
        blobmsg_add_string(&b, "result", "stopped");
    }
    else if (!strcmp(method, "restart")) {
        blobmsg_add_string(&b, "result", "restarted");
    }
    else if (!strcmp(method, "config")) {
        blobmsg_add_string(&b, "cmd", cmd);
        blobmsg_add_string(&b, "value", value);
        blobmsg_add_string(&b, "result", "config updated");
    }
    else if (!strcmp(method, "info")) {
        blobmsg_add_string(&b, "version", "1.0.0");
        blobmsg_add_string(&b, "name", "system_manager");
    }
    else {
        blobmsg_add_string(&b, "error", "unknown method");
    }

    ubus_send_reply(ctx, req, b.head);
    blob_buf_free(&b);

    return 0;
}

/* =========================
 * 메서드 등록 (6개)
 * ========================= */
static const struct ubus_method methods[] = {
    UBUS_METHOD_NOARG("status",  handle_request),
    UBUS_METHOD_NOARG("start",   handle_request),
    UBUS_METHOD_NOARG("stop",    handle_request),
    UBUS_METHOD_NOARG("restart", handle_request),
    UBUS_METHOD("config", policy, handle_request),
    UBUS_METHOD_NOARG("info",    handle_request),
};

static struct ubus_object_type obj_type =
    UBUS_OBJECT_TYPE("system_manager", methods);

static struct ubus_object obj = {
    .name = "system_manager",
    .type = &obj_type,
    .methods = methods,
    .n_methods = ARRAY_SIZE(methods),
};

/* =========================
 * 초기화
 * ========================= */
void ubus_init(void)
{
    ctx = ubus_connect(NULL);
    if (!ctx) {
        fprintf(stderr, "ubus connect failed\n");
        return;
    }

    ubus_add_object(ctx, &obj);
}
