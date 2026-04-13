// image_server.c
#include "../std.h"
#include "../common/common.h"
#include "image_server.h"

static int imagemaster_sock = 0;
static struct sockaddr_in imagemaster_srv;

static void image_accept_cb(struct uloop_fd *u, unsigned int events)
{
    while (1) {
        int c = accept(u->fd, NULL, NULL);

        if (c < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;

            perror("accept");
            break;
        }

        printf("Image update request\n");

        close(c);
    }
}

int image_server_init(void)
{
    imagemaster_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (imagemaster_sock < 0) {
        perror("socket");
        return -1;
    }

    int flags = fcntl(imagemaster_sock, F_GETFL, 0);
    fcntl(imagemaster_sock, F_SETFL, flags | O_NONBLOCK);

    int opt = 1;
    setsockopt(imagemaster_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&imagemaster_srv, 0, sizeof(imagemaster_srv));

    const char *port_str = uci_get(gsystemmanager_cfg, "common", "image_port");
    if (!port_str) {
        printf("port_str NULL\n");
        return -1;
    }
    int port = atoi(port_str);

    imagemaster_srv.sin_family = AF_INET;
    imagemaster_srv.sin_port = htons(port);
    imagemaster_srv.sin_addr.s_addr = INADDR_ANY;

    if (bind(imagemaster_sock, (struct sockaddr *)&imagemaster_srv, sizeof(imagemaster_srv)) < 0) {
        perror("bind");
        close(imagemaster_sock);
        return -1;
    }

    if (listen(imagemaster_sock, 5) < 0) {
        perror("listen");
        close(imagemaster_sock);
        return -1;
    }

    img_fd.fd = imagemaster_sock;
    img_fd.cb = image_accept_cb;

    uloop_fd_add(&img_fd, ULOOP_READ);

    printf("Image server started (uloop)\n");

    return 0;
}
