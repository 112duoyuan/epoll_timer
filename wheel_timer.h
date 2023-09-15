#ifndef WHEEL_TIME_H__
#define WHEEL_TIME_H__

#include <sys/time.h>
#include "connect.h"


#define BUFFER_SIZE 64
class tw_timer;

typedef struct client_data{
    sockaddr_in address;
    int sockfd;
    char buf[BUFFER_SIZE];
    tw_timer * timer;
}client_data;

class tw_timer{
public:
    tw_timer(int rot, int ts);
    void(*cb)(client_data*);
    
    int rotation;/*记录定时器在时间轮转多少圈后生效*/
    int time_slot; /*记录定时器属于时间轮上哪个槽（对应的链表，下同）*/
    client_data * user_data; /*客户数据*/
    tw_timer * next;
    tw_timer * prev;
};
class time_wheel{
public:
    time_wheel();
    ~time_wheel();
    tw_timer * add_timer(int timeout);
    void del_timer(tw_timer * timer);
    void tick();


private:
    static const int N = 60;/*时间轮上槽的数目*/
    /*每1 s时间轮转动一次，即槽间隔为1 s*/
    static const int SI=1;
    /*时间轮的槽，其中每个元素指向一个定时器链表，链表无序*/
    tw_timer*slots[N];
    int cur_slot; /*时间轮的当前槽*/
};


#endif