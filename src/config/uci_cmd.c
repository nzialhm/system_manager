#include "../std.h"
#include "uci_cmd.h"
#include <uci.h>

/* =========================
 * GET
 * ========================= */
int uci_get_value(const char *package, const char *section, const char *option,
                  char *out, int out_len)
{
    if (!package || !section || !option || !out || out_len <= 0)
        return -1;

    struct uci_context *ctx = uci_alloc_context();
    if (!ctx)
        return -1;

    char path[128];
    snprintf(path, sizeof(path), "%s.%s.%s", package, section, option);

    struct uci_ptr ptr;
    if (uci_lookup_ptr(ctx, &ptr, path, true) != UCI_OK) {
        uci_free_context(ctx);
        return -1;
    }

    if (!ptr.o || ptr.o->type != UCI_TYPE_STRING) {
        uci_free_context(ctx);
        return -1;
    }

    snprintf(out, out_len, "%s", ptr.o->v.string);

    uci_free_context(ctx);
    return 0;
}

/* =========================
 * SET
 * ========================= */
int uci_set_value(const char *package, const char *section,
                  const char *option, const char *value)
{
    if (!package || !section || !option || !value)
        return -1;

    struct uci_context *ctx = uci_alloc_context();
    if (!ctx)
        return -1;

    char path[256];
    snprintf(path, sizeof(path), "%s.%s.%s=%s",
             package, section, option, value);

    struct uci_ptr ptr;
    if (uci_lookup_ptr(ctx, &ptr, path, true) != UCI_OK) {
        uci_free_context(ctx);
        return -1;
    }

    if (uci_set(ctx, &ptr) != UCI_OK) {
        uci_free_context(ctx);
        return -1;
    }

    uci_free_context(ctx);
    return 0;
}

/* =========================
 * COMMIT
 * ========================= */
int uci_commit_package(const char *package)
{
    if (!package)
        return -1;

    struct uci_context *ctx = uci_alloc_context();
    if (!ctx)
        return -1;

    struct uci_package *pkg = NULL;

    if (uci_load(ctx, package, &pkg) != UCI_OK) {
        uci_free_context(ctx);
        return -1;
    }

    if (uci_commit(ctx, &pkg, false) != UCI_OK) {
        uci_unload(ctx, pkg);
        uci_free_context(ctx);
        return -1;
    }

    uci_unload(ctx, pkg);
    uci_free_context(ctx);
    return 0;
}
