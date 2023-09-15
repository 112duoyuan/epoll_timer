#ifndef LST_TIMER
#define LST_TIMER
#include <sys/time.h>
#include "connect.h"

#define BUFFER_SIZE 64

class util_timer;

typedef struct client_data
{
    sockaddr_in address;
    int sockfd;
    char buf[BUFFER_SIZE];
    util_timer * timer;
}client_data;

class util_timer{
public:
    util_timer()
    :prev(NULL)
    ,next(NULL){}

    time_t expire;//任务超时时间，使用绝对时间
    void(*cb)(client_data*);/*任务回调函数*/
    client_data * user_data;
    util_timer * prev;
    util_timer * next;
};
//升序定时器链表
class sort_timer_lst{
public:
    sort_timer_lst()
    :head(NULL)
    ,tail(NULL){}

    ~sort_timer_lst();
    //将目标定时器timer添加到链表中
    void add_timer(util_timer* timer);
    //当某个定时任务发生变化时，调整对应的定时器在链表中的位置。
    //这个函数只考虑被调 整的定时器的超时时间延长的情况，
    //即该定时器需要往链表的尾部移动
    void adjust_timer(util_timer * timer);

    void del_timer(util_timer * timer);

    void tick();

private:
    /*一个重载的辅助函数，它被公有的add_timer函数和adjust_timer函数调用。
    该函 数表示将目标定时器timer添加到节点lst_head之后的部分链表中*/
    void add_timer(util_timer * tiemr,util_timer* lst_head);

    util_timer* head;
    util_timer* tail;

};




#endif