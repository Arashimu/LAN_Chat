#include "ThreadPool.h"
#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
ThreadPool::ThreadPool(size_t ThreadNum,ShutdownMode shutdown_mode,size_t MaxQueueSize)
        :_ThreadNum(ThreadNum),_MaxQueueSize(MaxQueueSize),_ThreadPoolCond(_TaskQueueMutex),_ShutdownMode(shutdown_mode){
    while(_Threads.size()<ThreadNum){
        pthread_t tid;
        if(!pthread_create(&tid,nullptr,TaskForWorkerThreads,this)){
            _Threads.push_back(tid);
        }
    }
}

ThreadPool::~ThreadPool(){
   _TaskQueueMutex.Lock();
   if(_ShutdownMode==Immediate_Quit){
        while(!_TaskQueue.empty()){
            _TaskQueue.pop();
        }
   }
   for(size_t i=0;i<_ThreadNum;i++){
        auto ThreadExit=[](void *)->void {
            pthread_exit(0);
        };
        ThreadPoolTask task={ThreadExit,nullptr};
        _TaskQueue.push(task);
   }
   _ThreadPoolCond.BroadCast();
   for(size_t i=0;i<_ThreadNum;i++){
    pthread_join(_Threads[i],nullptr);
   }
   _TaskQueueMutex.Unlock();
}

bool ThreadPool::AddTask(void (*func)(void*),void *args){
    MutexLockGuard gurad(_TaskQueueMutex);
    if(_TaskQueue.size()>_MaxQueueSize){
        return false;
    }else{
        ThreadPoolTask task={ func, args };
        _TaskQueue.push(task);
        _ThreadPoolCond.Signal();
        return true;
    }

}
void *ThreadPool::TaskForWorkerThreads(void *arg){
    ThreadPool *pool=(ThreadPool*)arg;
    ThreadPoolTask task;
    while(1){ //每个线程一直循环尝试拿出任务执行
        pool->_TaskQueueMutex.Lock();  
        //std::cout<<pool->_TaskQueue.size()<<"\n"
        while(pool->_TaskQueue.size()==0){
            pool->_ThreadPoolCond.Wait();
        }
        assert(pool->_TaskQueue.size()!=0);
        task=pool->_TaskQueue.front();
        pool->_TaskQueue.pop();
        pool->_TaskQueueMutex.Unlock();
        try{
            (task.func)(task.args); //执行任务
        }catch(...){
            close(*(int*)task.args);
            continue;
        }
    }
    assert(0 && "TaskForWorkerThreads_ UNREACHABLE!");
    return nullptr;
}
