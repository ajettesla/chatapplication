#include <iostream>
#include <vector>
#include <stdlib.h>
#include <string>
#include <cstdlib>
#include <cstring>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <signal.h>
#include <unistd.h>
#include <cerrno>
#include <cstdio>
#include <pthread.h>
#include <unistd.h>
#include <algorithm>
#include <queue>


typedef struct clientDataStruct{
    int csockefd;
    std::string nickName;
    int ipstatus;
    struct sockaddr_in ipv4;
    struct sockaddr_in6 ipv6;
    int clientid;
}clientDataStruct;


typedef struct clientSendStruct{
  std::string message;
  int csocket;
}clientSendStruct;

/////////////////////////////////

#define MAXCLIENTCOUNT 50 

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
  for(int i = 0; i< MAXCLIENTCOUNT; i++){
    std::cout << clientFD[i] << " ";
  }
  std::cout << std::endl;
}
//////////////////////////////////////////
pthread_mutex_t vect;
pthread_mutex_t arr;
pthread_mutex_t qu;
pthread_cond_t cv;
///////////////////vector clientDataVector 
std::vector<clientDataStruct*> clientDataVector;

int getpostion(clientDataStruct *client){
   auto it = std::find(clientDataVector.begin(), clientDataVector.end(), client);
   int position = std::distance(clientDataVector.begin(), it);
   return position;
}

void removeclientVector(clientDataStruct *client){
  delete client;
  clientDataVector.erase(clientDataVector.begin() + getpostion(client));
  }


void addtoclienttoVector(clientDataStruct *client){
  clientDataVector.push_back(client);
}

clientDataStruct* getclientbysocket(int sock){
  for(int i=0; i<clientDataVector.size(); i++){
    if(clientDataVector[i]->csockefd == sock){
       return clientDataVector[i];
    }
  }
}

///end of vector of clientDataVector

std::queue<int> socketque;

void printIP(struct sockaddr_in *ipv4);

void printIP(struct sockaddr_in6 *ipv6);

void intialsetup(int socket_t);

void *commCallback(void *sock);

void *cleansocketfun(void *nothing);

std::vector<std::string> split(std::string sString,std::string delimiter);

int gsready(std::string &ip, int port,int* ipstatus);

void *sendCallback(void *nothing);

int clientid = 0;

std::queue <clientSendStruct*> queu;

int mastersocketfd;

int main(int argc, char *argv[]){

pthread_mutex_init(&vect, NULL);
pthread_mutex_init(&qu, NULL);
pthread_mutex_init(&arr, NULL);
pthread_cond_init(&cv, NULL);

 ///intial 
 std::string delimiter = ":";

 std::vector<std::string> outputString = split(argv[1],":"); 

 std::string ipString = "";

 int port;

 if(outputString.size() > 2){
  port = atoi(outputString[outputString.size()-1].c_str());
  for(int i=0; i < 8 ; i++){
   ipString = ipString + outputString[i];}}
else{
   port = atoi(outputString[1].c_str());
   ipString = outputString[0];}

int *ipstatus = new int;

struct sockaddr_in clientAddr;
socklen_t clientlen = sizeof(clientAddr);

struct sockaddr_in6 clientAddr6;
socklen_t clientlen6 = sizeof(clientAddr6);
///end of intial
clearfd();
mastersocketfd = gsready(ipString ,port, ipstatus);
setclientfd(mastersocketfd);

pthread_t sendhandel;
pthread_create(&sendhandel, NULL, sendCallback, NULL);


while(1){
    pthread_mutex_lock(&arr);
    re_inti_readfd();
    pthread_mutex_unlock(&arr);
    select(getmax() + 1, &readfds, NULL, NULL, NULL);
    if(FD_ISSET(mastersocketfd, &readfds)){
      int *csocketfd = new int;
      if(*ipstatus == 1){
      *csocketfd = accept(mastersocketfd, (struct sockaddr*)&clientAddr, &clientlen);}
      else if(*ipstatus == 2){
      *csocketfd = accept(mastersocketfd, (struct sockaddr*)&clientAddr6, &clientlen6);
      }
      if(*csocketfd < 0){perror("error with accept system call");exit(1);}
      clientDataStruct *client = new clientDataStruct;
      client->clientid = clientid;
      client->csockefd = *csocketfd;
      if(*ipstatus == 1){client->ipstatus = 1; client->ipv4 = clientAddr;}
      if(*ipstatus == 2){client->ipstatus = 2; client->ipv6 = clientAddr6;}

      pthread_mutex_lock(&arr);
      setclientfd(*csocketfd);
      pthread_mutex_unlock(&arr);

      pthread_mutex_lock(&vect);
      clientDataVector.push_back(client);
      pthread_mutex_unlock(&vect);
      intialsetup(*csocketfd);    
      delete csocketfd;

      }
      else{
       for(int i=0; i< getmax() +1 ; i++){
        pthread_mutex_lock(&arr);
        if(FD_ISSET(clientFD[i], &readfds)){
          int *sock = new int;
          *sock = clientFD[i];
          pthread_t communicationhandel;
          pthread_create(&communicationhandel, NULL, commCallback, (void*)sock);
        }
        pthread_mutex_unlock(&arr);
       }
      }

}


  delete ipstatus;
  return 0;
}

