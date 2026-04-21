// apip.h
#ifndef __APIP_H_
#define __APIP_H_

/**
 * default gateway (AP IP) 가져오기
 * @param ifname 인터페이스 (ex: "nct11af1")
 * @param ipbuf 결과 저장
 * @param buflen 버퍼 크기
 * @return 0 성공, -1 실패
 */
int get_gateway_ip(const char *ifname, char *ipbuf, int buflen);

#endif //__APIP_H_


// char ip[MAX_IP_LEN];

// // TVWS 무선 기준
// if (get_gateway_ip("nct11af1", ip, sizeof(ip)) == 0) {
//     printf("AP IP: %s\n", ip);
// } else {
//     printf("Failed to get AP IP\n");
// }
