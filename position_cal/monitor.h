#ifndef  __MONITOR_H__
#define __MONITOR_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define MONITOR_PORT 10000
#define QUERY_PORT   10000
#define STR_QUERY_IP     "192.168.60.110"

typedef enum{
    CONVERT_XY = 0x90,
    CONVERT_LB,
    DRAW_MAP,
}action_map_e;

struct pos_longlati_t{
    double longti;
    double lati;
};


#define CONVERT_BUFF_LEN 1024
struct action_map_t{
    int  action;
    int  len;
    char conv_buf[CONVERT_BUFF_LEN];
}__attribute__((packed));

#define XY_ARR_SIZE  30

struct pos_xy_t{
   double x;
   double y;
};



extern int Send_UdpPack(int fd,struct sockaddr_in *dst_addr,char *buf,int len);
extern int Create_UDPServer(short port);
extern int Poll_RecvMsg(int fd,char *buf,int len);
extern void pos_calc_init();
extern void pos_convert_xy(struct pos_longlati_t *ppos,struct pos_xy_t *pos_xy,int num);
extern void show_xy_func(double x,double y); 
extern void convert_xy_lb(struct pos_xy_t *pos_xy,int num,char *strbuf);
extern void show_multi_points();


#endif