void *commCallback(void *sock){
    int soc = *(int*)sock;
    delete sock;
    clientDataStruct *client;
    pthread_mutex_lock(&vect);
    for(int i=0; i< clientDataVector.size(); i++){
      if(clientDataVector[i]->csockefd == soc){
        client = clientDataVector[i];
        break;
      }
    }
    pthread_mutex_unlock(&vect);
    std::string nickName = client->nickName;
    char recvbuffer[300];
    int sent_recv_bytes = recv(soc, recvbuffer, sizeof(recvbuffer),0);
    if(sent_recv_bytes < 0){perror("error with recv function 1");exit(1);}
    if(strncmp(recvbuffer,"\nTERMINATE1.0",13) == 0){
    std::cout << "TERMINATE IS RECEIVED\n";
    char ok[] = "ok";
    int sent_recv_bytes = send(soc, ok,sizeof(ok),0);
    if(sent_recv_bytes < 0){perror("error with send");exit(1);}
    removeclientfd(soc);
    socketque.push(soc);
    clientid--;
    sleep(1);
    close(soc);
    pthread_exit(0);}
    std::string temp(recvbuffer);
    clientSendStruct *data =  new clientSendStruct;
    temp = nickName + ": " + temp;
    data->message = temp;
    data->csocket = soc;
    if(clientid > 1){
    pthread_mutex_lock(&qu);
    queu.push(data);
    pthread_cond_signal(&cv);
    pthread_mutex_unlock(&qu);}
  return nullptr;
}

void intialsetup(int socket_t){
  char testbuffer[] = "HELLO 1.0\n";
  int sent_recv_bytes = send(socket_t,testbuffer, sizeof(testbuffer), 0);
  if(sent_recv_bytes < 0){perror("error with send fucntion 1");exit(1);}
  char recvNickname[17];

  int sent_recv_byte = recv(socket_t, recvNickname,sizeof(recvNickname),0);
  if(sent_recv_byte < 0){perror("error with recv function 2");exit(1);}
  if(sent_recv_byte > 15){
    char errormes[] = "ERROR\n";
    sent_recv_bytes = send(socket_t, errormes, sizeof(errormes),0);
    if(sent_recv_bytes < 0){perror("error with send ");exit(1); }}
  else{
      char okmes[] = "OK\n";
      sent_recv_bytes = send(socket_t, okmes, sizeof(okmes),0);
       if(sent_recv_bytes < 0){perror("error with send ");exit(1); }}
  recvNickname[sent_recv_byte -1] = '\0';

  std::string temp(recvNickname);
  clientDataStruct *client;
  pthread_mutex_lock(&vect);
  for(int i=0; i< clientDataVector.size(); i++){
    if(clientDataVector[i]->csockefd == socket_t){
      client = clientDataVector[i];
      break;
    }
  }client->nickName = temp;
  pthread_mutex_unlock(&vect);  
  clientid++;

}


