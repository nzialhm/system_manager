

//config_reader.h
#ifndef __CONFIG_READER_H_
#define __CONFIG_READER_H_

#define MAX_KEY_LEN   64
#define MAX_VAL_LEN   128
#define MAX_CONFIG    64

#define DEVICENAME "/dev/mmcblk1p6"

struct config_item {
    char key[MAX_KEY_LEN];
    char val[MAX_VAL_LEN];
};

struct devconfig {
    struct config_item items[MAX_CONFIG];
    int count;
};

extern struct devconfig g_cfg;


/* 읽기 */
int read_all_config(const char *device, struct devconfig *cfg);

/* 조회 */
const char* config_get(struct devconfig *cfg, const char *key);

#endif //__CONFIG_READER_H_
