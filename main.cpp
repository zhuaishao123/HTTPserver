#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <error.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include "locker.h"
#include "threadpool.h"
#include <signal.h>
#include "http_conn.h"
#include <errno.h>
#include <assert.h>


#define MAX_FD 65535  //最大文件描述符个数
#define MAX_EVENT_NUMBER 10000  //单次监听最大数量

//添加文件描述符到epoll中
extern void addfd(int epollfd,int fd, bool one_shot);
//从epoll中删除文件描述符
extern void removefd(int epollfd,int fd);
//从epoll中修改文件描述符
extern void modfd(int epollfd,int fd,int ev);

// 添加信号捕捉
void addsig(int sig , void(handler)(int)){
    struct sigaction sa;
    memset(&sa,0,sizeof(sa));
    sa.sa_handler = handler;
    sigfillset(&sa.sa_mask);
    assert( sigaction( sig, &sa, NULL ) != -1 );

}

int main(int argc , char * argv[]){
    
    if(argc <= 1){
        printf("按照如下格式运行: %s port_number\n",basename(argv[0]));
        exit(-1);
    }

    //获取端口号
    int port = atoi(argv[1]);


    //对SIGPIE信号处理
    addsig(SIGPIPE,SIG_IGN);

    
    //创建线程池
    threadpool<http_conn> * pool = NULL;
    try{
        pool = new threadpool<http_conn>;
    }catch(...){
        exit(-1);   
    }

    
    //创建一个数组，保存所有的客户端信息
    http_conn * users = new http_conn[MAX_FD];
    
    //创建监听套接字
    int listenfd = socket(PF_INET,SOCK_STREAM,0);

    //绑定
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);


    //设置端口复用
    int reuse = 1;
    setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse));   //delete--------------------
    int ret = bind(listenfd,(struct sockaddr *)&address,sizeof(address));
    if(ret == -1){
        perror("bind");
        exit(-1);
    }

    
    //监听
    ret = listen(listenfd,5);
    if(ret == -1){
        perror("listen");
        exit(-1);
    }


    //创建epoll对象,事件数组，添加
    epoll_event events[MAX_EVENT_NUMBER];
    int epollfd = epoll_create(5);

    //将监听文件描述符添加到epoll对象中
    addfd(epollfd,listenfd,false);
    http_conn::m_epollfd = epollfd;

    
    while(true){
        
        int num = epoll_wait(epollfd,events,MAX_EVENT_NUMBER,-1);
        if( (num < 0) && (errno!=EINTR)){
            printf("epoll failure\n");
            break;
        }
        // printf("num个数为 : %d\n",num);
        //循环遍历
        for(int i = 0;i<num;i++){
            int sockfd = events[i].data.fd;
            if(sockfd == listenfd){
                //有客户端连接进来
                printf("有客户端连接\n");
                struct sockaddr_in clientaddr;
                socklen_t clientaddr_len = sizeof(clientaddr);
                int connfd = accept(sockfd,(struct sockaddr *)&clientaddr,&clientaddr_len);
                if(connfd == -1){
                    perror("accept");
                    exit(-1);
                }

                if(http_conn::m_user_count >= MAX_FD){
                    //目前连接数满了
                    //给客户端写一个信息，服务端正忙。
                    
                    close(connfd);
                    continue;
                }
                //将新的客户端数据初始化，放到数组中
                users[connfd].init(connfd, clientaddr);

            }else if(events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)){   //EPOLLRDHUP | EPOLLHUP | EPOLLERR
                //对方异常断开或者错误等事件
                //关闭连接
                printf("对方异常\n");
                users[sockfd].close_conn();

            }else if(events[i].events & EPOLLIN){
                //读
                if(users[sockfd].read()){
                    //一次性把数据全部读完
                    pool->append(users + sockfd);
                    //pool.append(&users[sockfd]);

                }else{
                    users[sockfd].close_conn();
                }
            }else if(events[i].events & EPOLLOUT){
                if(!users[sockfd].write()){ //一次性写完所有数据 
                    users[sockfd].close_conn();
                }
            }
        }
    }

    close(epollfd);
    close(listenfd);
    delete [] users;
    delete pool;
    return 0;

}