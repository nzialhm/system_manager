// uci_reader.c
#include "../std.h"
#include "../common/common.h"
#include "uci_reader.h"

#define TYPE_LEN 64
#define NANE_LEN 64
/* ------------------------------ */
static struct uci_section* find_section(struct uci_config *cfg, const char *name)
{
    for (int i = 0; i < cfg->section_count; i++) {
        if (strcmp(cfg->sections[i].name, name) == 0)
            return &cfg->sections[i];
    }
    return NULL;
}

/* ------------------------------ */
int uci_load(const char *filepath, struct uci_config *cfg)
{
    FILE *fp = fopen(filepath, "r");
    if (!fp) return -1;

    char line[256];
    struct uci_section *current = NULL;

    memset(cfg, 0, sizeof(*cfg));

    while (fgets(line, sizeof(line), fp)) {

        char *l = line;
        while (*l == ' ' || *l == '\t') l++;

        if (*l == '#' || *l == '\n')
            continue;

        /* config */
        if (strncmp(l, "config", 6) == 0) {
            char type[TYPE_LEN], name[NANE_LEN];
            sscanf(l, "config %s '%[^']'", type, name);

            struct uci_section *sec = &cfg->sections[cfg->section_count++];
            memset(sec, 0, sizeof(*sec));
            snprintf(sec->name, sizeof(sec->name), "%s", name);

            current = sec;
        }

        /* option */
        else if (strncmp(l, "option", 6) == 0 && current) {
            char key[KEY_LEN], val[VALUE_LEN];
            sscanf(l, "option %s '%[^']'", key, val);

            struct uci_option *opt = &current->options[current->option_count++];
            snprintf(opt->key, sizeof(opt->key), "%s", key);
            snprintf(opt->value, sizeof(opt->value), "%s", val);
        }
    }

    fclose(fp);
    return 0;
}

/* ------------------------------ */
const char* uci_get(struct uci_config *cfg,
                    const char *section,
                    const char *option)
{
    struct uci_section *sec = find_section(cfg, section);
    if (!sec) return NULL;

    for (int i = 0; i < sec->option_count; i++) {
        if (strcmp(sec->options[i].key, option) == 0)
            return sec->options[i].value;
    }

    return NULL;
}

/* ------------------------------ */
int uci_set(struct uci_config *cfg,
            const char *section,
            const char *option,
            const char *value)
{
    struct uci_section *sec = find_section(cfg, section);

    if (!sec) {
        sec = &cfg->sections[cfg->section_count++];
        memset(sec, 0, sizeof(*sec));
        snprintf(sec->name, sizeof(sec->name), "%s", section);
    }

    for (int i = 0; i < sec->option_count; i++) {
        if (strcmp(sec->options[i].key, option) == 0) {
            snprintf(sec->options[i].value,
                     sizeof(sec->options[i].value),
                     "%s", value);
            return 0;
        }
    }

    struct uci_option *opt = &sec->options[sec->option_count++];
    snprintf(opt->key, sizeof(opt->key), "%s", option);
    snprintf(opt->value, sizeof(opt->value), "%s", value);

    return 0;
}

/* ------------------------------ */
int uci_save(const char *filepath, struct uci_config *cfg)
{
    FILE *fp = fopen(filepath, "w");
    if (!fp) return -1;

    for (int i = 0; i < cfg->section_count; i++) {
        struct uci_section *sec = &cfg->sections[i];

        fprintf(fp, "config section '%s'\n", sec->name);

        for (int j = 0; j < sec->option_count; j++) {
            fprintf(fp, "    option %s '%s'\n",
                    sec->options[j].key,
                    sec->options[j].value);
        }
        fprintf(fp, "\n");
    }

    fclose(fp);
    return 0;
}
