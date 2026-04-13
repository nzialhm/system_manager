// master.c
#include "std.h"
#include "../common/common.h"
#include "master.h"
#include "master_module/alive_server.h"

/* ------------------------------
 * 외부에서 호출하는 API
 * ------------------------------ */

int master_init(const char *model)
{
    printf("Start MASTER mode (%s)\n", model);

    /* Alive 서버 */
    if (alive_server_start() < 0)
        return -1;

    /* Image 서버 (uloop 기반 TCP) */
    if (image_server_init() < 0) {
        printf("image_server_init failed\n");
        return -1;
    }

    /* ubus 초기화 */
    if (ubus_init() < 0) {
        printf("ubus_init failed\n");
        return -1;
    }

    /* device cleanup timer */
    cleanup_timer.cb = cleanup_devices_cb;
    uloop_timeout_set(&cleanup_timer, MASTER_ALIVECHECK_TIME);

    return 0;
}
