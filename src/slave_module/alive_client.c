// alive_client.c
#include "../std.h"
#include "../common/common.h"
#include "alive_client.h"
#include "../apip.h"
#include "../config/uci_cmd.h"

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
            printf("[SLAVE] ACK SERIALNUMBER : %s\n", dev->serial);
        }
        else if (strncmp(token, "CHANNELID=", 10) == 0){
            dev->channel_id = atoi(token + 10);
            printf("[SLAVE] ACK CHANNELID : %d\n", dev->channel_id);
        }
        else if (strncmp(token, "ACKMSG=", 7) == 0){
            printf("[SLAVE] ACK ACKMSG : %s\n", (token + 7));
        }
        else if (strncmp(token, "SLAVEKEY=", 9) == 0){
            printf("[SLAVE] ACK SLAVEKEY : %s\n", (token + 9));
        }
        else if (strncmp(token, "USEABLE=", 8) == 0){
            dev->useable = atoi(token + 8);
            printf("[SLAVE] ACK USEABLE : %s\n", (token + 8));
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

    printf("[SLAVE] ACK from == %s ==\n",  inet_ntoa(srv.sin_addr));
}

void alive_cb(struct uloop_timeout *t)
{
    send_alive();
    uloop_timeout_set(t, SLAVE_ALIVECHECK_TIME);
}

void make_alivesocket(void)
{
    memset(&chinfo, 0, sizeof(chinfo));
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

    // const char *serialNumber = uci_get(gsystemmanager_cfg, "slave", "serialNumber");
    // const char *ksDeviceEmissionPower = uci_get(gsystemmanager_cfg, "slave", "ksDeviceEmissionPower");
    // const char *ksCertId = uci_get(gsystemmanager_cfg, "slave", "ksCertId");
    // const char *ksDeviceType = uci_get(gsystemmanager_cfg, "slave", "ksDeviceType");
    // const char *modelId = uci_get(gsystemmanager_cfg, "slave", "modelId");
    // const char *geo_lati = uci_get(gsystemmanager_cfg, "slave", "geo_lati");
    // const char *geo_long = uci_get(gsystemmanager_cfg, "slave", "geo_long");
    // const char *ant_heightType = uci_get(gsystemmanager_cfg, "slave", "ant_heightType");
    // const char *ant_height = uci_get(gsystemmanager_cfg, "slave", "ant_height");

    char serialNumber[64] = {0};
    char ksDeviceEmissionPower[64] = {0};
    char ksCertId[64] = {0};
    char ksDeviceType[64] = {0};
    char modelId[64] = {0};
    char geo_lati[64] = {0};
    char geo_long[64] = {0};
    char ant_heightType[64] = {0};
    char ant_height[64] = {0};

    if (uci_get_value("system", "dev", "serialNumber", serialNumber, sizeof(serialNumber)) < 0) {
        printf("[ERROR] failed serialNumber\n");
    }
    if (uci_get_value("system", "dev", "emissionPower", ksDeviceEmissionPower, sizeof(ksDeviceEmissionPower)) < 0) {
        printf("[ERROR] failed ksDeviceEmissionPower\n");
    }
    if (uci_get_value("system", "dev", "ksCertId", ksCertId, sizeof(ksCertId)) < 0) {
        printf("[ERROR] failed ksCertId\n");
    }
    if (uci_get_value("system", "dev", "deviceType", ksDeviceType, sizeof(ksDeviceType)) < 0) {
        printf("[ERROR] failed ksDeviceType\n");
    }
    if (uci_get_value("system", "dev", "modelId", modelId, sizeof(modelId)) < 0) {
        printf("[ERROR] failed modelId\n");
    }
    if (uci_get_value("paws", "global", "lati", geo_lati, sizeof(geo_lati)) < 0) {
        printf("[ERROR] failed geo_lati\n");
    }
    if (uci_get_value("paws", "global", "long", geo_long, sizeof(geo_long)) < 0) {
        printf("[ERROR] failed geo_long\n");
    }
    if (uci_get_value("paws", "dev", "ant_heightType", ant_heightType, sizeof(ant_heightType)) < 0) {
        printf("[ERROR] failed ant_heightType\n");
    }
    if (uci_get_value("paws", "dev", "ant_height", ant_height, sizeof(ant_height)) < 0) {
        printf("[ERROR] failed ant_height\n");
    }
    

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

    printf("[SLAVE] Alive send ==========================\n");
    sendto(aliveslave_sock, msg, strlen(msg), 0,
           (struct sockaddr *)&aliveslave_srv, sizeof(aliveslave_srv));

    printf("[SLAVE] Alive recv ==========================\n");
    /* ACK 대기 */
    recv_alive_ack(aliveslave_sock);

}
