// ubus.c
#include "../std.h"
#include "ubus.h"
#include "../master_module/alive_server.h"   // device_info

static struct ubus_context *ctx;

extern char g_mode[32];
extern char g_model[32];

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
        blobmsg_add_string(&b, "mode", g_mode);
        blobmsg_add_string(&b, "model", g_model);
    }
    else if (!strcmp(method, "config")) {
        blobmsg_add_string(&b, "cmd", cmd);
        blobmsg_add_string(&b, "value", value);
        blobmsg_add_string(&b, "result", "config updated");
    }
    else {
        blobmsg_add_string(&b, "error", "unknown method");
    }

    ubus_send_reply(ctx, req, b.head);
    blob_buf_free(&b);

    return 0;
}

static int get_devices(struct ubus_context *ctx,
                       struct ubus_object *obj,
                       struct ubus_request_data *req,
                       const char *method,
                       struct blob_attr *msg)
{
    struct blob_buf b = {};
    blob_buf_init(&b, 0);

    void *arr = blobmsg_open_array(&b, "devices");

    struct device_info *d;

    list_for_each_entry(d, &device_list, list) {
        void *obj = blobmsg_open_table(&b, NULL);

        char buf[64];  // 숫자 → 문자열 변환용

        blobmsg_add_string(&b, "serial", d->serial);
        blobmsg_add_string(&b, "model", d->model_id);
        blobmsg_add_string(&b, "cert_id", d->cert_id);
        blobmsg_add_string(&b, "type", d->type);
        blobmsg_add_string(&b, "ip", d->ip);

        // power (int → string)
        snprintf(buf, sizeof(buf), "%u", d->power);
        blobmsg_add_string(&b, "power", buf);

        // lat (double → string)
        snprintf(buf, sizeof(buf), "%.6f", d->lat);
        blobmsg_add_string(&b, "lat", buf);

        // lon (double → string)
        snprintf(buf, sizeof(buf), "%.6f", d->lon);
        blobmsg_add_string(&b, "lon", buf);

        blobmsg_add_string(&b, "height_type", d->height_type);

        // height (double → string)
        snprintf(buf, sizeof(buf), "%.2f", d->height);
        blobmsg_add_string(&b, "height", buf);

        blobmsg_close_table(&b, obj);
    }

    blobmsg_close_array(&b, arr);

    ubus_send_reply(ctx, req, b.head);
    blob_buf_free(&b);

    return 0;
}

static int get_deviceslist(struct ubus_context *ctx,
                       struct ubus_object *obj,
                       struct ubus_request_data *req,
                       const char *method,
                       struct blob_attr *msg)
{
    struct blob_buf b = {};
    blob_buf_init(&b, 0);

    void *arr = blobmsg_open_array(&b, "deviceslist");

    struct device_info *d;

    list_for_each_entry(d, &device_list, list) {
        void *obj = blobmsg_open_table(&b, NULL);

        blobmsg_add_string(&b, "serial", d->serial);
        blobmsg_add_u32(&b, "online", d->online);

        blobmsg_close_table(&b, obj);
    }

    blobmsg_close_array(&b, arr);

    ubus_send_reply(ctx, req, b.head);
    blob_buf_free(&b);

    return 0;
}

/* =========================
 * 메서드 등록 (6개)
 * ========================= */
static const struct ubus_method methods[] = {
    UBUS_METHOD_NOARG("status",  handle_request),
    UBUS_METHOD("config", handle_request, policy),
    UBUS_METHOD_NOARG("devices", get_devices),
    UBUS_METHOD_NOARG("deviceslist", get_deviceslist),
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

    ubus_add_uloop(ctx);   //추가해야함
    ubus_add_object(ctx, &obj);
}
