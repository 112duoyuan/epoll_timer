#include "connect.h"
#include <sys/time.h>
#include <stddef.h>
#include "list_timer.h"

#define FD_LIMIT 65535
#define MAX_EVENT_NUMBER 1024
#define TIMESLOT 5
static int pipefd[2];

static sort_timer_lst timer_lst;
static int epollfd = 0;

int setnonblock(int fd){
    int old_option = fcntl(fd,F_GETFL);//Value of file descriptor flags.
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd,F_SETFL,new_option);
    return old_option;
}
void addfd(int epollfd,int fd){
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET;
    epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&event);
    setnonblock(fd);
}

void sig_handler(int sig){
    int save_errno = errno;
    int msg = sig;
    send(pipefd[1],(char *)&msg,1,0);//1 write 0 read
    errno = save_errno;
}
void addsig(int sig){
    struct sigaction sa;
    memset(&sa,'\0',sizeof(sa));
    sa.sa_handler = sig_handler;
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig,&sa,NULL) != -1);
}

void timer_handler(){
    timer_lst.tick();
    alarm(TIMESLOT);
}
///*定时器回调函数，它删除非活动连接socket上的注册事件，并关闭之*/
void cb(client_data * user_data){
    epoll_ctl(epollfd,EPOLL_CTL_DEL,user_data->sockfd,0);
    assert(user_data);
    close(user_data->sockfd);
    printf("close fd %d\n",user_data->sockfd);
}

int timeout_connect(const char * ip,int port,int time){
    //sockaddr_in
    int ret = 0;
    struct sockaddr_in  address;
    bzero(&address,sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET,ip,&address.sin_addr);
    address.sin_port = htons(port);
    int sockfd = socket(PF_INET,SOCK_STREAM,0);
    assert(sockfd > 0);

    struct timeval timeout;
    timeout.tv_sec = time;
    timeout.tv_usec = 0;
    socklen_t len = sizeof(timeout);
    ret = setsockopt(sockfd,SOL_SOCKET,SO_SNDTIMEO,&timeout,len);
    assert(ret != -1);
    ret = connect(sockfd,(struct sockaddr*)&address,sizeof(address));
    if(ret == -1){

    }
    //处理定时任务
    if(errno == EINPROGRESS){
        printf("time out\n");
        return -1;
    }
    return sockfd;
}

/*处理非活动连接*/

int close_sock(const char * ip,int port){
    int ret = 0;
    struct sockaddr_in  address;
    bzero(&address,sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET,ip,&address.sin_addr);
    address.sin_port = htons(port);
/*AF_INET（又称 PF_INET）是 IPv4 网络协议的套接字类型*/
    int listenfd = socket(PF_INET,SOCK_STREAM,0);
    assert(listenfd >= 0);
    ret = bind(listenfd,(struct sockaddr*)&address,sizeof(address));
    assert(ret != -1);

    ret = listen(listenfd,5);
    assert(ret!= -1);
    
    epoll_event events[MAX_EVENT_NUMBER];
    int epollfd = epoll_create(5);
    assert(epollfd != -1);
    addfd(epollfd,listenfd);
    //创建无名的套接字
    ret = socketpair(PF_UNIX,SOCK_STREAM,0,pipefd);
    assert(ret != -1);
    setnonblock(pipefd[1]);
    addfd(epollfd,pipefd[0]);

    /*设置信号处理函数*/
    addsig(SIGALRM);
    addsig(SIGTERM);
    bool stop_server = false;
    client_data * users = new client_data[FD_LIMIT];
    bool timeout = false;
    alarm(TIMESLOT);
    while(!stop_server){
        int number = epoll_wait(epollfd,events,MAX_EVENT_NUMBER,- 1);
        if(number < 0 && (errno != EINTR)){
            printf("epoll faileure\n");
            break;
        }
        for(int i = 0; i < number;i++){
            int sockfd = events[i].data.fd;
            printf("active sock total is number %d\n",number);
            printf("processing sockfd is %d\n",sockfd);
            if(sockfd == listenfd){
                struct sockaddr_in client_address;
                socklen_t client_addrlength = sizeof(client_address);
                int connfd = accept(listenfd,(struct sockaddr *)&client_address,&client_addrlength);
                addfd(epollfd,connfd);
                users[connfd].address = client_address;
                users[connfd].sockfd = connfd;
                util_timer * timer = new util_timer;
                timer->user_data = &users[connfd];
                timer->cb = cb;
                time_t cur = time(NULL);
                timer->expire = cur + 3 * TIMESLOT;
                users[connfd].timer = timer;
                timer_lst.add_timer(timer);        
            }else if((sockfd == pipefd[0]) && (events[i].events & EPOLLIN)){
                int sig;
                char signals[1024];
                ret = recv(pipefd[0],signals,sizeof(signals),0);
                if(ret == -1){
                    continue;
                }else if(ret == 0){
                    continue;
                }else{
                    for(int i = 0;i < ret; i++){
                        switch(signals[i]){
                            case SIGALRM:
                            {
                                timeout = true;
                                break;
                            }
                            case SIGTERM:
                            {
                                stop_server = true;
                            }
                        }
                    }
                }
            }else if(events[i].events & EPOLLIN){
                memset(users[sockfd].buf,0x00,BUFFER_SIZE);
                ret = recv(sockfd,users[sockfd].buf,BUFFER_SIZE-1,0);
                printf("get %d bytes of client data %s from %d\n",ret,
                users[sockfd].buf,sockfd);

                util_timer * timer = users[sockfd].timer;
                if(ret < 0){ 
                    if(errno != EAGAIN){
                        cb(&users[sockfd]);
                        if(timer){
                            printf("ret < 0");
                            timer_lst.del_timer(timer);
                        }
                    }       
                }else if(ret == 0){
                    /*如果对方已经关闭连接，则我们也关闭连接，并移除对应的定时器*/
                    cb(&users[sockfd]);
                    if(timer){
                        printf("ret =0");
                        timer_lst.del_timer(timer);
                    }
                }else{
                    /*如果某个客户连接上有数据可读，则我们要调整该连接对应的定时器，以延迟该连接被
                        关闭的时间*/
                    if(timer){
                        time_t cur = time(NULL);
                        timer->expire = cur + 3 * TIMESLOT;
                        printf("adjust timer once\n");
                        timer_lst.adjust_timer(timer);
                    }
                }
            }else{
                //others
            }
        }
/*最后处理定时事件，因为I/O事件有更高的优先级。当然，这样做将导致定时任务不能
精确地按照预期的时间执行*/
        if(timeout){
            timer_handler();
            timeout=false;
        }    
    }
    close(listenfd);
    close(pipefd[1]);
    close(pipefd[0]);
    delete []users;
    return 0;
    
}






int main(int argc ,char ** argv){
    if(argc <= 2){
        printf("usage:%s ip_address port_num\n",basename(argv[0]));
        return 1;
    }
    const char * ip = argv[1];
    int port = atoi(argv[2]);
    // int sockfd = timeout_connect(ip,port,10);
    // if(sockfd < 0){
    //     return 1;
    // }
    /*-----------------------------------------------*/
    int ret = close_sock(ip,port);
    if(!ret){
        printf("wrong !!!!!");
    }
    return 0;
    
}