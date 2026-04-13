// image_client.c
#include "../std.h"
#include "../common/common.h"
#include "image_client.h"

static int imageslave_sock = 0;
static struct sockaddr_in imageslave_srv;

void image_client_start(void)
{
    imageslave_sock = socket(AF_INET, SOCK_STREAM, 0);

    const char *ip = uci_get(gsystemmanager_cfg, "common", "server_ip");
    const char *port_str = uci_get(gsystemmanager_cfg, "common", "image_port");
    if (!port_str) {
        printf("port_str NULL\n");
        return -1;
    }
    int port = atoi(port_str);

    memset(&imageslave_srv, 0, sizeof(imageslave_srv));
    imageslave_srv.sin_family = AF_INET;
    imageslave_srv.sin_port = htons(port);
    imageslave_srv.sin_addr.s_addr = inet_addr(ip);

    if (connect(imageslave_sock, (struct sockaddr *)&imageslave_srv, sizeof(imageslave_srv)) == 0) {
        printf("Request image update\n");

        send(imageslave_sock, "update", 6, 0);
    }

    close(imageslave_sock);
    imageslave_sock = 0;
}
