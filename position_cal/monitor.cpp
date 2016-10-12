#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <unistd.h>
#include "monitor.h"


int    sockfd = -1;
struct sockaddr_in client_addr;

void *monitor_func(void *arg)
{
	char msgbuf[1600]={0,};
	int len = -1;
	while(1){
		len = Poll_RecvMsg(sockfd,msgbuf,sizeof(msgbuf));
		if(len == -1)
			continue;
		                
	}
}
char* get_xy_unit(char *buf,struct pos_xy_t *pos)
{
   char *next = NULL;
   double x,y;
   
   if(NULL == buf)
   {
     return NULL;
   }
   printf("%s\n",buf);
   x = strtod(buf,&next);
   if(*next != ','  ){
     fprintf(stderr,"%s\n","wrong xy format!!!");
     return NULL;
   }
   buf = next+1;
   y = strtod(buf,&next);
   pos->x = x;
   pos->y = y;
   printf("[x,y]=[%.3f,%.3f]\n",x,y);
   return next;
}

#define TEST_STR  "123.45,678.98 | 12.12,78.98 |987.456,98.000 | 12345.10,8765.0|123.456,987.678"

char arr_lb_res[1024];

void get_xy_func2(struct pos_xy_t *ppos,int num)
{
        char msgbuf[1024]={0,};
        int len = -1;
        int i   = 0;
        char *ptr = msgbuf;
        char *next = NULL;
        char *p = NULL;	

        len = Poll_RecvMsg(sockfd,msgbuf,sizeof(msgbuf));
        if(len == -1){
         fprintf(stderr,"%s\n","can't get xy from udp server");
         return ;
        }
        memset(arr_lb_res,0,len > sizeof(arr_lb_res)?sizeof(arr_lb_res):len);
        memcpy(arr_lb_res,msgbuf,len);	
	printf("res_LB:[%s]\n",arr_lb_res);
//        memcpy(msgbuf,TEST_STR,strlen(TEST_STR));
        next = strtok_r(ptr,"|",&p);     
        get_xy_unit(ptr,ppos);
        i++; 
        while(next != NULL){ 
         next = strtok_r(NULL,"|",&p);    
         if(NULL == next){
           break;
         } 
         get_xy_unit(next,ppos+i);
         i++;
         if(i>=num){
          break;
         }
        }
}



void get_xy_func(struct pos_xy_t *ppos,int num)
{
        char msgbuf[1024]={0,};
        int len = -1;
        int i   = 0;
        char *ptr = msgbuf;
        char *next = NULL;
        char *p = NULL;	

        len = Poll_RecvMsg(sockfd,msgbuf,sizeof(msgbuf));
        if(len == -1){
         fprintf(stderr,"%s\n","can't get xy from udp server");
         return ;
        }
//        memcpy(msgbuf,TEST_STR,strlen(TEST_STR));
        next = strtok_r(ptr,"|",&p);     
        get_xy_unit(ptr,ppos);
        i++; 
        while(next != NULL){ 
         next = strtok_r(NULL,"|",&p);    
         if(NULL == next){
           break;
         } 
         get_xy_unit(next,ppos+i);
         i++;
         if(i>=num){
          break;
         }
        }
}

int Create_UDPServer(short port)
{
	int fd;
	struct sockaddr_in addr;

	addr.sin_family      = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port        = htons(port);
	fd = socket (AF_INET , SOCK_DGRAM , IPPROTO_UDP );
	if(fd <= 0){
		return -1;
	}
	if(-1 == bind(fd,(struct sockaddr *)&addr,sizeof(addr))){
		close(fd);
		return -1;
	}

	return fd;
}



int Send_UdpPack(int fd,struct sockaddr_in *dst_addr,char *buf,int len)
{
	int sendlen;
	
	sendlen = sendto (fd,buf,len, 0 ,(struct sockaddr *)dst_addr,sizeof(struct sockaddr_in));
	if(sendlen!=len){
		return -1;
	}
	return sendlen;
}

