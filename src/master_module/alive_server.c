// alive_server.c
#include "../std.h"
#include "../common/common.h"
#include "alive_server.h"

/* ------------------------------
 * 설정값
 * ------------------------------ */
#define DEVICE_TIMEOUT 30   // seconds

#define ALIVERECVBUF_SIZE 1024
#define MSGBUF_SIZE 1024
#define MSGACKBUF_SIZE 128

static int alivemaster_sock = 0;
static struct sockaddr_in alivemaster_srv;
#define MSGACK_STR "ACKMSG=%s,SERIALNUMBER=%s,CHANNELID=%d,SLAVEKEY=%d,USEABLE=%d"

static void send_alive_ack(int sock, struct sockaddr_in *cli, struct device_info *dev)
{
    char msg[MSGACKBUF_SIZE];
    memset(&msg, 0, sizeof(msg));
    snprintf(msg, sizeof(msg),
            MSGACK_STR,
            "OK",
            dev->serial,
            dev->channel_id,
            dev->slave_key,
            dev->useable
            );
    
    sendto(sock, msg, strlen(msg), 0,
           (struct sockaddr *)cli, sizeof(*cli));
}

/* ------------------------------
 * device 찾기
 * ------------------------------ */
static struct device_info* find_device(const char *serial)
{
    struct device_info *d;

    list_for_each_entry(d, &device_list, list) {
        if (strcmp(d->serial, serial) == 0)
            return d;
    }

    return NULL;
}

/* ------------------------------
 * alive 메시지 파싱
 * ------------------------------ */
static void parse_alive(char *msg, struct device_info *dev)
{
    char *token = strtok(msg, ",");

    while (token) {
        if (strncmp(token, "SERIALNUMBER=", 13) == 0)
            snprintf(dev->serial, sizeof(dev->serial), "%s", token + 13);

        else if (strncmp(token, "MODELID=", 8) == 0)
            snprintf(dev->model_id, sizeof(dev->model_id), "%s", token + 8);

        else if (strncmp(token, "KSCERTID=", 9) == 0)
            snprintf(dev->cert_id, sizeof(dev->cert_id), "%s", token + 9);

        else if (strncmp(token, "KSDEVICETYPE=", 13) == 0)
            snprintf(dev->type, sizeof(dev->type), "%s", token + 13);

        else if (strncmp(token, "KSDEVICEEMISSIONPOWER=", 22) == 0)
            dev->power = atoi(token + 22);

        else if (strncmp(token, "LATITUDE=", 9) == 0)
            dev->lat = atof(token + 9);

        else if (strncmp(token, "LONGITUDE=", 10) == 0)
            dev->lon = atof(token + 10);

        else if (strncmp(token, "HEIGHTTYPE=", 11) == 0)
            snprintf(dev->height_type, sizeof(dev->height_type), "%s", token + 11);

        else if (strncmp(token, "HEIGHT=", 7) == 0)
            dev->height = atof(token + 7);

        else if (strncmp(token, "SLAVEKEY=", 9) == 0)
            dev->slave_key = atoi(token + 9);

        token = strtok(NULL, ",");
    }
}

/* ------------------------------
 * device 업데이트
 * ------------------------------ */
static struct device_info*  update_device(struct sockaddr_in *cli, char *msg)
{
    struct device_info tmp;
    memset(&tmp, 0, sizeof(tmp));

    inet_ntop(AF_INET, &cli->sin_addr, tmp.ip, sizeof(tmp.ip));

    char msg_copy[MSGBUF_SIZE];
    snprintf(msg_copy, sizeof(msg_copy), "%s", msg);

    parse_alive(msg_copy, &tmp);

    /* 필수 방어 */
    if (tmp.serial[0] == '\0') {
        printf("[WARN] SERIALNUMBER missing\n");
        return NULL;
    }

    struct device_info *dev = find_device(tmp.serial);

