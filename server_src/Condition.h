#ifndef __CONDITION_H__
#define __CONDITION_H__

#include <pthread.h>
#include "MutexLock.h"

class Condition{
private:
    MutexLock &_lock;
    pthread_cond_t _cond;
public:
    Condition(MutexLock &mutex):_lock(mutex) {
        pthread_cond_init(&_cond,nullptr);
    }
    ~Condition() {pthread_cond_destroy(&_cond);}
    void Signal() {pthread_cond_signal(&_cond);}
    void BroadCast() { pthread_cond_broadcast(&_cond);}
    void Wait() { pthread_cond_wait(&_cond,_lock.GetMutex());}
};


#endif