int Poll_RecvMsg(int fd,char *buf,int len)
{
	int sret;
	int recvlen;
	int maxfdp = 0;
	struct timeval timeout;
	fd_set   fds;
	int addrlen;
	
	maxfdp = fd;

	timeout.tv_sec = 0;
	timeout.tv_usec = 20*1000;

	FD_ZERO(&fds);
	FD_SET(fd,&fds);
	sret   =   select(maxfdp+1,&fds,NULL,NULL,&timeout);
		    
	if(sret<=0){
		return -1;
	}
        if(FD_ISSET(fd,&fds)){
		addrlen = sizeof(struct sockaddr);
		recvlen = recvfrom (fd , buf , len , 0, (struct sockaddr *)&client_addr,(socklen_t*)&addrlen);

		if (recvlen<=0){
			return -1;
		}
	}
	return recvlen;
}



void pos_convert_xy(struct pos_longlati_t *ppos,struct pos_xy_t *pos_xy,int num)
{
  
  char    send_buf[512]={0,};
  struct  action_map_t  act;
  int     i = 0;
  char    *ptr = act.conv_buf;
  int     offlen = 0;
  int     left = sizeof(act.conv_buf);
//  struct  pos_xy_t pos_xy[XY_ARR_SIZE];
 
  memset(&act,0,sizeof(act)); 
//  memset(pos_xy,0,sizeof(pos_xy));
  for(i=0;i<num;i++){
    if(i>0)
   	 offlen = snprintf(ptr,left,"|%.6f,%.6f",ppos[i].longti,ppos[i].lati);
    else
   	 offlen = snprintf(ptr,left,"%.6f,%.6f",ppos[i].longti,ppos[i].lati);
    ptr += offlen;
    left -= offlen;
  }
  act.action = CONVERT_XY;
  act.len    = ptr - act.conv_buf;
  memcpy(send_buf,&act,sizeof(act));
  Send_UdpPack(sockfd,&client_addr,send_buf,sizeof(send_buf));//(char*)&act,sizeof(act.action)+sizeof(act.len)+act.len+1);
  get_xy_func(pos_xy,num > XY_ARR_SIZE ? XY_ARR_SIZE : num);
}

void convert_xy_lb(struct pos_xy_t *pos_xy,int num,char *strbuf)
{
  struct  action_map_t  act;
  char    *ptr = act.conv_buf;
  char    send_buf[1280]={0,}; 
  memset(&act,0,sizeof(act));
  memcpy(act.conv_buf,strbuf,strlen(strbuf)+1); 
  act.action = CONVERT_LB;
  act.len    = strlen(ptr);
  memcpy(send_buf,&act,sizeof(act));
  Send_UdpPack(sockfd,&client_addr,send_buf,sizeof(send_buf));
  get_xy_func2(pos_xy,num > XY_ARR_SIZE ? XY_ARR_SIZE : num);
}

void show_multi_points()
{
  struct  action_map_t  act;
  char    *ptr = act.conv_buf;
  char    send_buf[1280]={0,}; 
  memset(&act,0,sizeof(act));
  memcpy(act.conv_buf,arr_lb_res,sizeof(arr_lb_res)); 
  act.action = DRAW_MAP;
  act.len    = strlen(ptr);
  memcpy(send_buf,&act,sizeof(act));
  Send_UdpPack(sockfd,&client_addr,send_buf,sizeof(send_buf));
}


void show_xy_func(double x,double y)
{
  char    send_buf[256]={0,};
  struct  action_map_t  act;
  char    *ptr = act.conv_buf;
 
  memset(&act,0,sizeof(act)); 
  snprintf(ptr,sizeof(act.conv_buf),"%.6f,%.6f",x,y);
  act.action = CONVERT_LB;
  act.len    = strlen(ptr);
  memcpy(send_buf,&act,sizeof(act));
  Send_UdpPack(sockfd,&client_addr,send_buf,sizeof(send_buf)); 
}

void pos_calc_init()
{
	pthread_t tid;
	int on=1;
	sockfd = Create_UDPServer(MONITOR_PORT);
/*	if(sockfd != -1){
		pthread_create(&tid,NULL,monitor_func,NULL);
		setsockopt(sockfd,SOL_SOCKET,SO_BROADCAST,(char*)&on,sizeof(on));
	}else{
                fprintf(stderr,"%s\n","create sock error");
                exit(0);
        }*/
	client_addr.sin_family      = AF_INET;
	client_addr.sin_addr.s_addr = inet_addr(STR_QUERY_IP);
	client_addr.sin_port        = htons(QUERY_PORT);
}