    if (dev) {
        /* 안전 복사 */
        snprintf(dev->serial, sizeof(dev->serial), "%s", tmp.serial);
        snprintf(dev->model_id, sizeof(dev->model_id), "%s", tmp.model_id);
        snprintf(dev->cert_id, sizeof(dev->cert_id), "%s", tmp.cert_id);
        snprintf(dev->type, sizeof(dev->type), "%s", tmp.type);
        snprintf(dev->ip, sizeof(dev->ip), "%s", tmp.ip);
        dev->power = tmp.power;
        dev->lat = tmp.lat;
        dev->lon = tmp.lon;
        dev->height = tmp.height;
        dev->slave_key = tmp.slave_key;
 
        snprintf(dev->height_type, sizeof(dev->height_type), "%s", tmp.height_type);

        dev->online = 1;
        dev->last_seen = time(NULL);

        printf("[UPDATE] %s (%s)\n", dev->serial, dev->ip);
    } else {
        dev = malloc(sizeof(*dev));
        if (!dev)
            return dev;

        memset(dev, 0, sizeof(*dev));
        memcpy(dev, &tmp, sizeof(tmp));

        INIT_LIST_HEAD(&dev->list);

        dev->online = 1;
        dev->pawsnew = 1;
        dev->useable = 1;

        dev->last_seen = time(NULL);

        list_add_tail(&dev->list, &device_list);

        printf("[NEW] %s (%s)\n", dev->serial, dev->ip);
    }
    return dev;
}

/* ------------------------------
 * Alive 수신
 * ------------------------------ */
static void alive_recv_cb(struct uloop_fd *u, unsigned int events)
{
    while (1) {
        char buf[ALIVERECVBUF_SIZE];
        memset(&buf, 0, sizeof(buf));
        struct sockaddr_in cli;
        socklen_t len = sizeof(cli);

        int n = recvfrom(u->fd, buf, sizeof(buf)-1, 0,
                         (struct sockaddr *)&cli, &len);

        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;

            perror("recvfrom");
            break;
        }

        buf[n] = '\0';

        struct device_info *dev = update_device(&cli, buf);

        send_alive_ack(u->fd, &cli, dev);

        printf("[MASTER] Alive from %s: %s : SLAVEKEY=%d\n",
        inet_ntoa(cli.sin_addr), buf, dev->slave_key);
    }
}

/* ------------------------------
- * device timeout 정리
- * ------------------------------ */
void cleanup_devices_cb(struct uloop_timeout *t)
{
    struct device_info *d, *tmp;
    time_t now = time(NULL);

    list_for_each_entry_safe(d, tmp, &device_list, list) {
        if (now - d->last_seen > DEVICE_TIMEOUT) {
            d->online = 0;
            printf("[REMOVE] %s (%s)\n",
                   d->serial,
                   d->ip);

            list_del(&d->list);
            free(d);
        }
        else {
            d->online = d->online + 1;
        }
    }

    /* 5초마다 반복 */
    uloop_timeout_set(t, MASTER_ALIVECHECK_TIME);
}


/* ------------------------------
 * Alive 서버 시작
 * ------------------------------ */
int alive_server_start(void)
{
    alivemaster_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (alivemaster_sock < 0) {
        perror("socket");
        return -1;
    }

    int flags = fcntl(alivemaster_sock, F_GETFL, 0);
    fcntl(alivemaster_sock, F_SETFL, flags | O_NONBLOCK);

    memset(&alivemaster_srv, 0, sizeof(alivemaster_srv));

    const char *port_str = uci_get(gsystemmanager_cfg, "common", "alive_port");
    if (!port_str) {
        printf("port_str NULL\n");
        return -1;
    }
    int port = atoi(port_str);

    alivemaster_srv.sin_family = AF_INET;
    alivemaster_srv.sin_port = htons(port);
    alivemaster_srv.sin_addr.s_addr = INADDR_ANY;

    if (bind(alivemaster_sock, (struct sockaddr *)&alivemaster_srv, sizeof(alivemaster_srv)) < 0) {
        perror("bind");
        close(alivemaster_sock);
        return -1;
    }

    sock_fd.fd = alivemaster_sock;
    sock_fd.cb = alive_recv_cb;

    uloop_fd_add(&sock_fd, ULOOP_READ);

    printf("[MASTER] Alive server start (port %d)\n", port);

    return 0;
}
