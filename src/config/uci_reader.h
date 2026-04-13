// uci_reader.h
#ifndef __UCI_READER_H__
#define __UCI_READER_H__

#define MAX_SECTIONS 64
#define MAX_OPTIONS  64

#define KEY_LEN 64
#define VALUE_LEN 512

struct uci_option {
    char key[KEY_LEN];
    char value[VALUE_LEN];
};

struct uci_section {
    char name[KEY_LEN];
    struct uci_option options[MAX_OPTIONS];
    int option_count;
};

struct uci_config {
    struct uci_section sections[MAX_SECTIONS];
    int section_count;
};

/* API */
int uci_load(const char *filepath, struct uci_config *cfg);
const char* uci_get(struct uci_config *cfg,
                    const char *section,
                    const char *option);

int uci_set(struct uci_config *cfg,
            const char *section,
            const char *option,
            const char *value);

int uci_save(const char *filepath, struct uci_config *cfg);

#endif
//사용 예제
// struct uci_config cfg;
// uci_load("/nct11af/system_manager", &cfg);
// const char *ip = uci_get(&cfg, "lan", "ipaddr");
// printf("IP = %s\n", ip);
// uci_set(&cfg, "lan", "ipaddr", "192.168.1.100");
// uci_save("/etc/config/network", &cfg);
