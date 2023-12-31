#include<sys/signal.h>
#include<event.h>
void signal_cb(int fd,short event,void*argc){
    struct event_base*base=(event_base*)argc;
    struct timeval delay={2,0};
    printf("Caught an interrupt signal;exiting cleanly in two seconds...\n");
    event_base_loopexit(base,&delay);
}
void timeout_cb(int fd,short event,void*argc){
    printf("timeout\n");
}
int main(){
    //1）调用event_init函数创建event_base对象。一个event_base相当于一个Reactor实例。
    struct event_base*base=event_init();
/*
#define evsignal_new(b,x,cb,arg)\
event_new((b),(x),EV_SIGNAL|EV_PERSIST,(cb),(arg))
#define evtimer_new(b,cb,arg)event_new((b),-1,0,(cb),(arg))
*/

    //创建信号事件处理器
    struct event*signal_event=evsignal_new(base,SIGINT,signal_cb,base);
    event_add(signal_event,NULL);
    timeval tv={1,0};
    //定时事件处理器
    struct event*timeout_event=evtimer_new(base,timeout_cb,NULL);
    event_add(timeout_event,&tv);
    event_base_dispatch(base);
    event_free(timeout_event);
    event_free(signal_event);
    event_base_free(base);
}