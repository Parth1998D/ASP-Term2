#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>

void handle_client(int client){
	char pattern[20];
	read(client, pattern, 20);	  // receiving pattern from client
	send(client,"received",9,0);  // sending acknowledgement

	char content[5000];
	read(client, content, 5000);  // receiving file content from client
	send(client,"received",9,0);  // sending acknowledgement

  // saving file2 content in server
	char serverFileName[15];
	sprintf(serverFileName,"%d.txt", getpid());
	FILE *fptr = fopen(serverFileName,"w");
	fprintf(fptr,"%s",content);
	fclose(fptr);

	char clientFileName[15];  
	read(client, clientFileName, 15); // receiving file2 name from client

	char command[100];
	sprintf(command, "grep -w --color=always %s %s | sed 's/^/%s:/'", pattern, serverFileName, clientFileName);

	// redirecting output to client
  int save_out = dup(fileno(stdout));
  dup2(client, fileno(stdout));
	
	system(command);

	// resetting to standard output
  fflush(stdout); close(client);
  dup2(save_out, fileno(stdout));
  close(save_out);
}

int main(int argc, char *argv[]){  
  int sd, client, portNumber = 5555;
  struct sockaddr_in servAdd;  // server socket address

  sd = socket(AF_INET, SOCK_STREAM, 0);
  servAdd.sin_family = AF_INET;
  servAdd.sin_addr.s_addr = htonl(INADDR_ANY);
  servAdd.sin_port = htons((uint16_t)portNumber);
  
  bind(sd, (struct sockaddr *) &servAdd, sizeof(servAdd));
  listen(sd, 5);

  while(1){
    client = accept(sd, NULL, NULL);
    printf("Got a client\n");

    if(!fork())
      handle_client(client);

    close(client);
  }
}