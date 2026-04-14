#include "../std.h"
#include "uci_cmd.h"

#define CMD_BUF 256

int uci_get_value(const char *package, const char *section, const char *option, char *out, int out_len)
{
    char cmd[CMD_BUF];
    snprintf(cmd, sizeof(cmd), "uci get %s.%s.%s 2>/dev/null", package, section, option);

    FILE *fp = popen(cmd, "r");
    if (!fp)
        return -1;

    if (fgets(out, out_len, fp) == NULL) {
        pclose(fp);
        return -1;
    }

    // newline 제거
    out[strcspn(out, "\n")] = 0;

    pclose(fp);
    return 0;
}

int uci_set_value(const char *package, const char *section, const char *option, const char *value)
{
    char cmd[CMD_BUF];
    snprintf(cmd, sizeof(cmd), "uci set %s.%s.%s='%s'", package, section, option, value);

    return system(cmd);
}

int uci_commit(const char *package)
{
    char cmd[CMD_BUF];
    snprintf(cmd, sizeof(cmd), "uci commit %s", package);

    return system(cmd);
}
