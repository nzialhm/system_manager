// server_adress.h
#ifndef __SERVERADRESS_H_
#define __SERVERADRESS_H_

/**
 * 지정한 네트워크 인터페이스의 IPv4 주소를 가져온다.
 *
 * @param ifname 인터페이스 이름 (ex: "eth1")
 * @param ipbuf 결과 IP 문자열 저장 버퍼
 * @param buflen 버퍼 크기
 * @return 0 성공, -1 실패
 */
int get_ip_address(const char *ifname, char *ipbuf, int buflen);

#endif //__SERVERADRESS_H_

/**
 * 서버 IP 선택 정책
 * 1. eth1 (유선)
 * 2. 실패 시 nct11af1 fallback
 */
// int get_server_ip(char *ipbuf, int buflen)
// {
//     if (get_ip_address("eth1", ipbuf, buflen) == 0) {
//         return 0;
//     }

//     // fallback (무선)
//     if (get_ip_address("nct11af1", ipbuf, buflen) == 0) {
//         return 0;
//     }

//     return -1;
// }
