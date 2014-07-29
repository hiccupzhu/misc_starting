#include <stdio.h>
#include <string.h>

typedef unsigned char u8_t;
typedef unsigned short u16_t;

u16_t check_sum(u16_t *w,int len){
    int sum = 0;
    for(sum = 0; len > 1; len -= 2) {
        sum += *w++;
    }
    if (len == 1) {
        sum += *(unsigned char *) w;
    }
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return (u16_t)(~sum);
}

int main(){
    FILE* fp = fopen("e:\\temp\\ip.dat", "rb");
    static const int count = 200;
    u8_t buf[count];
    fread(buf, count, 1, fp);
//     for(int i = 0;i < count; i++){
//         printf("%02X ", (int)buf[i]);
//     }
//     printf("\n");
    memmove(buf + 34 + 12, buf + 34, 86);
    u8_t* p = buf + 34;
    *p++ = 10;
    *p++ = 0;
    *p++ = 1;
    *p++ = 10;
    *p++ = 113;
    *p++ = 31;
    *p++ = 30;
    *p++ = 141;
    *p++ = 0;
    *p++ = 17;
    *p++ = 0;
    *p++ = 86;
    p += 4;
    *p++ = 0;
    *p++ = 86;
    u16_t cksum = check_sum((u16_t*)(buf + 34), 86 + 12);
    printf("check sum:0x%04X\n", cksum);
    return 0;
}