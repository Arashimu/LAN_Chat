#include <iostream>
#include "ThreadPool.h"
#include "Utils.h"
#include <string.h>
#include <cstring>
#include <stdio.h>
#include <time.h>
#include <map>
#include <set>
#include "MyDB.h"
using namespace std;

struct Client{
    int fd;
    string username;
};
struct Msg{
    int type;
    char msg[1024];
};
vector<Client>onlineuser;
map<string,string> user;
map<int,string> f2u;
map<string,int>u2f;
set<int>fs;
char broadcasemsg[100];
MutexLock BroadLock;
MutexLock OnlineUserLock;
MutexLock UserLock;
MyDB db;
int Login(int &fd){
    char sendbuf[1024]={0};
    char username[1024]={0};
    char password[1024]={0};
    memset(sendbuf,0,sizeof(sendbuf));
    memset(username,0,sizeof(username));
    memset(password,0,sizeof(password));
    strcpy(sendbuf, "\033[31;5;1m==========Login Page==========\033[0m\n"
                    "please input your username: ");
    int len=send(fd,sendbuf,strlen(sendbuf),0);
    if(len<0){
        perror("send error");
        return -1;
    }
    len=recv(fd,username,sizeof(username),0);
    if(len<0){
        perror("recv username error");
        return -1;
    }

    memset(sendbuf,0,sizeof(sendbuf));
    strcpy(sendbuf,"please input your password: ");
    len=send(fd,sendbuf,strlen(sendbuf),0);
    if(len<0){
        perror("send error");
        return -1;
    }
    len=recv(fd,password,sizeof(password),0);
    if(len<0){
        perror("recv username error");
        return -1;
    }

    string usr(username);
    string psd(password);
    UserLock.Lock();
     if(!QueryUser(username,db)){
        UserLock.Unlock();
        return 1;
    }
    if(!QueryIsMatch(username,password,db)){
        UserLock.Unlock();
        return 2;
    }
    UserLock.Unlock();

    OnlineUserLock.Lock();
    onlineuser.push_back({fd,usr});
    f2u[fd]=usr;
    u2f[usr]=fd;
    OnlineUserLock.Unlock();
    return 0;
}
int Register(int &fd){
     char sendbuf[1024]={0};
     char username[1024]={0};
     char password[1024]={0};
     memset(sendbuf,0,sizeof(sendbuf));
     strcpy(sendbuf, "\033[31;5;1m=========Register Page=========\033[0m\n"
                     "please input your username: ");
    int len=send(fd,sendbuf,strlen(sendbuf),0);
    if(len<0){
        perror("send error");
        return -1;
    }
    len=recv(fd,username,sizeof(username),0);
    if(len<0){
        perror("recv username error");
        return -1;
    }

    memset(sendbuf,0,sizeof(sendbuf));
    strcpy(sendbuf,"please input your password: ");
    len=send(fd,sendbuf,strlen(sendbuf),0);
    if(len<0){
        perror("send error");
        return -1;
    }
    len=recv(fd,password,sizeof(password),0);
    if(len<0){
        perror("recv username error");
        return -1;
    }

    string usr(username);
    string psd(password);
    UserLock.Lock();
    if(QueryUser(username,db)){
        UserLock.Unlock();
        return 1;
    }
    InsertUser(username,password,db);
    UserLock.Unlock();
    return 0;

}

void BoradCast(int &fd,string &msg){
    char sendbuf[1024]={0};
    memset(sendbuf,0,sizeof(sendbuf));
    strcpy(sendbuf,msg.c_str());
    int len=0;
    for(auto &[f,u]:onlineuser){
        if(fd!=f){
        len=send(f,sendbuf,strlen(sendbuf),0);
        if(len<0){
            perror("broadcas send error");
            return;
        }
      }
    }
}

