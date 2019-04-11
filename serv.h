
#ifndef SERV_H_INCLUDED
#define SERV_H_INCLUDED
#include "common.h"
#include "ipc.h"
#include "pa2345.h"
#include "banking.h"
#define NO_PPID -1
typedef struct io{
int src;
int cnt;
int want_type;
}io;


#endif // SERV_H_INCLUDED
