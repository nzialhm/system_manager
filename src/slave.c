// slave.c
#include "std.h"
#include "../common/common.h"
#include "slave.h"
#include "slave_module/image_client.h"
#include "slave_module/alive_client.h"

int slave_init(const char *model)
{
    printf("Start SLAVE mode (%s)\n", model);

    image_client_start();

    make_alivesocket();
    alive_timer.cb = alive_cb;
    uloop_timeout_set(&alive_timer, SLAVE_ALIVECHECK_TIME);

    return 0;
}

