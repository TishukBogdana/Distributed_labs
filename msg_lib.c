#include <errno.h>
#include "serv.h"
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
extern int pipe_tab[MAX_PROCESS_ID+1][MAX_PROCESS_ID+1][2];
int send(void * self, local_id dst, const Message * msg){
    io * data = (io*) self;
    int len = msg->s_header.s_payload_len+sizeof(MessageHeader);
    write(pipe_tab[data->src][dst][1], msg, len);
    return  EXIT_SUCCESS;
}

int send_multicast(void * self, const Message * msg){
    int i=0;
    io * data = (io*) self;
    int len = msg->s_header.s_payload_len+sizeof(MessageHeader);

    for(i=0; i<=data->cnt; i++){
        if( i!= data->src){
       write(pipe_tab[data->src][i][1], msg, len);
     }
   }
    return  EXIT_SUCCESS;

}

int receive(void * self, local_id from, Message * msg)
{
    io * data = (io*) self;
    char buf[sizeof(Message)]= {0};
    int flags;
    memset(msg->s_payload, 0 , MAX_PAYLOAD_LEN);
        if(read(pipe_tab[from][data->src][0], buf, 1) !=-1){
                flags = fcntl(pipe_tab[from][data->src][0], F_GETFL, 0);
                fcntl(pipe_tab[from][data->src][0], F_SETFL, flags^O_NONBLOCK);
                read(pipe_tab[from][data->src][0],buf+1, sizeof(MessageHeader)-1);
                memcpy(msg, buf, sizeof(MessageHeader));
                read(pipe_tab[from][data->src][0],buf, msg->s_header.s_payload_len);
                memcpy(msg->s_payload, buf, msg->s_header.s_payload_len);
                fcntl(pipe_tab[from][data->src][0], F_SETFL, O_NONBLOCK);

        }
    return EXIT_SUCCESS;
}

int receive_any(void * self, Message * msg)
{
     io * data = (io*) self;
     int   i =0;
     int flags;
     char buf[sizeof(Message)] = {0};
     int iters;
     int counter =0;
     memset(msg->s_payload, 0 , MAX_PAYLOAD_LEN);
     if(data->src==PARENT_ID)
        iters=data->cnt;
    else
        iters=data->cnt-1;
    while(counter < iters){
         for (i =1; i<=data->cnt; i++){
         if(i!=data->src){
         msg->s_header.s_magic=0;
         msg->s_header.s_type=0;

                if(read(pipe_tab[i][data->src][0], buf, 1)!=-1){
                flags = fcntl(pipe_tab[i][data->src][0], F_GETFL, 0);
                fcntl(pipe_tab[i][data->src][0], F_SETFL, flags^O_NONBLOCK);
                read(pipe_tab[i][data->src][0],buf+1,sizeof(MessageHeader)-1);
                memcpy(msg, buf, sizeof(MessageHeader));
                read(pipe_tab[i][data->src][0],buf, msg->s_header.s_payload_len);
                memcpy(msg->s_payload, buf,msg->s_header.s_payload_len);
                fcntl(pipe_tab[i][data->src][0], F_SETFL, O_NONBLOCK);
                if((msg->s_header.s_magic)&&(msg->s_header.s_type==data->want_type)){
                    counter ++;
                  // printf("%d rcv %s", data->src, msg->s_payload);
                }
                }
             }
        }
    }
    return EXIT_SUCCESS;
}
//
Message create_msg( int s_magic, int s_type, timestamp_t local_time ,const char * const buf, int src , int pid, int ppid, int money){
    Message msg;
    MessageHeader header;
    header.s_magic = s_magic;
    header.s_type = s_type;
    header.s_local_time = local_time;
    if (!strcmp(buf, log_started_fmt))
    sprintf(msg.s_payload,buf,src, src,pid, ppid, money);
    else {
        if(!strcmp(buf, log_done_fmt))
        sprintf(msg.s_payload, buf,src, src, money);
        else
        sprintf(msg.s_payload, buf, src, src);

    }
    header.s_payload_len = strlen(msg.s_payload);
    msg.s_header = header;
    return msg;
}
//timestamp_t get_physical_time(){return 0;}
