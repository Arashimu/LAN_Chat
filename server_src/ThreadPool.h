#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__

#include "Condition.h"

#include <cassert>
#include <queue>

class ThreadPool{
public:
    enum ShutdownMode{
        WaitWrokFinish_Quit,
        Immediate_Quit,
    };
    ThreadPool(size_t ThreadNum,ShutdownMode shutdown_mode=WaitWrokFinish_Quit,size_t MaxQueueSize=-1);

    ~ThreadPool();

    bool AddTask(void (*func)(void*),void *args);
private:
    static void *TaskForWorkerThreads(void *arg);
    struct ThreadPoolTask{
        void (*func)(void*);
        void *args;
    };
    size_t _ThreadNum;
    size_t _MaxQueueSize;


    std::queue<ThreadPoolTask> _TaskQueue; //任务队列
    
    std::vector<pthread_t> _Threads;   //等待线程集合

    MutexLock _TaskQueueMutex;
    Condition _ThreadPoolCond;

    ShutdownMode _ShutdownMode;

};



#endif