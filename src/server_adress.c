// server_address.c
#include "std.h"
#include "../common/common.h"
#include "server_adress.h"
#include <unistd.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/socket.h>

/**
 * 인터페이스 IP 가져오기
 */
int get_ip_address(const char *ifname, char *ipbuf, int buflen)
{
    int fd;
    struct ifreq ifr;

    if (!ifname || !ipbuf)
        return -1;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
        return -1;

    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ - 1);

    if (ioctl(fd, SIOCGIFADDR, &ifr) < 0) {
        close(fd);
        return -1;
    }

    struct sockaddr_in *ipaddr = (struct sockaddr_in *)&ifr.ifr_addr;

    snprintf(ipbuf, buflen, "%s", inet_ntoa(ipaddr->sin_addr));

    close(fd);
    return 0;
}


