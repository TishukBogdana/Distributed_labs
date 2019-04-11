#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include "serv.h"
#include <string.h>
extern int pipe_tab[MAX_PROCESS_ID+1][MAX_PROCESS_ID+1][2];

int create_topology(int num){
int i, j;
char buf[255];
int fd_log_pipe = open(pipes_log, O_RDWR|O_CREAT|O_APPEND, 0777);
for(i=0; i<=num; i++){
    for(j=0; j<=num; j++){
        if(i != j){
        if(pipe(pipe_tab[i][j])==-1){
        perror("");
        return EXIT_FAILURE;
        }

        fcntl(pipe_tab[i][j][0], F_SETFL, O_NONBLOCK);
        fcntl(pipe_tab[i][j][1], F_SETFL, O_NONBLOCK);

      sprintf(buf, "connect %d with %d \n ", i, j);
      write(fd_log_pipe, buf, strlen(buf));

        }
    }
}
return EXIT_SUCCESS;
}


int close_desc(int num , local_id lid){
int i, j;
for(i=0; i<=num; i++){
    for(j=0; j<=num; j++){
        if(i !=j){
           if( i== lid){
            if(close(pipe_tab[i][j][0])==-1)
            {

            perror("0-");
            return EXIT_FAILURE;
            }
           }

           if( j== lid){
           if(close(pipe_tab[i][j][1])==-1)
            {
            perror("1-");
            return EXIT_FAILURE;
            }
           }
           if((i != lid)&&(j!= lid)){
                if(close(pipe_tab[i][j][0])==-1)
                {
                perror("0-");
                return EXIT_FAILURE;
                }
                if(close(pipe_tab[i][j][1])==-1)
                {
                perror("1-");
                return EXIT_FAILURE;
                }
                }
           }
        }
    }


return EXIT_SUCCESS;
}