void ChatPage(int &fd){
    char recvbuf[1024]={0};
    char sendbuf[1024]={0};
    int len;
    pthread_t tid;
    // pthread_create(&tid,0,OnlineView,(void*)&fd);
    //OnlineView(fd);
    for(auto &[f,u]:onlineuser){
        if(f==fd) continue;
        string msg="online:"+u;
        cout<<msg<<"\n";
        memset(sendbuf,0,sizeof(sendbuf));
        strcpy(sendbuf,msg.c_str());
        len=send(fd,sendbuf,strlen(sendbuf),0);
        if(len<0){
            perror("Chat Page send error");
            continue;
        }
        sleep(1);
    }
    while(1){
        memset(recvbuf,0,sizeof(recvbuf));
        len=recv(fd,recvbuf,sizeof(recvbuf),0);
        if(len<0){
            perror("recv error");
            continue;
        }
        printf("%s\n",recvbuf);
        if(strchr(recvbuf,':')==nullptr){   //public chat
            if(strcmp(recvbuf,"exit")==0){
                printf("user:%s logout\n",f2u[fd].c_str());
                string msg="offline:"+f2u[fd];
                f2u.erase(f2u.find(fd));
                for(int i=0;i<(int)onlineuser.size();i++){
                    int f=onlineuser[i].fd;
                    if(f==fd){
                        onlineuser.erase(onlineuser.begin()+i);
                        break;
                    }
                }
                BoradCast(fd,msg);
                return;
            }
            //printf("%s\n",recvbuf);
            for(auto &[f,u]:onlineuser){
            if(f==fd) continue;
            else{
                memset(sendbuf,0,sizeof(sendbuf));
                strcpy(sendbuf,recvbuf);
                len=send(f,sendbuf,strlen(sendbuf),0);
                if(len<0){
                    fprintf(stderr,"send to %s error: ",u.c_str());
                    perror("");     
                    continue;
                }
             }
            }
        }else{                  
            char *msg=strchr(recvbuf,':')+1;
            char  *token;
            token=strtok(recvbuf," ");
            if(strcmp(token,"send")!=0){
                continue;
            }
            token=strtok(nullptr," ");
            if(strcmp(token,"to")!=0){
                continue;
            }
            token=strtok(nullptr," ");
            string usr(token);
            int tofd=u2f[usr];
            if(tofd==fd){
                continue;
            }
            string strmsg(msg);
            string finmsg="["+f2u[fd]+" send to you]>> "+strmsg;
            memset(sendbuf,0,sizeof(sendbuf));
            strcpy(sendbuf,(char *)finmsg.c_str());
            len=send(tofd,sendbuf,strlen(sendbuf),0);
            if(len<0){
                perror("send error");
                continue;
            }
        }
    }
}
void InitPage(int &fd){
    char recvbuf[1025]={0};
    char sendbuf[1025]={0};
    int len;
    while(1){
        memset(sendbuf,0,sizeof(sendbuf));
        strcpy(sendbuf,"\033[31;5;1m[system info:]\033[0m\n"
                       "choice:\n"
                       "1.login\n"
                       "2.register\n"
                       "3.exit");
        len=send(fd,sendbuf,strlen(sendbuf),0);
        if(len<0){
            perror("send error ");
            return;
        }
        memset(recvbuf,0,sizeof(recvbuf));
        len=recv(fd,recvbuf,sizeof(recvbuf),0);
        if(len<0){
            perror("recv error");
            return;
        }
        if(strcmp(recvbuf,"1")==0){
            memset(sendbuf,0,sizeof(sendbuf));
            switch(Login(fd)){
                case -1:
                    strcpy(sendbuf,"server send message error\n");
                    len=send(fd,sendbuf,strlen(sendbuf),0);
                    if(len<0){
                        perror("send error");
                        return;
                    }
                    break;
                case 1:
                    strcpy(sendbuf,"the user is not registered\n");
                    len=send(fd,sendbuf,strlen(sendbuf),0);
                    if(len<0){
                        perror("send error");
                        return;
                    }
                    break;
                case 2:
                    strcpy(sendbuf,"password error\n");
                    len=send(fd,sendbuf,strlen(sendbuf),0);
                    if(len<0){
                        perror("send error");
                        return;
                    }
                    break;
                case 0:
                    strcpy(sendbuf,"login success");
                    len=send(fd,sendbuf,strlen(sendbuf),0);
                    if(len<0){
                        perror("send error");
                        return;
                    }
                    string msg="online:"+f2u[fd];
                    BoradCast(fd,msg);
                    sleep(1);
                    ChatPage(fd);
                    return;
                    break;
            }
        }else if(strcmp(recvbuf,"2")==0){
            memset(sendbuf,0,sizeof(sendbuf));
            switch(Register(fd)){
                case -1:
                    strcpy(sendbuf,"server send message error\n");
                    len=send(fd,sendbuf,strlen(sendbuf),0);
                    if(len<0){
                        perror("send error");
                        return;
                    }
                    break;
                case 1:
                    strcpy(sendbuf,"the user already exists\n");
                    len=send(fd,sendbuf,strlen(sendbuf),0);
                    if(len<0){
                        perror("send error");
                        return;
                    }
                    break;
                case 0:
                    strcpy(sendbuf,"register success\n");
                    len=send(fd,sendbuf,strlen(sendbuf),0);
                    if(len<0){
                        perror("send error");
                        return;
                    }
                    break;

            }
        }else if(strcmp(recvbuf,"3")==0){
            return;
        }else{
            memset(sendbuf,0,sizeof(sendbuf));
            strcpy(sendbuf,"\033[31;5;1m please input valid choice\033[0m\n");
            len=send(fd,sendbuf,strlen(sendbuf),0);
            if(len<0){
                perror("send error");
                return;
            }
        }
    }   
}
void HandleConnect(void *arg){
    int fd=*(int*)arg;
    InitPage(fd);
    char buf[1024];
    memset(buf,0,sizeof(buf));
    strcpy(buf,"exit");
    int len=send(fd,buf,strlen(buf),0);
    if(len<0){
        perror("send error");
    }
    return;
    
}
int main(int argc,char **argv){
    if(argc!=2){
        fprintf(stderr,"usage: port\n");
        exit(1);
    }
    db.InitDB("localhost","root","","Users");
    int port=atoi(argv[1]);
    int listen_fd=-1;
    if((listen_fd=socket_bind_and_listen(port))<0){
        fprintf(stderr,"socket bind error!");
        exit(1);
    }
    printf("server start ....\n");
    ThreadPool threadpool(16);
    while(1){
        sockaddr_in client_addr;
        socklen_t client_addr_len=sizeof(client_addr);
        int client_fd;
        if((client_fd=accept(listen_fd,(sockaddr *)&client_addr,&client_addr_len))<0){
            fprintf(stderr,"client connect error!\n");
            continue;
        }else{
            char addr[1024]={0};
            
            printf("[%s:%d] connect\n",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
            //printf("client connect success\n");
        }
        fs.insert(client_fd);
        int *cur_fd_ptr=new int(client_fd);
        threadpool.AddTask(HandleConnect,cur_fd_ptr);
    }
    close(listen_fd);
}
