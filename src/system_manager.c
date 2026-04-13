// system_manager.c
#include "std.h"
#include "../common/common.h"
#include "../config/config_reader.h"
#include <libubox/uloop.h>
#include <libubus.h>

#define MASTER_MODE "master"
#define SLAVE_MODE "slave"

struct devconfig g_cfg;

extern int master_init(const char *model);
extern int slave_init(const char *model);
extern struct ubus_context *ubus_get_ctx(void);

static struct uci_config systemmanager_cfg;
#define SLAVECONFIG_PATH "/nct11af/system_manager/systemmanager_config"
struct uci_config* gsystemmanager_cfg = &systemmanager_cfg;

int main(int argc, char **argv)
{
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

        char* _nct11af_mode = config_get(&g_cfg, "nct11af_mode");
        char* _serialnum = config_get(&g_cfg, "serialnum");

        if (!_nct11af_mode || !_serialnum) {
            printf("[ERROR] config missing\n");
            return -1;
        }

        /* mode 설정 */
        if (!strcmp(_nct11af_mode, "ap"))
            mode = MASTER_MODE;
        else if (!strcmp(_nct11af_mode, "sta"))
            mode = SLAVE_MODE;
        else {
            printf("[ERROR] unknown nct11af_mode: %s\n", _nct11af_mode);
            return -1;
        }

        /* model 추출 (앞 4자리) */
        static char model_buf[16];  // static으로 유지
        memset(model_buf, 0, sizeof(model_buf));

        strncpy(model_buf, _serialnum, 4);
        model_buf[4] = '\0';

        model = model_buf;

        printf("[CONFIG] mode=%s model=%s\n", mode, model);
    }

    if(uci_load(SLAVECONFIG_PATH, &systemmanager_cfg)==-1)
    {
        printf("config file error \n");
        return -1;
    }
   
   uloop_init();

    /* 이거 반드시 필요 */
    ubus_init();

    if (!strcmp(mode, MASTER_MODE)) {
        master_init(model);
    } else {
        slave_init(model);
    }

    ubus_add_uloop(ubus_get_ctx());

    uloop_run();
    uloop_done();

    return 0;
}
