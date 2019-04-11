#ifndef FUNC_H_INCLUDED
#define FUNC_H_INCLUDED
#include "serv.h"
Message create_msg( int s_magic, int s_type, timestamp_t local_time ,const char * const buf, int src , int pid, int ppid, int money);
int create_topology(int num);
int close_desc(int num ,  local_id lid);
BalanceState set_state_and_write_hist(BalanceHistory * hist, timestamp_t time, balance_t balance);
void validate_hist(BalanceHistory * hist, timestamp_t time, balance_t balance);
#endif // FUNC_H_INCLUDED
