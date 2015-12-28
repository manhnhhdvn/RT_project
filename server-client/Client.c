#include<stdio.h>
#include<netinet/in.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<ctype.h>
#include<string.h>
#include<stdlib.h>
#define MAX_SIZE 2000
int main(){
  struct sockaddr_in server;
  int sockfd;
  char *sendMes=malloc(MAX_SIZE);
  char *recvMes=malloc(MAX_SIZE);
  strcpy(sendMes,"");
  strcpy(recvMes,"");
  int countTotalRecvData=0, countTotalSendData=0, countRecvData=0, countSendData=0;
  printf("Creat a socket, please wait...\n");
  if((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1){
    printf("Socket retrieve failed!\n");
    return 1;
  }else printf("Socket retrieve success!...\n");
  memset(&server,'0',sizeof(server));
  server.sin_family=AF_INET;
  server.sin_addr.s_addr=inet_addr("192.168.0.115");
  server.sin_port=htons(5678);
  if(connect(sockfd,(struct sockaddr *)&server, sizeof(server))==-1){
    printf("Connect failed!\n");
    return 1;
  }else printf("Connected. Ready to communicate with server!\n");
  while(1){
    sendMes=malloc(MAX_SIZE);
    recvMes=malloc(MAX_SIZE);
    strcpy(sendMes,"");
    strcpy(recvMes,"");
    printf("Write anything: ");
    //scanf("%c",&blank);
    gets(sendMes);
    //scanf("%s",sendMes);
    // printf("Your write: %s\n",sendMes);
    countSendData=send(sockfd,sendMes,strlen(sendMes),0);
    if(countSendData==-1){
      printf("Send data failed!\n");
      return 1;
    }else if((strcmp(sendMes,"q")==0)||(strcmp(sendMes,"Q")==0)){
      sendMes=malloc(MAX_SIZE);
      strcpy(sendMes,"");
      printf("Disconnected!\n");
      printf("Total size of data sent: %d\n",countTotalSendData);
      printf("Total size of data received: %d\n",countTotalRecvData);
      break;
    }else{
      countTotalSendData=countTotalSendData+countSendData;
      countRecvData=recv(sockfd,recvMes,2000,0);
      if(countRecvData==-1){
	printf("Receive data failed!\n");
	return 1;
      }else{
	countTotalRecvData=countTotalRecvData+countRecvData;
	printf("Server reply: %s\n",recvMes);
	recvMes=malloc(MAX_SIZE);
	strcpy(recvMes,"");
      }
    }
  }
  close(sockfd);
  return 0;
}
