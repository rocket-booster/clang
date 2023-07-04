/*
 * @Author: RockyWu
 * @Date: 2022-05-01 12:16:41
 * @LastEditors: RockyWu
 * @LastEditTime: 2022-05-01 23:18:40
 * @FilePath: /clang/self_utility/01_thread_pool/thread_pool.h
 * @Description: 
 *   // 1 
 *   ln -s thread_pool.hpp thread_pool.cpp
 *   g++ -pthread -std=c++11  -o test thread_pool.cpp
 *   
 *   // 2 -std=c++11
 *   cp src to main.cpp
 *   g++ -pthread   -o test main.cpp
 * Copyright (c) 2022 by RockyWu/Personal, All Rights Reserved. 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>

namespace ROC_ThreadPool{
  class ThreadPool;
  
  // start 双向链表操作
  // 头插法
  #define LL_ADD(item, list) do { 	\
    item->prev = NULL;				\
    item->next = list;				\
    list = item;					\
  } while(0)

  #define LL_REMOVE(item, list) do {						\
    if (item->prev != NULL) item->prev->next = item->next;	\
    if (item->next != NULL) item->next->prev = item->prev;	\
    if (list == item) list = item->next;					\
    item->prev = item->next = NULL;							\
  } while(0)
  // end 双向链表操作 


  class Worker { 
  public:
    // 自身属性
    pthread_t m_thread_id = -1;
    int m_exit_flag = 0;

    // 组织: 双向队列    
    Worker* prev = nullptr;
    Worker* next = nullptr;

    // 从ThreadPool取任务
    ThreadPool* m_pool = nullptr;

  };// end class Worker
  

  class Task {
  public:
    // 自身属性
    void (*m_task_func)(Task*);
    void* m_data = nullptr;
    
    // 组织: 双向队列    
    Task* prev = nullptr;
    Task* next = nullptr;

  };// end class Task


  class ThreadPool{
    private:
    int m_num_workers = 1;
    Worker* m_workers = nullptr;  // 执行队列  
    Task* m_wait_tasks = nullptr; // 任务队列

    pthread_cond_t m_task_cond = PTHREAD_COND_INITIALIZER;
    pthread_mutex_t m_task_mutex = PTHREAD_MUTEX_INITIALIZER;
    
    public:
    
    ThreadPool(int num_workers){
      if (num_workers < 1) num_workers = 1;

      m_num_workers = num_workers;
    }

    int Init(){
      for (int i = 0;i < m_num_workers;i ++) {

        // 创建对象
        Worker* new_worker = new Worker();
        if (new_worker == NULL) {
          perror("new Worker failed!");
          return 1;
        }
        
        // 初始化对象
        new_worker->m_pool = this;      
        int ret = pthread_create(&new_worker->m_thread_id, NULL, WorkerThreadEntry, (void *)new_worker);
        if (ret) {          
          perror("pthread_create failed!");
          delete new_worker;
          return 2;
        }
        
        // 内部组织起来
        LL_ADD(new_worker, new_worker->m_pool->m_workers);
      }
    }

    int DeInit(){
      Worker *worker = nullptr;
      for (worker = m_workers; worker != nullptr; worker = worker->next) {
        worker->m_exit_flag = 1;
      }

	    pthread_mutex_lock(&m_task_mutex);
      
	    m_workers = nullptr;   // worker 退出时自己释放
	    m_wait_tasks = nullptr;// 不做链表内容释放, 用处不大

	    pthread_cond_broadcast(&m_task_cond);
	    pthread_mutex_unlock(&m_task_mutex);
      return 0;
    }

    int push_task(Task* task){
      pthread_mutex_lock(&m_task_mutex);

      LL_ADD(task, m_wait_tasks);
      
      pthread_cond_signal(&m_task_cond);
      pthread_mutex_unlock(&m_task_mutex);
      return 0;
    }

    Task* pop_task(Worker* worker){
        // part1: 获取任务
        pthread_mutex_lock(&m_task_mutex);

        // 等待任务队列初始化完毕
        while (m_wait_tasks == NULL) {
          if (worker->m_exit_flag) break;
          pthread_cond_wait(&m_task_cond, &m_task_mutex);
        }
        
        // 取一个等待任务
        Task *task = m_wait_tasks;
        if (task != nullptr) {
          LL_REMOVE(task, m_wait_tasks);
        }
        
        pthread_mutex_unlock(&m_task_mutex);
        return task;
    }

    static void* WorkerThreadEntry(void *ptr){
      Worker *worker = (Worker*)ptr;

      while (1) {
        // 工作线程退出
        if (worker->m_exit_flag) {          
          break;
        }

        // part1: 取任务
        Task* task = worker->m_pool->pop_task(worker);
        if (task == nullptr){
          continue;
        }       

        // part2: 处理任务
        task->m_task_func(task);
      }

      free(worker);
      pthread_exit(NULL);
    }// end WorkerThreadFunc
  };// end class ThreadPool

}// end namespace


#if 0 // g++ 无法直接编译 h
#include "thread_pool.h"
/*
 * @Author: RockyWu
 * @Date: 2022-05-01 21:41:58
 * @LastEditors: RockyWu
 * @LastEditTime: 2022-05-01 23:14:55
 * @FilePath: /clang/self_utility/01_thread_pool/main.cpp
 * @Description: 
 * 
 * Copyright (c) 2022 by RockyWu/Personal, All Rights Reserved. 
 */
using namespace ROC_ThreadPool;
#define ROC_MAX_NUM_THREAD 4
#define ROC_COUNTER_SIZE		1000

void counter(Task *task) {

	int index = *(int*)task->m_data;

	printf("index : %d, selfid : %lu\n", index, pthread_self());
	
	delete task->m_data; // 用户销毁
	delete task;         // 用户销毁
}
int main(int argc, char *argv[]) {

	ThreadPool pool(ROC_MAX_NUM_THREAD);
  
	pool.Init();
	
	int i = 0;
	for (i = 0;i < ROC_MAX_NUM_THREAD;i ++) {
		Task *task = new Task(); // 用户创建
		if (task == nullptr) {
			perror("malloc");
			exit(1);
		}
		
		task->m_task_func = counter;
		task->m_data = new int;// 用户创建
		*(int*)task->m_data = i;
    pool.push_task(task);			
	}
  
	getchar();
	printf("\n");	
}
#endif 