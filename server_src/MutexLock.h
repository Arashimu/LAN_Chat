#ifndef __MUTEXLOCK_H__
#define __MUTEXLOCK_H__

#include <pthread.h>

class MutexLock{
private:
    pthread_mutex_t _mutex;
public:
    MutexLock() { pthread_mutex_init(&_mutex,nullptr);}
    ~MutexLock() { pthread_mutex_destroy(&_mutex);}
    void Lock() { pthread_mutex_lock(&_mutex);}
    void Unlock() { pthread_mutex_unlock(&_mutex);}
    pthread_mutex_t *GetMutex() { return &_mutex;}
};

class MutexLockGuard{
private:
    MutexLock &_lock;
public:
    MutexLockGuard(MutexLock &mutex):_lock(mutex){_lock.Lock();}
    ~MutexLockGuard() {_lock.Unlock();}
};
#endif


