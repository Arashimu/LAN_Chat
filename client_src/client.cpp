

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <cstring>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <functional>
#include <signal.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <ncurses.h>
#include <vector>
using namespace std;

int connfd;
fd_set fds;
int maxfd;

WINDOW *MainWin;
WINDOW *InitWin;
WINDOW *LoginWin,*RegisterMain;
WINDOW *MsgWinBox,*InputWinBox,*StatusWinBox;
WINDOW *MsgWin,*InputWin,*StatusWin;

vector<string> st;
void _add(string s){
    st.push_back(s);
}
void _delete(string s){
    for(int i=0;i<(int)st.size();i++){
        if(st[i]==s){
            st.erase(st.begin()+i);
            break;
        }
    }
}
void _print(){
    wclear(StatusWin);
    wrefresh(StatusWin);
    mvwprintw(StatusWin,1,1,"online:\n");
    int ha=2;
    for(auto s:st){
        char buf[100]={0};
        sprintf(buf,"%s",s.c_str());
        mvwprintw(StatusWin,ha,1,"%s\n",buf);
        ha++;
    }
    wrefresh(StatusWin);
}
int judjestrthe(char *sended)
{
    int a=0;
    string adda(sended);
    while(sended[a]!=':')
    {
        a++;
    }
    if(a==6)
    {
        string addb(adda,a+1,strlen(sended));
        _add(addb);
    }
    if(a==7)
    {
        string addb(adda,a+1,strlen(sended));
        _delete(addb);
    }
    _print();
    return a;
}
void InitSrc(){
    MainWin=initscr();
    head=creatlist();
    cbreak();
    InitWin=subwin(MainWin,10,60,0,0);
    LoginWin=subwin(MainWin,5,100,0,0);
    RegisterMain=subwin(MainWin,5,100,0,0);

    MsgWinBox=subwin(MainWin,15,60,0,24);
    MsgWin=subwin(MsgWinBox,13,58,1,25);

    InputWinBox=subwin(MainWin,5,60,15,24);
    InputWin=subwin(InputWinBox,3,45,16,29);

    StatusWinBox=subwin(MainWin,20,23,0,0);
    StatusWin=subwin(StatusWinBox,18,21,1,1);
    scrollok(MsgWin,true);
    scrollok(StatusWin,true);
    keypad(MainWin,TRUE);

    start_color();
    use_default_colors();
    init_pair(1, -1, -1);
    init_pair(2, COLOR_RED, -1);
    refresh();

}
void MainPage(){
    InitSrc();
    clear();
    refresh();
    char cmd[1024]={0};
    char buf[1025]={0};
    box(StatusWinBox,0,0);
    box(InputWinBox,0,0);
    box(MsgWinBox,0,0);
    
    wattron(MsgWinBox, COLOR_PAIR(2));
    mvwaddstr(MsgWinBox, 0, 22, " Message  Page");
    wattroff(MsgWinBox, COLOR_PAIR(2));
    mvwprintw(StatusWin,1,1,"online:\n");
    mvwprintw(InputWinBox,1,1,">>");
    mvwprintw(InputWin,0,1,"");
    wrefresh(StatusWinBox);
    wrefresh(StatusWin);
    wrefresh(InputWinBox);
    wrefresh(MsgWinBox);
    wrefresh(InputWin);
    do{
        memset(cmd,0,sizeof(cmd));
        FD_ZERO(&fds);
        FD_SET(0,&fds);
        maxfd=0;
        FD_SET(connfd,&fds);
        if(connfd>maxfd) maxfd=connfd;
        timeval time;
        time.tv_sec=1;
        time.tv_usec=0;
        // mvwprintw(InputWin,1,1,"in put msg: ");
        // wrefresh(InputWin);
        int err=select(maxfd+1,&fds,nullptr,nullptr,&time);
        if(err==-1){
            perror("select error");
            return;
        }else if(err==0){
            continue;
        }else{
            if(FD_ISSET(connfd,&fds)){
                bzero(buf,1025);
                int len=recv(connfd,buf,1024,0);
                if(len>0){
                        if(judjestrthe(buf)==6||judjestrthe(buf)==7){
                            wmove(InputWin,0,0);
                            wrefresh(InputWin);
             
                        }else{
                            
                        scroll(MsgWin);
                        mvwprintw(MsgWin,12,1,buf);
                        wrefresh(MsgWin);
                        memset(buf,0,sizeof(buf));
                        }
                }else{
                    if(len<0) {
                        perror("recv error");
                    }
                    close(connfd);
                    exit(1);
                }
            }

            if(FD_ISSET(0,&fds)){
                memset(cmd,0,sizeof(cmd));
                wgetstr(InputWin,cmd);
                scroll(MsgWin);
                mvwprintw(MsgWin,12,55-strlen(cmd),cmd);
                wrefresh(MsgWin);
                wclear(InputWin);
                wrefresh(InputWin);
                int len=send(connfd,cmd,strlen(cmd),0);
                if(len<0){
                    perror("send error");
                }
            }
        }
        wclear(InputWin);
        wrefresh(InputWin);
    }while(strcmp(cmd,"exit")!=0);
    clear();
    refresh();

    
}

int main(int argc,char **argv){
   
    if(argc!=2){
        fprintf(stderr,"usage: port\n");
        exit(1);
    }
    int port=atoi(argv[1]);
    sockaddr_in client_addr;
    memset(&client_addr,'\0',sizeof(client_addr));
    client_addr.sin_family=AF_INET;
    client_addr.sin_port=htons((unsigned short)port);
    //inet_aton("127.0.0.1",&client_addr.sin_addr);
    client_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    connfd=socket(AF_INET,SOCK_STREAM,0);
    if(connect(connfd,(sockaddr *)&client_addr,sizeof(client_addr))<0){
        fprintf(stderr,"connect error\n");
        exit(1);
    }else{
        char buf[1025];
        while(1){
            FD_ZERO(&fds);
            FD_SET(0,&fds);
            maxfd=0;
            FD_SET(connfd,&fds);
            if(connfd>maxfd) maxfd=connfd;
            timeval time;
            time.tv_sec=1;
            time.tv_usec=0;
            int err=select(maxfd+1,&fds,nullptr,nullptr,&time);
            if(err==-1){
                perror("select error");
                break;
            }else if(err==0){
                continue;
            }else{
                if(FD_ISSET(connfd,&fds)){
                    bzero(buf,1025);
                    int len=recv(connfd,buf,1024,0);
                    if(len>0){
                        if(strcmp(buf,"exit")==0){
                            close(connfd);
                            exit(0);
                        }else if(strcmp(buf,"login success")==0){ 
                            MainPage();
                        }else{
                            cout<<buf<<"\n";
                        }
                    }else{
                        if(len<0) perror("recv error");
                        break;
                    }
                }
                if(FD_ISSET(0,&fds)){
                    bzero(buf,1025);
                    fgets(buf,1024,stdin);
                    int len=send(connfd,buf,strlen(buf)-1,0);
                    if(len<0){
                        perror("send error");
                    }
                }
            }
        }
    }
    close(connfd);
    return 0;
}
