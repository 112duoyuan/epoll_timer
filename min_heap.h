#ifndef __MIN_HEAR_H__
#define __MIN_HEAR_H__

#include "connect.h"
#include <sys/time.h>
#include <exception>

using std::exception;
#define BUFFER_SIZE 64

class heap_timer;

typedef struct client_data{
    sockaddr_in address;
    int sockfd;
    char buf[BUFFER_SIZE];
    heap_timer * timer;
}client_data;

class heap_timer{
public:
    heap_timer(int delay);
    time_t expire;/*定时器生效的绝对时间*/
    void(*cb)(client_data*);
    client_data * user_data;
};

class time_heap{
public:
    time_heap(int cap);
    time_heap(heap_timer**init_array,int size,int capacity);

    ~time_heap();
    void add_timer(heap_timer* timer);
    void del_timer(heap_timer* timer);

    heap_timer* top()const;
    void pop_timer();
    void tick();
    bool empty()const{return cur_size == 0;}


private:
    heap_timer ** array;/*堆数组*/
    int capacity;/*堆数组的容量*/
    int cur_size; /*堆数组当前包含元素的个数*/

    void percolate_down(int hole);

    void resize();

};


#endif