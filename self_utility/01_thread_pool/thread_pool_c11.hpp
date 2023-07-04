/*
 * @Author: RockyWu
 * @Date: 2022-05-01 12:16:41
 * @LastEditors: RockyWu
 * @LastEditTime: 2022-05-01 23:09:27
 * @FilePath: /clang/self_utility/01_thread_pool/thread_pool_c11.hpp
 * @Description: 编译命令
 *   cp src to main.cpp
 *   g++ -pthread -std=c++11  -o test main.cpp
 * Copyright (c) 2022 by RockyWu/Personal, All Rights Reserved. 
 */

#include <iostream>         // std::cout

#include <thread>             // std::thread
#include <mutex>              // std::mutex, std::unique_lock(支持cond的notify_one后再次lock)
#include <condition_variable> // std::condition_variable notify_all notify_one

#include <queue>      // std::queue
#include <memory>     // std::shared_ptr
#include <functional> // std::function

namespace ROC_ThreadPool{
  class ThreadPool;

  class Worker { 
  public:
    // 自身属性
    std::thread m_thread;
    int m_exit_flag = 0;

    // 从ThreadPool取任务
    ThreadPool* m_pool = nullptr;

  };// end class Worker
  

  class Task {
  public:
    // 自身属性
    std::function<void(std::shared_ptr<Task>)> m_task_func;
    void* m_data;

    Task(std::function<void(std::shared_ptr<Task>)> task_func, void* data){
      m_task_func = task_func;
      m_data = data;
    }
    ~Task(){
      std::cout << __FUNCTION__ << ":" << __LINE__ << std::endl;
      if (m_data != nullptr){
        delete m_data;
        m_data = nullptr;
      }
    }
  };// end class Task


  class ThreadPool{
    private:
    int m_num_workers = 1;
    std::queue<std::shared_ptr<Worker>> m_workers;  // 执行队列    
    std::queue<std::shared_ptr<Task>> m_wait_tasks; // 任务队列
    
    std::mutex m_task_mutex;
    std::condition_variable m_task_cond;

    public:
    ThreadPool(int num_workers){
      if (num_workers < 1) num_workers = 1;
      m_num_workers = num_workers;
    }

    int Init(){
      for (int i = 0;i < m_num_workers;i ++) {

        // 创建对象
        std::shared_ptr<Worker> new_worker = std::shared_ptr<Worker>(new Worker());
        if (new_worker == nullptr) {
          perror("new Worker failed!");
          return 1;
        }
                
        // 初始化对象
        new_worker->m_pool = this;            
		    new_worker->m_thread = std::thread(WorkerThreadEntry, new_worker);  
        
        // 内部组织起来
        m_workers.push(new_worker);

      }
    }

    void DeInit(){
      std::lock_guard<std::mutex> lock(m_task_mutex);
      while(!m_workers.empty()){
        auto& worker = m_workers.front(); 
        worker->m_exit_flag = 1;
        m_workers.pop();
      }
      
      while(!m_wait_tasks.empty()){
        auto& task = m_wait_tasks.front();         
        m_wait_tasks.pop();
      }      
      m_task_cond.notify_all();
    }

    int push_task(std::shared_ptr<Task> task){
      std::lock_guard<std::mutex> lk(m_task_mutex);
      m_wait_tasks.push(task);      
      m_task_cond.notify_one();      
      return 0;
    }

    std::shared_ptr<Task> pop_task(std::shared_ptr<Worker> worker){
        // part1: 获取任务
        std::unique_lock<std::mutex> lock(m_task_mutex);        

        // 等待任务队列初始化完毕
        while (m_wait_tasks.empty()) {
          if (worker->m_exit_flag) break;
          m_task_cond.wait(lock);          
        }
                
        if (m_wait_tasks.empty())
          return nullptr;

        // 取一个等待任务
        auto task = m_wait_tasks.front();        
        m_wait_tasks.pop();
        return task;
    }

    static void* WorkerThreadEntry(std::shared_ptr<Worker> worker){

      while (1) {
        // 工作线程退出
        if (worker->m_exit_flag) {          
          break;
        }

        // part1: 取任务
        auto task = worker->m_pool->pop_task(worker);
        if (task == nullptr){
          continue;
        }       

        // part2: 处理任务
        task->m_task_func(task);
      }
      
      pthread_exit(NULL);
    }// end WorkerThreadFunc
  };// end class ThreadPool

}// end namespace


#if 0 // g++ 无法直接编译 hpp
/*
 * @Author: RockyWu
 * @Date: 2022-05-01 21:41:58
 * @LastEditors: RockyWu
 * @LastEditTime: 2022-05-01 22:44:54
 * @FilePath: /clang/self_utility/01_thread_pool/main.cpp
 * @Description: 编译命令
 *   g++ -pthread -std=c++11  -o test main.cpp
 * Copyright (c) 2022 by RockyWu/Personal, All Rights Reserved. 
 */
#include "thread_pool_c11.hpp"
using namespace ROC_ThreadPool;
#define ROC_MAX_NUM_THREAD 4
#define ROC_COUNTER_SIZE		1000

void counter(std::shared_ptr<Task> task) {

	//int index = task->m_data;
  int index = *(int*)task->m_data;
	printf("index : %d, selfid : %lu\n", index, pthread_self());	
  delete task->m_data;
  task->m_data = nullptr;
}
int main(int argc, char *argv[]) {

	ThreadPool pool(ROC_MAX_NUM_THREAD);
  
	pool.Init();
	
	int i = 0;
	for (i = 0;i < ROC_MAX_NUM_THREAD;i ++) {
    int *data = new int;
    *data = i;
		std::shared_ptr<Task> task(new Task(counter, (void*)data)); // 用户创建
		if (task == nullptr) {
			perror("new");
			exit(1);
		}
				
    pool.push_task(task);			
	}
  
	getchar();
	printf("\n");	
}
#endif 