#include <stdio.h>
#include <stdlib.h>
#include "pa2345.h"
#include <fcntl.h>
#include <unistd.h>
#include "func.h"
#include "banking.h"
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
int pipe_tab[MAX_PROCESS_ID+1][MAX_PROCESS_ID+1][2];

void transfer(void * parent_data, local_id src, local_id dst,
              balance_t amount)
{

    TransferOrder * order ;
    Message * msg;
    msg = (Message *) malloc( sizeof(Message));
    order = (TransferOrder* ) malloc(sizeof(TransferOrder));
    msg->s_header.s_magic=MESSAGE_MAGIC;
    msg->s_header.s_type = TRANSFER;
    msg->s_header.s_local_time = get_physical_time();
    order->s_src = src;
    order-> s_dst=dst;
    order->s_amount= amount;
    memcpy(msg->s_payload, order, sizeof(TransferOrder));
    msg-> s_header.s_payload_len = sizeof(TransferOrder);
    send(parent_data, src, msg);
   while(msg->s_header.s_type!=ACK){
    receive(parent_data, dst, msg);
    }

}
int main(int argc, char * argv[])
{
    balance_t balance;
    int pid, i, fd_events;
    local_id lid = PARENT_ID;
    char * arg = argv[1];
    balance_t * st_balances;
    int x=0;
    AllHistory * all_hist;

    all_hist = (AllHistory *)malloc(sizeof(AllHistory));
    if(!strcmp(arg, "-p")){
    x = atoi(argv[2]);
   }

   if(x>MAX_PROCESS_ID){
       printf("Too large number of process");
       return EXIT_FAILURE;
   }
     all_hist->s_history_len = x;
    st_balances = (balance_t *) calloc(x, sizeof(balance_t));
      for(i = 3 ; i< 3+x; i++ ){
        st_balances[i-3]= atoi(argv[i]);
      }

       fd_events =open(events_log, O_RDWR|O_CREAT|O_APPEND, 0777);
       create_topology(x);
       for(i=1;i<=x;i++){
           balance = st_balances[i-1];
           pid = fork();
           if(!pid)
            break;
       }
       if(pid>0){
       io p_data;
       Message msg;
        close_desc(x, PARENT_ID);
        p_data.cnt=x;
        p_data.src= PARENT_ID;
        p_data.want_type = STARTED;
        receive_any(&p_data, &msg);

        bank_robbery(&p_data, x);

        msg=create_msg(MESSAGE_MAGIC, STOP, get_physical_time(),"", PARENT_ID, 0,0, 0);
        send_multicast(&p_data, &msg);

        p_data.want_type = DONE;
        receive_any(&p_data, &msg);
        p_data.want_type = BALANCE_HISTORY;

        for(i=1 ; i<=x; i++){
        msg.s_header.s_type=0;
        while(msg.s_header.s_type != BALANCE_HISTORY){
                receive(&p_data, i, &msg);

        }

            memcpy(&all_hist->s_history[i-1], msg.s_payload,sizeof(BalanceHistory));

        }
        for(i=0;i<x;i++){
          wait(0);
       }

      print_history(all_hist);
       }
        if(pid==0){
        int j;
        timestamp_t l_time =0;
        int dones=0;
        io ch_data;
        Message msg, rcv, answ;
        TransferOrder order;
        BalanceState state;
        BalanceHistory * hist;
        lid=i;
        int my_pid=getpid();
        int ppid = getppid();
        ch_data.cnt=x;
        ch_data.src = lid;
        state.s_balance_pending_in=0;
        state.s_time=get_physical_time();
        state.s_balance=balance;
        hist = (BalanceHistory*) malloc(sizeof(BalanceHistory));
        hist->s_id=lid;
        hist->s_history_len=1;
        hist->s_history[0]=state;
        close_desc(x, lid);
        msg = create_msg(MESSAGE_MAGIC, STARTED, get_physical_time(),log_started_fmt, lid, my_pid, ppid, state.s_balance);
        answ = create_msg(MESSAGE_MAGIC, ACK, get_physical_time(),"ACK", lid, my_pid, ppid, state.s_balance);
        send_multicast(&ch_data, &msg);
        write(fd_events,msg.s_payload, strlen(msg.s_payload));
        printf("%s", msg.s_payload);

        ch_data.want_type =0;
        receive_any(&ch_data, &rcv);

        msg = create_msg(MESSAGE_MAGIC, STARTED, get_physical_time(),log_received_all_started_fmt, lid, my_pid, ppid, state.s_balance);
        write(fd_events,msg.s_payload, strlen(msg.s_payload));
        printf(" %s ", msg.s_payload);

        while(rcv.s_header.s_type!=STOP){
            for(j = 0; j<=x ;j++){
                if(j != lid){
                    rcv.s_header.s_type=0;
                    receive(&ch_data, j, &rcv);
                    if(rcv.s_header.s_type==TRANSFER)
                        {
                            memcpy(&order, rcv.s_payload, sizeof(order));

                            if( lid == order.s_src)
                            {
                            balance =balance - order.s_amount;
                            state = set_state_and_write_hist(hist,rcv.s_header.s_local_time,  balance);
                            l_time =rcv.s_header.s_local_time;
                            send(&ch_data, order.s_dst, &rcv);

                            }
                            if( lid == order.s_dst)
                            {

                            balance = balance+order.s_amount;
                            state = set_state_and_write_hist(hist, rcv.s_header.s_local_time,  balance);

                            l_time=rcv.s_header.s_local_time;
                            send(&ch_data, 0, &answ);
                            }

                        }

                    if(rcv.s_header.s_type==STOP){
                    l_time =rcv.s_header.s_local_time;
                      break;
                    }
                    if(rcv.s_header.s_type==DONE){
                  dones ++;
                    }


                }
            }
        }
        validate_hist(hist, l_time, balance);
        msg = create_msg(MESSAGE_MAGIC, DONE, get_physical_time(),log_done_fmt, lid, my_pid, ppid, state.s_balance);
        send_multicast(&ch_data, &msg);

         ch_data.want_type = DONE;

         while(dones<x-1)
        for(j=1 ; j<=x;j++){
        if(j != lid){
        receive(&ch_data, j, &rcv);
            if(rcv.s_header.s_type == ch_data.want_type){
            dones++;

            }
            }
        }
        write(fd_events,msg.s_payload, strlen(msg.s_payload));
        printf("%s", msg.s_payload);

        msg.s_header.s_type=BALANCE_HISTORY;
        memcpy(msg.s_payload, hist, sizeof(BalanceHistory));
        msg.s_header.s_payload_len = sizeof(BalanceHistory);
        send(&ch_data, 0 , &msg);


        }
        if(pid<0){
        perror("");
        return EXIT_FAILURE;
        }
return EXIT_SUCCESS;
}

BalanceState set_state_and_write_hist(BalanceHistory * hist, timestamp_t time, balance_t balance){
BalanceState st;
st.s_balance = balance;
timestamp_t prev = hist->s_history[hist->s_history_len-1].s_time;
st.s_time =time+1;
st.s_balance_pending_in=0;
while(prev != st.s_time -1)
{
prev++;
BalanceState add;
add.s_balance_pending_in = 0;
add.s_time=prev;
add.s_balance =hist->s_history[hist->s_history_len-1].s_balance;
hist->s_history[hist->s_history_len] = add;
hist->s_history_len ++;
}

hist->s_history[hist->s_history_len] = st;
hist->s_history_len ++;
return st;
}
void validate_hist(BalanceHistory * hist, timestamp_t time, balance_t balance){
timestamp_t prev = hist->s_history[hist->s_history_len-1].s_time;
while(prev != time )
{
prev++;
BalanceState add;
add.s_balance_pending_in = 0;
add.s_time=prev;
add.s_balance =hist->s_history[hist->s_history_len-1].s_balance;
hist->s_history[hist->s_history_len] = add;
hist->s_history_len ++;
}


}
