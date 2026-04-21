// system_manager.c
#include "std.h"
#include "common/common.h"
#include "config/config_reader.h"
#include "ubus/ubus.h"

#define MASTER_MODE "master"
#define SLAVE_MODE "slave"

struct devconfig g_cfg;

extern int master_init(const char *model);
extern int slave_init(const char *model);
extern struct ubus_context *ubus_get_ctx(void);

static struct uci_config systemmanager_cfg;
#define SLAVECONFIG_PATH "/nct11af/system_manager/systemmanager_config"
struct uci_config* gsystemmanager_cfg = &systemmanager_cfg;

//ubus 
char g_mode[32];
char g_model[32];

int main(int argc, char **argv)
{
    char ip[MAX_IP_LEN];
    char serverip[MAX_IP_LEN];

    memset(g_mode, 0, sizeof(g_mode));
    memset(g_model, 0, sizeof(g_model));
    
    const char *model = NULL;
    const char *mode = NULL;

    read_all_config(DEVICENAME, &g_cfg);


    for (int i = 1; i < argc; i++) {
        if (!strncmp(argv[i], "model=", 6))
            model = argv[i] + 6;
        else if (!strncmp(argv[i], "mode=", 5))
            mode = argv[i] + 5;
    }

    if (!model || !mode) {
        printf("[INFO] Using config values\n");

        char nct11af_mode[32] = {0};
        char serialnum[64] = {0};

        /* UCI에서 읽기 */
        if (uci_get_value("wireless", "default_nct11af1", "mode",
                        nct11af_mode, sizeof(nct11af_mode)) < 0) {
            printf("[ERROR] failed to get wireless mode\n");
            return -1;
        }

        if (uci_get_value("system", "@system[0]", "serialnum",
                        serialnum, sizeof(serialnum)) < 0) {
            printf("[ERROR] failed to get serialnum\n");
            return -1;
        }

        /* mode 설정 */
        if (!strcmp(nct11af_mode, "ap"))
            mode = (char*)MASTER_MODE;
        else if (!strcmp(nct11af_mode, "sta"))
            mode = (char*)SLAVE_MODE;
        else {
            printf("[ERROR] unknown nct11af_mode: %s\n", nct11af_mode);
            return -1;
        }

        /* model 추출 (앞 4자리) */
        static char model_buf[16];
        memset(model_buf, 0, sizeof(model_buf));

        strncpy(model_buf, serialnum, 4);
        model_buf[4] = '\0';

        model = model_buf;

        printf("[CONFIG] mode=%s model=%s\n", mode, model);
    }

    memset(&systemmanager_cfg, 0, sizeof(systemmanager_cfg));
    if(uci_load(SLAVECONFIG_PATH, &systemmanager_cfg)==-1)
    {
        printf("config file error \n");
        return -1;
    }
    printf("port=%s\n", uci_get(gsystemmanager_cfg, "common", "alive_port"));
   
   uloop_init();

    /* 이거 반드시 필요 */
    ubus_init();

    if (!strcmp(mode, MASTER_MODE)) {
        master_init(model);
    } else {
        slave_init(model);
    }

    strncpy(g_mode, mode, strlen(mode));
    strncpy(g_model, model, strlen(model));

    uloop_run();
    uloop_done();

    return 0;
}
