#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

// to check if file exists or not
void fileExists(char *file){
  if(access(file, F_OK )==-1){
    printf("%s not found\n",file);
    exit(0);
  }
}

int main(int argc, char *argv[]){
  int server, portNumber = 5555;
  char ip[30]="127.0.0.1";
  struct sockaddr_in servAdd;     // server socket address
  
  if(argc != 4){
    printf("Call model: %s <pattern> <file1> <file2>\n", argv[0]);
    exit(0);
  }

  char *pattern=argv[1];
  char *file1=argv[2];
  char *file2=argv[3];

  fileExists(file1);
  fileExists(file2);

  if ((server = socket(AF_INET, SOCK_STREAM, 0)) < 0){
     fprintf(stderr, "Cannot create socket\n");
     exit(1);
  }

  servAdd.sin_family = AF_INET;
  servAdd.sin_port = htons((uint16_t)portNumber);

  if(inet_pton(AF_INET, ip, &servAdd.sin_addr) < 0){
    fprintf(stderr, " inet_pton() has failed\n");
    exit(2);
  }

  if(connect(server, (struct sockaddr *) &servAdd, sizeof(servAdd))<0){
    fprintf(stderr, "connect() has failed, exiting\n");
    exit(3);
 }

  // read content from file2 to send it to server
  char * content = 0;
  long length;
  FILE * f = fopen(file2, "rt");
  if(f)
  {
    fseek (f, 0, SEEK_END);
    length = ftell (f);
    fseek (f, 0, SEEK_SET);
    content = malloc (length);
    if (content)
      fread (content, 1, length, f);
    fclose (f);
  }

  char acknowledgement[9];

  send(server, pattern, strlen(pattern)+1, 0);  // sending pattern to server
  read(server, acknowledgement, 9);             // receiving acknowledgement from server
  send(server, content, length+1, 0);           // sending content of file2 to server
  read(server, acknowledgement, 9);             // receiving acknowledgement from server
  send(server, file2, strlen(file2)+1, 0);      // sending filename of file2 to server

  // running grep on file1 locally
  char command[100];
  sprintf(command, "grep -w --color=always %s %s | sed 's/^/%s:/'", pattern, file1, file1);
  system(command);
	
  //receivig output from server
  char buffer[5000];
	read(server, buffer, 5000);
  fprintf(stderr,"%s",buffer);

  close(server);
}