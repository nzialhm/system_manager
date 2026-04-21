// apip.c
#include "std.h"
#include "../common/common.h"
#include "apip.h"

/**
 * /proc/net/route 파싱해서 gateway 가져오기
 */
int get_gateway_ip(const char *ifname, char *ipbuf, int buflen)
{
    FILE *fp;
    char line[256];

    if (!ifname || !ipbuf)
        return -1;

    fp = fopen("/proc/net/route", "r");
    if (!fp)
        return -1;

    while (fgets(line, sizeof(line), fp)) {
        char iface[32];
        unsigned long dest, gateway;

        if (sscanf(line, "%31s %lx %lx", iface, &dest, &gateway) != 3)
            continue;

        // default route (Destination == 0)
        if (strcmp(iface, ifname) == 0 && dest == 0) {
            unsigned char bytes[4];
            bytes[0] = gateway & 0xFF;
            bytes[1] = (gateway >> 8) & 0xFF;
            bytes[2] = (gateway >> 16) & 0xFF;
            bytes[3] = (gateway >> 24) & 0xFF;

            snprintf(ipbuf, buflen, "%u.%u.%u.%u",
                     bytes[0], bytes[1], bytes[2], bytes[3]);

            fclose(fp);
            return 0;
        }
    }

    fclose(fp);
    return -1;
}