void *sendCallback(void *nothing){
    while(1){
      if(clientid > 1){
    pthread_mutex_lock(&qu);
    if(queu.empty()){
        pthread_cond_wait(&cv,&qu);}//end of if l
    if(!queu.empty()){
       for(int i=0; i < getmax()+1; i++){
        if(clientFD[i] != -1){
          if(clientFD[i] != queu.front()->csocket){
            if(clientFD[i] != mastersocketfd){
            int sent_recv_bytes = send(clientFD[i],queu.front()->message.c_str(),queu.front()->message.length(), 0);
            if(sent_recv_bytes < 0){perror("error with send function");exit(1);}}
          }
        }
       }
      delete queu.front();
      queu.pop();   
    pthread_mutex_unlock(&qu);
    }
   //end of else loop
    }//end of clientID if loop
 
   }//end of while loop
  
return nullptr;
}


void printIP(struct sockaddr_in *ipv4){
  char ipv4buffer[INET_ADDRSTRLEN];
  const char *result = inet_ntop(AF_INET,&(ipv4->sin_addr),ipv4buffer,sizeof(ipv4buffer));
  if(result == nullptr){
    perror("error with inet_ntop");}
    std::cout << ipv4buffer << " port " << ipv4->sin_family;
    }


void printIP(struct sockaddr_in6 *ipv6){
  char ipv6buffer[INET6_ADDRSTRLEN];
  const char *result = inet_ntop(AF_INET,&(ipv6->sin6_addr),ipv6buffer,sizeof(ipv6buffer));
  if(result == nullptr){
    perror("error with inet_ntop");}
    std::cout << ipv6buffer << " port " << ipv6->sin6_port;
}

void *cleansocketfun(void *nothing){
  while(1){
    sleep(2);
    if(!socketque.empty()){
      close(socketque.front());
    }
  }
}


int gsready(std::string &ip, int port,int* ipstatus){

int socketfd; 
struct sockaddr_in ipv4;
struct sockaddr_in6 ipv6;
struct addrinfo hint, *output, *temp;
memset(&hint, 0, sizeof(hint));
hint.ai_family = AF_UNSPEC;
hint.ai_socktype = SOCK_STREAM;
int status = getaddrinfo(ip.c_str(), NULL, &hint, &output);
if(status != 0){
std::cout << "There is problem in getting getaddrinfo" << std::endl;

}

for(temp=output; temp != NULL;temp->ai_addr){

if(temp->ai_family == AF_INET){
ipv4.sin_family = AF_INET;
ipv4.sin_port = htons(port);
ipv4.sin_addr.s_addr = ((struct sockaddr_in*)temp->ai_addr)->sin_addr.s_addr;
socketfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
if(socketfd > 0){
  if(bind(socketfd,(struct sockaddr*)&ipv4,sizeof(struct sockaddr)) < 0){perror("error with binding the ip address");exit(1);}
  *ipstatus = 1;
   break;
}}
                                              
else if(temp->ai_family == AF_INET6){
ipv6.sin6_family = AF_INET6;
ipv6.sin6_port = htons(port);
ipv6.sin6_addr = ((struct sockaddr_in6*)temp->ai_addr)->sin6_addr;
socketfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
if(socketfd > 0){
  if(bind(socketfd,(struct sockaddr*)&ipv6,sizeof(struct sockaddr_in6)) < 0){perror("error with binding the ip address");exit(1);}
  *ipstatus = 2;
   break;
}}}

if(*ipstatus != 1 && *ipstatus != 2){
  perror("error with socket");
  exit(1);}

if(listen(socketfd, 5 < 0)){perror("error with listen function");exit(1);}
  
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
