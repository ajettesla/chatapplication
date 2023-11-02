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
#include <regex>


int sockfd;
#define MAXCLIENTCOUNT 3
fd_set readfds;
int clientFD[MAXCLIENTCOUNT];

void clearfd(){
    FD_ZERO(&readfds);
    for(int i=0; i< MAXCLIENTCOUNT;i++){
        clientFD[i] = -1;
    }
}

int getmax(){
    int max = 0;
    for(int i=0; i < MAXCLIENTCOUNT; i++){
        if(clientFD[i] > max){
            max = clientFD[i];
        }
    }
    return max;
}

void re_inti_readfd(){
    FD_ZERO(&readfds);
    for(int i=0; i < MAXCLIENTCOUNT; i++){
        if(clientFD[i] != -1){
        FD_SET(clientFD[i], &readfds);}
    }
}

void setclientfd(int socket){
    for(int i=0; i < MAXCLIENTCOUNT; i++){
        if(clientFD[i] == -1){
            clientFD[i] = socket;
            break;
        }
        
    }
}

void removeclientfd(int socket){
    for(int i =0; i < MAXCLIENTCOUNT; i++){
        if(clientFD[i] == socket){
            clientFD[i] = -1;
        }
    }
}

void printclientfd(){
    for(int i=0;i< MAXCLIENTCOUNT; i++){
        if(clientFD[i] != -1){
            std::cout << clientFD[i] << std::endl;
        }
    }
}

int gsready(std::string &ip, int port,int* ipstatus, char* buffer);

std::vector<std::string> split(std::string sString,std::string delimiter);

void *inputCallback(void *soc);

 void closeconnection(int signal){
     std::string terminate = "\nTERMINATE1.0";
     int sent_recv_bytes = send(sockfd, terminate.c_str(), terminate.length(),0);
     if(sent_recv_bytes < 0){perror("error with send");fflush(stderr);exit(1);}
     close(sockfd);
     exit(0);
 }


int main(int argc, char* argv[]){
 std::string delimiter = ":";


 if(argc < 3){
    std::cout << "ERROR" << std::endl;
    exit(1);
 }

 std::vector<std::string> outputString = split(argv[1],":"); 

if(outputString[1].empty()){
        std::cout << "ERROR" << std::endl;
        exit(1);
    }

 std::string ipString = "";

 int port;

 char nickName[13];

if(strlen(argv[2]) > 13){  std::cout << "ERROR" << std::endl;
    exit(1); }
else{
    strcpy(nickName, argv[2]);
    std::regex pattern("^[a-zA-Z0-9]{1,13}$"); 
    bool val = std::regex_search(nickName, pattern);
    if(val == 0){
        std::cout << "ERROR" << std::endl;
        exit(1);
    }

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
clearfd();
setclientfd(sockfd);
setclientfd(STDIN_FILENO);
signal(SIGINT,closeconnection);
re_inti_readfd();

while(1){
re_inti_readfd();
int rc = select(getmax() + 1,&readfds, NULL, NULL, NULL);
if(rc < 0){perror("error with select system call"); fflush(stderr);exit(1);}

if(FD_ISSET(sockfd, &readfds)){
    char recvbuffer[300];
    memset(recvbuffer,0,sizeof(recvbuffer));
    int sent_recv_bytes = recv(sockfd, recvbuffer,sizeof(recvbuffer),0);
    if(sent_recv_bytes < 0){perror("error with recv");fflush(stderr);exit(1);}
    std::string temp(recvbuffer);
    if(temp.substr(0,3) == "MSG"){
      std::vector<std::string> st = split(temp," ");
     // if(st[1] != nickName){
        char lastchar = temp.back();
        if(lastchar == '\n'){
        std::cout << temp.substr(4);}
        else{
            std::cout << "ERROR " << std::endl;
        }
        fflush(stdout);
      //}

    }
    else{
      char error[] = "ERROR\n";
      int sent_recv_bytes = send(sockfd, error, sizeof(error),0);
      if(sent_recv_bytes <0){std::cout << "error with send " << std::endl;fflush(stderr);exit(1);}}

    fflush(stdout);}
else if(FD_ISSET(STDIN_FILENO, &readfds)){
                char input[300];
                memset(input,0,sizeof(input));
                if (fgets(input, sizeof(input), stdin) != nullptr) {
                      std::string sendMessage(input);
                      if(sendMessage.length() < 225){
                      sendMessage = "MSG " + sendMessage;
                      int sent_recv_bytes = send(sockfd,sendMessage.c_str(), sendMessage.length(),0);
                      if(sent_recv_bytes < 0){perror("error with send"); fflush(stderr);exit(1);}}
                      else{
                        std::cout << "ERROR" << std::endl;
                        fflush(stdout);
                      }} }}

return 0;
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
   if(connect(socketfd, (struct sockaddr*)&ipv4, sizeof(struct sockaddr)) < 0){perror("error with connect");close(socketfd);fflush(stderr);exit(1);}
    std::cout << "Connected to " << ip << ":" << port << std::endl;
  *ipstatus = 1;
   break;
}}
                                              
else if(temp->ai_family == AF_INET6){
ipv6.sin6_family = AF_INET6;
ipv6.sin6_port = htons(port);
ipv6.sin6_addr = ((struct sockaddr_in6*)temp->ai_addr)->sin6_addr;
socketfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
if(socketfd > 0){
   if(connect(socketfd, (struct sockaddr*)&ipv6, sizeof(struct sockaddr_in6)) < 0){perror("error with connect");close(socketfd);fflush(stderr);exit(1);}
   std::cout << "Connected to " << ip << ":" << port << std::endl;
  *ipstatus = 2;
   break;
}}}

if(*ipstatus != 1 && *ipstatus != 2){perror("error with socket");close(socketfd);fflush(stderr);exit(1);}

char testmessage[11];
memset(testmessage, 0, sizeof(testmessage));
int sent_recv_bytes = recv(socketfd, testmessage, sizeof(testmessage),0);
if(sent_recv_bytes < 0){perror("error with recv function"); close(socketfd);fflush(stderr);exit(1);}
if(strncmp(testmessage, "HELLO 1\n",8) == 0){
    std::cout << "Server protocol HELLO 1" << std::endl;
    std::cout << "protocols supported, sending Nickname" << std::endl;
    fflush(stdout);
    char nickName[18] = "NICK ";
    strcat(nickName,buffer);
    sent_recv_bytes = send(socketfd, nickName, strlen(nickName),0);
    if(sent_recv_bytes < 0){perror("error with send function");fflush(stderr);exit(1);}
    char revcheck[7];
    memset(revcheck, 0, sizeof(revcheck));
    sent_recv_bytes = recv(socketfd, revcheck,sizeof(revcheck), 0);
    if(sent_recv_bytes < 0){perror("error with recv function"); close(socketfd);fflush(stderr);exit(1);}
    if(strncmp(revcheck,"OK\n",3) !=0){
        std::cout << "ERROR" << std::endl;exit(1);}
    else{
        std::cout << "Name accepted!" << std::endl;
    }

}
else{
    std::cout << "ERROR" << std::endl;
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

  if(nString.size() < 2){
    std::cout << "ERROR" << std::endl;
    exit(1);
  }



return nString;
}