#ifndef HEAD_H_INCLUDED
#define HEAD_H_INCLUDED
#pragma comment(lib,"ws2_32.lib")
#include<stdio.h>
#include<winsock2.h>
#include<stdlib.h>
#include<windows.h>
#include<string.h>
#include<process.h>
#define THREAD_NUM 10
#define USER_SEND_MAX 280
#define BROADCAST_MAX 500
#define FILE_DATA_MAX 8092
#include <sqlite3.h>
#include <io.h>
/*
struct sockaddr_in
{
    short   sin_family; //eg.AF_INET,AF_INET6
    unsigned short sin_port; //eg.htons(3490)
    struct in_addr sin_addr;
    char sin_zero[8];
};

typedef struct in_addr
{
    union
    {
        struct{
        u_char s_b1,s_b2,s_b3,s_b4;//unsigned char
        }S_un_b;
        struct{
        u_short s_w1,s_w2;
        }S_un_w;
        u_long S_addr;
    }S_un;
}IN_ADDR,*PIN_ADDR,FAR*LPIN_ADDR;

struct sockaddr{
    unsigned short sa_family;
    char sa_data[14];
};
*/




#endif // HEAD_H_INCLUDED
