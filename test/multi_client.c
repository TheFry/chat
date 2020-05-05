#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#define NUM_CLIENTS 9
int main(int argc, char**argv){
   int i;
   pid_t process;
   char buff[100] = "";
   char *port = argv[2];
   int num_clients = atoi(argv[1]);


   for(i = 0; i < num_clients; i++){
      process = fork();
      if(!process){
         sprintf(buff, "luke %d", i);
         execl("/home/luke/chat/cclient", "cclient", buff, "localhost", port, (char *)NULL);
         perror("exec");
      }
   }

}