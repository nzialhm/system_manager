// config_reader.c
#include "../std.h"
#include "../common/common.h"
#include <unistd.h>
#include "config_reader.h"

int read_all_config(const char *device, struct devconfig *cfg)
{
    FILE *f = fopen(device, "rb");
    if (!f) {
        perror("open device");
        return -1;
    }

    memset(cfg, 0, sizeof(*cfg));

    char buf[256] = {0};
    int idx = 0;
    int zero_cnt = 0;

    while (1) {
        int c = fgetc(f);
        if (c == EOF)
            break;

        if (c < ' ') {
            zero_cnt++;

            if (idx > 0) {
                buf[idx] = '\0';

                char *eq = strchr(buf, '=');
                if (eq && cfg->count < MAX_CONFIG) {
                    *eq = '\0';

                    strncpy(cfg->items[cfg->count].key, buf, MAX_KEY_LEN - 1);
                    strncpy(cfg->items[cfg->count].val, eq + 1, MAX_VAL_LEN - 1);

                    cfg->count++;
                }

                idx = 0;
                memset(buf, 0, sizeof(buf));
            }

            if (zero_cnt > 1)
                break;

        } else {
            zero_cnt = 0;

            if (idx < sizeof(buf) - 1)
                buf[idx++] = (char)c;
        }
    }

    fclose(f);
    return 0;
}

const char* config_get(struct devconfig *cfg, const char *key)
{
    for (int i = 0; i < cfg->count; i++) {
        if (strcmp(cfg->items[i].key, key) == 0)
            return cfg->items[i].val;
    }
    return NULL;
}
