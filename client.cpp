#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <cstring>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cerrno>
#include <signal.h>
#include <cstdio>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>


int sockfd;

int gsready(std::string &ip, int port,int* ipstatus, char* buffer);

std::vector<std::string> split(std::string sString,std::string delimiter);

void *inputCallback(void *soc);

 void closeconnection(int signal){
     std::string terminate = "\nTERMINATE1.0";
     int sent_recv_bytes = send(sockfd, terminate.c_str(), terminate.length(),0);
     if(sent_recv_bytes < 0){perror("error with send");exit(1);}
     close(sockfd);
     exit(0);
 }


int main(int argc, char* argv[]){

 std::string delimiter = ":";

 std::vector<std::string> outputString = split(argv[1],":"); 

 std::string ipString = "";

 int port;

 char nickName[13];


if(strlen(argv[2]) > 13){  std::cout << "Please enter nick name less than 13 characters" << std::endl;
    exit(1); }
else{
    strcpy(nickName, argv[2]);
}

 if(outputString.size() > 2){
  port = atoi(outputString[outputString.size()-1].c_str());
  for(int i=0; i < 8 ; i++){
   ipString = ipString + outputString[i];}}
else{
   port = atoi(outputString[1].c_str());
   ipString = outputString[0];}

int *ipstatus = new int;


sockfd = gsready(ipString, port, ipstatus, nickName);

signal(SIGINT,closeconnection);

int *soc = new int;
*soc = sockfd;
pthread_t getinput;
pthread_create(&getinput, NULL, inputCallback, (void*)soc);

while(1){
    char recvbuffer[300];
    memset(recvbuffer,0,sizeof(recvbuffer));
    int sent_recv_bytes = recv(sockfd, recvbuffer,sizeof(recvbuffer),0);
    if(sent_recv_bytes < 0){perror("error with recv");exit(1);}
    std::string temp(recvbuffer);
    //temp = temp.substr(1,sent_recv_bytes - 4);
    std::cout << temp << std::endl;

}

return 0;
}


void *inputCallback(void *soc){
    while(1){
    int socketfd = *(int*)soc;
    std::string sendMessage;
    std::getline(std::cin,sendMessage);
    char newline[] = "\n";
    char version[] = "1.0";

    if(sendMessage.length() > 255){
        std::cout << "length of message must be less than 255 characters" << std::endl;
        exit(1);}
    else{
        //sendMessage = "\n" + sendMessage + "1.0";
        int sent_recv_bytes = send(socketfd,sendMessage.c_str(), sendMessage.length(),0);
        if(sent_recv_bytes < 0){perror("error with send"); exit(1);}
    }}
}

int gsready(std::string &ip, int port,int* ipstatus, char* buffer){
int socketfd; 
struct sockaddr_in ipv4;
struct sockaddr_in6 ipv6;
struct addrinfo hint, *output, *temp;
memset(&hint, 0, sizeof(hint));
hint.ai_family = AF_UNSPEC;
hint.ai_socktype = SOCK_STREAM;
int status = getaddrinfo(ip.c_str(), NULL, &hint, &output);
if(status != 0){
std::cout << "There is problem in getting getaddrinfo" << std::endl;}

for(temp=output; temp != NULL;temp->ai_addr){

if(temp->ai_family == AF_INET){
ipv4.sin_family = AF_INET;
ipv4.sin_port = htons(port);
ipv4.sin_addr.s_addr = ((struct sockaddr_in*)temp->ai_addr)->sin_addr.s_addr;
socketfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
if(socketfd > 0){
   if(connect(socketfd, (struct sockaddr*)&ipv4, sizeof(struct sockaddr)) < 0){perror("error with connect");close(socketfd);exit(1);}
  *ipstatus = 1;
   break;
}}
                                              
else if(temp->ai_family == AF_INET6){
ipv6.sin6_family = AF_INET6;
ipv6.sin6_port = htons(port);
ipv6.sin6_addr = ((struct sockaddr_in6*)temp->ai_addr)->sin6_addr;
socketfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
if(socketfd > 0){
   if(connect(socketfd, (struct sockaddr*)&ipv6, sizeof(struct sockaddr_in6)) < 0){perror("error with connect");close(socketfd);exit(1);}
  *ipstatus = 2;
   break;
}}}

if(*ipstatus != 1 && *ipstatus != 2){perror("error with socket");close(socketfd);exit(1);}
char testmessage[11];

int sent_recv_bytes = recv(socketfd, testmessage, sizeof(testmessage),0);
if(sent_recv_bytes < 0){perror("error with recv function"); close(socketfd);exit(1);}

if(strncmp(testmessage, "HELLO 1.0\n",10) == 0){
    sent_recv_bytes = send(socketfd, buffer, sizeof(buffer),0);
    if(sent_recv_bytes < 0){perror("error with send function");exit(1);}
    char revcheck[7];
    sent_recv_bytes = recv(socketfd, revcheck,sizeof(revcheck), 0);
    if(sent_recv_bytes < 0){perror("error with recv function"); close(socketfd);exit(1);}
    std::string rev(revcheck);
    if(rev == "ERROR\n"){std::cout << "INVALID NICK NAME\n" << std::endl;exit(1);}
    if(rev == "OK\n"){}

}
else{
    std::cout << "ERROR " << std::endl;
    exit(1);
}
  
freeaddrinfo(output);
return socketfd;
}

std::vector<std::string> split(std::string sString,std::string delimiter){

std::vector<std::string> nString;
std::string temp;

for(int i=0; i < static_cast<int>(sString.length());i++){
  int  count = 0;
  if(sString[i] == delimiter[0]){
        count++;
        nString.push_back(temp);
        temp  = "";
    }
  else{
        temp = temp +  sString[i];
         }

  if(count==0 && (i == static_cast<int>(sString.length()-1))){
         nString.push_back(temp);}               }



return nString;
}