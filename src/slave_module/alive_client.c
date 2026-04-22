// alive_client.c
#include "../std.h"
#include "../common/common.h"
#include "alive_client.h"
#include "../apip.h"

#define MSG_STR "SERIALNUMBER=%s,KSDEVICEEMISSIONPOWER=%s,KSCERTID=%s,KSDEVICETYPE=%s,MODELID=%s,LATITUDE=%s,LONGITUDE=%s,HEIGHTTYPE=%s,HEIGHT=%s,SLAVEKEY=%d"
#define ALIVESENDBUF_SIZE 1024
#define ALIVERECVBUF_SIZE 128

static int aliveslave_sock = 0;
static struct sockaddr_in aliveslave_srv;
static slave_key = 0;

struct slave_channel chinfo;
/* ------------------------------
 * alive 메시지 파싱
 * ------------------------------ */
static void parseslave_alive(char *msg, struct slave_channel *dev)
{
    char *token = strtok(msg, ",");

    while (token) {
        if (strncmp(token, "SERIALNUMBER=", 13) == 0){
            snprintf(dev->serial, sizeof(dev->serial), "%s", token + 13);
            printf("[SLAVE] ACK from SERIALNUMBER : %s\n", dev->serial);
        }
        else if (strncmp(token, "CHANNELID=", 10) == 0){
            dev->channel_id = atoi(token + 10);
            printf("[SLAVE] ACK from CHANNELID : %d\n", dev->channel_id);
        }

        token = strtok(NULL, ",");
    }
}

void recv_alive_ack(int sock)
{
    char buf[ALIVERECVBUF_SIZE];
    memset(&buf, 0, sizeof(buf));
    struct sockaddr_in srv;
    socklen_t len = sizeof(srv);

    int n = recvfrom(sock, buf, sizeof(buf)-1, 0,
                     (struct sockaddr *)&srv, &len);

    if (n < 0) {
        perror("recvfrom");
        return;
    }

    buf[n] = '\0';

    parseslave_alive(buf, &chinfo);
    slave_key++;

    printf("[SLAVE] ACK from %s: %s\n",
           inet_ntoa(srv.sin_addr), buf);
}

void alive_cb(struct uloop_timeout *t)
{
    send_alive();
    uloop_timeout_set(t, SLAVE_ALIVECHECK_TIME);
}

void make_alivesocket(void)
{
    aliveslave_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (aliveslave_sock < 0) {
        perror("socket");
        return;
    }

    /* timeout 설정 */
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    setsockopt(aliveslave_sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    memset(&aliveslave_srv, 0, sizeof(aliveslave_srv));

    const char *interface_name = "nct11af1";
    const char *interface = uci_get(gsystemmanager_cfg, "common", "server_interface");
    if(interface == NULL){
        interface = interface_name;
    }

    char serverip[MAX_IP_LEN];
    memset(serverip, 0, sizeof(serverip));
    // if (get_gateway_ip("nct11af1", serverip, sizeof(serverip)) == -1) {
    if (get_gateway_ip(interface, serverip, sizeof(serverip)) == -1) {
        printf("AP IP: %s\n", serverip);
    } 
    const char *port_str = uci_get(gsystemmanager_cfg, "common", "alive_port");
    if (!port_str) {
        printf("port_str NULL\n");
        return -1;
    }
    int port = atoi(port_str);

    aliveslave_srv.sin_family = AF_INET;
    aliveslave_srv.sin_port = htons(port);                 //  포트
    aliveslave_srv.sin_addr.s_addr = inet_addr(serverip);        //  IP
}

void close_alivesocket(void)
{
    close(aliveslave_sock);
    aliveslave_sock = 0;
}

void send_alive(void)
{
    char msg[ALIVESENDBUF_SIZE];
    memset(&msg, 0, sizeof(msg));

    const char *serialNumber = uci_get(gsystemmanager_cfg, "slave", "serialNumber");
    const char *ksDeviceEmissionPower = uci_get(gsystemmanager_cfg, "slave", "ksDeviceEmissionPower");
    const char *ksCertId = uci_get(gsystemmanager_cfg, "slave", "ksCertId");
    const char *ksDeviceType = uci_get(gsystemmanager_cfg, "slave", "ksDeviceType");
    const char *modelId = uci_get(gsystemmanager_cfg, "slave", "modelId");
    const char *geo_lati = uci_get(gsystemmanager_cfg, "slave", "geo_lati");
    const char *geo_long = uci_get(gsystemmanager_cfg, "slave", "geo_long");
    const char *ant_heightType = uci_get(gsystemmanager_cfg, "slave", "ant_heightType");
    const char *ant_height = uci_get(gsystemmanager_cfg, "slave", "ant_height");

    snprintf(msg, sizeof(msg),
            MSG_STR,
            serialNumber,
            ksDeviceEmissionPower,
            ksCertId,
            ksDeviceType,
            modelId,
            geo_lati,
            geo_long,
            ant_heightType,
            ant_height,
            slave_key
    );

    sendto(aliveslave_sock, msg, strlen(msg), 0,
           (struct sockaddr *)&aliveslave_srv, sizeof(aliveslave_srv));

    /* ACK 대기 */
    recv_alive_ack(aliveslave_sock);

    printf("[SLAVE] Alive sent\n");
}
