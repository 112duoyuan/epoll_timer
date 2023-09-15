#include "wheel_timer.h"
tw_timer::tw_timer(int rot,int ts)
    :next(NULL)
    ,prev(NULL)
    ,rotation(rot)
    ,time_slot(ts){

}

time_wheel::time_wheel()
    :cur_slot(0){
    for(int i = 0; i < N;i++){
        slots[i] = NULL;
    }  
}
time_wheel::~time_wheel(){
    for(int i = 0; i< N;i++){
        tw_timer * tmp = slots[i];
        while(tmp){
            slots[i] = tmp->next;
            delete tmp;
            tmp = slots[i];
        }
    }
}
/*根据定时值timeout创建一个定时器，并把它插入合适的槽中*/
tw_timer * time_wheel::add_timer(int timeout){
    if(timeout < 0){
        return NULL;
    }
    int ticks = 0;
 /*下面根据待插入定时器的超时值计算它将在时间轮转动多少个滴答后被触发，并将该滴
答数存储于变量ticks中。如果待插入定时器的超时值小于时间轮的槽间隔SI，则将ticks向
上折合为1，否则就将ticks向下折合为timeout/SI*/
    if(timeout < SI){
        ticks = 1;
    }else{
        ticks = timeout/SI;
    }
    /*计算待插入的定时器在时间轮转动多少圈后被触发*/
    int rotation = ticks /N;
    /*计算待插入的定时器应该被插入哪个槽中*/
    int ts = (cur_slot + (ticks % N)) % N;
    /*创建新的定时器，它在时间轮转动rotation圈之后被触发，且位于第ts个槽上*/
    tw_timer * timer = new tw_timer(rotation,ts);
    if(!slots[ts]){
        printf("add timer,totation is %d, ts is %d, cur_slot is %d\n",
            rotation,ts,cur_slot);
        slots[ts] = timer;
    }else{
        timer->next = slots[ts];
        slots[ts]->prev = timer;
        slots[ts] = timer;
    }
    return timer;
}
void time_wheel::del_timer(tw_timer * timer){
    if(!timer){
        return;
    }
    int ts= timer->time_slot;
    if(timer == slots[ts]){
        slots[ts] = slots[ts]->next;
        if(slots[ts]){
            slots[ts]->prev = NULL;
        }
        delete timer;
    }else{
        timer->prev->next = timer->next;
        if(timer->next){
            timer->next->prev = timer->prev;
        }
        delete timer;
    }
}
/*SI时间到后，调用该函数，时间轮向前滚动一个槽的间隔*/
void time_wheel::tick(){
    tw_timer * tmp = slots[cur_slot];
    printf("current slot is %d\n",cur_slot);
    while(tmp){
        printf("tick the timer once\n");
        /*如果定时器的rotation值大于0，则它在这一轮不起作用*/
        if(tmp->rotation > 0){
            tmp->rotation--;
            tmp = tmp->next;
        }else{
        /*否则，说明定时器已经到期，于是执行定时任务，然后删除该定时器*/
            tmp->cb(tmp->user_data);
            if(tmp == slots[cur_slot]){
                printf("delete header in cur_slot\n");
                slots[cur_slot]= tmp->next;
                delete tmp;
                if(slots[cur_slot]){
                    slots[cur_slot]->prev = NULL;
                }
                tmp = slots[cur_slot];
            }else{
                tmp->prev->next = tmp->next;
                if(tmp->next){
                    tmp->next->prev = tmp->prev;
                }
                tw_timer * tmp2 = tmp->next;
                delete tmp;
                tmp = tmp2;
            }
        }
    }
    cur_slot= ++cur_slot%N;

}
