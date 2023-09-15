#include "list_timer.h"
#include <time.h>
sort_timer_lst::~sort_timer_lst(){
    
}
void sort_timer_lst::add_timer(util_timer* timer){
    if(!timer){
        return;
    }
    if(!head){
        head = tail = timer;
        return;
    }
    //如果插入的定时器小于在链表中的定时器。插在头部，否则调用重载private函数
    if(timer->expire < head->expire){
        timer->next = head;
        head->prev =timer;
        head = timer;
        return ;
    }
    add_timer(timer,head);
}

void sort_timer_lst::adjust_timer(util_timer * timer){
    if(!timer){
        return;
    }
    util_timer * tmp = timer->next;
    if(!tmp || (timer->expire < tmp->expire)){
        return;
    }
    //如果目标定时器是链表的头结点，，则将该定时器从链表中取出并重新插入链表
    if(timer == head){
        head = head->next;
        head->prev = NULL;
        timer->next = NULL;
        add_timer(timer,head);
/*如果目标定时器不是链表的头节点，
则将该定时器从链表中取出，然后插入其原来所在 位置之后的部分链表中*/
    }else{
        timer->prev->next = timer->next;
        timer->next->prev = timer->prev;
        add_timer(timer,timer->next);
    }
}
void sort_timer_lst::del_timer(util_timer * timer){
    if(!timer){
        printf("!time");
        return;
    }
    if(timer == head && timer->next == tail){
        printf("timer is head,next is tail");
        head = NULL;
        tail = NULL;
        delete timer;
        return ;
    }
    /*如果链表中至少有两个定时器，且目标定时器是链表的头结点，
    则将链表的头结点重置 为原头节点的下一个节点，然后删除目标定时器*/
    if(timer == head){
        printf("timer = head");
        head = head->next;
        head->prev = NULL;
        delete timer;
        return;
    }
    if(timer == tail){
        printf("timer == tail");
        tail = tail->prev;
        tail->next = NULL;
        delete timer;
        return ;
    }
    printf("timer in list!");
    timer->prev->next=timer->next; 
    timer->next->prev=timer->prev; 
    delete timer;
}
/*SIGALRM信号每次被触发就在其信号处理函数（如果使用统一事件源，则是主函数）
中 执行一次tick函数，以处理链表上到期的任务*/

void sort_timer_lst::tick(){
    if(!head){
        return ;
    }
    printf("timer tick\n");
    time_t cur = time(NULL);
    util_timer * tmp = head;
    while(tmp){
/*因为每个定时器都使用绝对时间作为超时值，所以我们可以把定时器的超时值
和系统当 前时间，比较以判断定时器是否到期*/
        if(cur<tmp->expire){
            break;
        }
/*调用定时器的回调函数，以执行定时任务*/
        tmp->cb(tmp->user_data);
        head = tmp->next;
        if(head){
            head->prev = NULL;
        }
        delete tmp;
        tmp = head;
    }
}
/*一个重载的辅助函数，它被公有的add_timer函数和adjust_timer函数调用。该函
数表示将目标定时器timer添加到节点lst_head之后的部分链表中*/
void sort_timer_lst::add_timer(util_timer * timer,util_timer* lst_head){
    util_timer * prev = lst_head;
    util_timer * tmp = prev->next;

    while(tmp){
        if(timer->expire < tmp->expire){
            prev->next=timer;
            timer->next=tmp;
            tmp->prev=timer;
            timer->prev=prev;
            break;
        }
        prev = tmp;
        tmp = tmp->next;
    }
    if(!tmp){
        prev->next=timer;
        timer->prev=prev;
        timer->next=NULL;
        tail=timer;
    }
}
