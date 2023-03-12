#include "Utils.h"

int socket_bind_and_listen(int port){
    int listen_fd=0;
    if((listen_fd=socket(AF_INET,SOCK_STREAM,0))==-1){
        return -1;
    }
    sockaddr_in server_addr;
    memset(&server_addr,'\0',sizeof(server_addr));

    server_addr.sin_family=AF_INET;
    server_addr.sin_port=htons((unsigned short)port);
    server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    //inet_aton("127.0.0.1",&server_addr.sin_addr);
    if(bind(listen_fd,(sockaddr *)&server_addr,sizeof(server_addr))==-1){
        return -1;
    }
    if(listen(listen_fd,1024)==-1){
        return -1;
    }
    return listen_fd;
}
bool QueryUser(char* username,MyDB &db){
    char sql[100];
    sprintf(sql,"Select * from user where username='%s';",username);
    printf("%s\n",sql);
    string cmd(sql);
    if(db.ExeSql(cmd)) return true;
    else return false;
}
bool QueryIsMatch(char *username,char *password,MyDB &db){
    char sql[100];
    sprintf(sql,"Select * from user where username='%s' and password='%s';",username,password);
    printf("%s\n",sql);
    string cmd(sql);
    if(db.ExeSql(cmd)) return true;
    else return false;
}
bool InsertUser(char *username,char *password,MyDB &db){
    char sql[50];
    sprintf(sql,"Insert user values('%s','%s');",username,password);
    printf("%s\n",sql);
    string cmd(sql);
    if(db.ExeSql(cmd)) return true;
    else return false;
}
