#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include "TaskQueue.h"
#include <iostream>
#include <string>
#include <string.h>
#include <unistd.h>

template <typename T>
class ThreadPool
{
public:
  ThreadPool(int min, int max);
  ~ThreadPool();
  void addTask(Task<T> task); // 添加任务
  int getBusyNum();           // 获取忙活的线程个数
  int getAliveNum();          // 获取存活的线程个数
private:
  static void *worker(void *arg);  // 工作线程的任务函数
  static void *manager(void *arg); // 管理者线程的任务函数
  void threadExit();

private:
  TaskQueue<T> *task_queue; // 任务队列
  pthread_t manager_id;     // 管理者线程ID
  pthread_t *thread_id;     // 工作的线程ID
  int min_num;              // 最小线程数量
  int max_num;              // 最大线程数量
  int busy_num;             // 正在工作的线程个数
  int alive_num;            // 存活的线程个数
  int exit_num;             // 要销毁的线程个数
  pthread_mutex_t lock;     // 锁整个线程池
  pthread_cond_t not_empty; // 任务队列是否已空
  static const int number = 2;
  bool shutdown = false; // 是否要销毁线程池
};

#endif