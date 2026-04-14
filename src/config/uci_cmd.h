// uci_cmd.h
#ifndef __UCICMD_H_
#define __UCICMD_H_

// 값 읽기
// ex: package="system", section="cfg", option="serialnum"
int uci_get_value(const char *package, const char *section, const char *option, char *out, int out_len);

// 값 쓰기
int uci_set_value(const char *package, const char *section, const char *option, const char *value);

// commit
int uci_commit(const char *package);

#endif //__UCICMD_H_

// char buf[128] = {0};
// // read
// if (uci_get_value("system", "cfg", "serialnum", buf, sizeof(buf)) == 0) {
//     printf("serialnum: %s\n", buf);
// } else {
//     printf("read fail\n");
// }
// // write
// uci_set_value("system", "cfg", "serialnum", "ABC123");
// // commit
// uci_commit("system");
