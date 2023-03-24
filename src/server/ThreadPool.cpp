#include "ThreadPool.h"
#include <iostream>

template <typename T>
ThreadPool<T>::ThreadPool(int minNum, int maxNum)
{
  this->task_queue = new TaskQueue<T>();

  this->min_num = minNum;
  this->max_num = maxNum;
  this->busy_num = 0;
  this->alive_num = 0;

  this->thread_id = new pthread_t[max_num];
  memset(this->thread_id, 0, sizeof(pthread_t) * max_num);

  if (pthread_mutex_init(&this->lock, NULL) != 0 || pthread_cond_init(&this->not_empty, NULL) != 0)
  {
    std::cout << "mutex or cond init failed" << std::endl;
    exit(-1);
  }

  for (int i = 0; i < this->min_num; i++)
  {
    pthread_create(&this->thread_id[i], NULL, worker, this);
    std::cout << "create thread " << this->thread_id[i] << std::endl;
  }

  pthread_create(&this->manager_id, NULL, manager, this);
}

template <typename T>
ThreadPool<T>::~ThreadPool()
{
  this->shutdown = true;
  // 阻塞等待管理者线程
  pthread_join(this->manager_id, NULL);
  // 唤醒所有阻塞的线程
  for (int i = 0; i < this->alive_num; i++)
  {
    pthread_cond_signal(&this->not_empty);
  }
  if (this->thread_id)
  {
    delete[] this->thread_id;
  }
  if (this->task_queue)
  {
    delete this->task_queue;
  }
  pthread_mutex_destroy(&this->lock);
  pthread_cond_destroy(&this->not_empty);
}

template <typename T>
void ThreadPool<T>::addTask(Task<T> task)
{
  if (this->shutdown)
  {
    return;
  }
  // 添加任务，不需要加锁，任务队列中有锁
  this->task_queue->addTask(task);
  // 唤醒工作线程
  pthread_cond_signal(&this->not_empty);
}

template <typename T>
int ThreadPool<T>::getBusyNum()
{
  pthread_mutex_lock(&lock);
  int busyNum = this->busy_num;
  pthread_mutex_unlock(&lock);
  return busyNum;
}

template <typename T>
int ThreadPool<T>::getAliveNum()
{
  pthread_mutex_lock(&lock);
  int threadNum = this->alive_num;
  pthread_mutex_unlock(&lock);
  return threadNum;
}

// 工作线程任务函数 template <typename T>
template <typename T>
void *ThreadPool<T>::worker(void *arg)
{
  ThreadPool *pool = static_cast<ThreadPool *>(arg);
  while (1)
  {
    // 访问任务队列（共享资源）加锁
    pthread_mutex_lock(&pool->lock);

    // 判断任务队列是否为空，若为空工作线程阻塞
    while (pool->task_queue->taskNum() == 0 && !pool->shutdown)
    {
      // std::cout << "thread " << std::to_string(pthread_self()) << " waiting..." << std::endl;
      // 阻塞线程
      pthread_cond_wait(&pool->not_empty, &pool->lock);

      // 判断是否要销毁线程
      if (pool->exit_num > 0)
      {
        pool->exit_num--;
        if (pool->alive_num > pool->min_num)
        {
          pool->alive_num--;
          pthread_mutex_unlock(&pool->lock);
          pool->threadExit();
        }
      }
    }

    // 判断线程池是否被摧毁
    if (pool->shutdown)
    {
      pthread_mutex_unlock(&pool->lock);
      pool->threadExit();
    }

    // 从任务队列中取出一个任务
    Task<T> task = pool->task_queue->getTask();
    // 工作的线程+1
    pool->busy_num++;
    // 任务队列解锁
    pthread_mutex_unlock(&pool->lock);
    // 执行任务
    // std::cout << "thread " << std::to_string(pthread_self()) << " start working..." << std::endl;
    task.function(task.arg1, task.arg2);
    // delete task.arg1;
    // delete task.arg2;
    task.arg1 = -1;
    task.arg2 = -1;

    // 任务处理结束
    // std::cout << "thread " << std::to_string(pthread_self()) << " finish work..." << std::endl;
    pthread_mutex_lock(&pool->lock);
    pool->busy_num--;
    pthread_mutex_unlock(&pool->lock);
  }
  return nullptr;
}

// 管理者线程任务函数
template <typename T>
void *ThreadPool<T>::manager(void *arg)
{
  ThreadPool *pool = static_cast<ThreadPool *>(arg);
  // 若线程池没有关闭，就一直检测
  while (!pool->shutdown)
  {
    // 每5秒检测一次
    sleep(5);
    // 取出线程池中的任务数和线程数量
    // 取出工作的线程池数量
    pthread_mutex_lock(&pool->lock);
    int queueSize = pool->task_queue->getTaskNum();
    int liveNum = pool->alive_num;
    int busyNum = pool->busy_num;
    pthread_mutex_unlock(&pool->lock);

    // 创建线程
    // 当前任务个数 > 存活的线程数 && 存活的线程数 < 最大线程个数
    if (queueSize > liveNum && liveNum < pool->maxNum)
    {
      // 加锁
      pthread_mutex_lock(&pool->lock);
      int num = 0;
      for (int i = 0; i < pool->max_num && num < ThreadPool::number && pool->alive_num < pool->max_num; i++)
      {
        if (pool->thread_id[i] == 0)
        {
          pthread_create(&pool->thread_id[i], NULL, worker, pool);
          num++;
          pool->alive_num++;
        }
      }
      pthread_mutex_unlock(&pool->lock);
    }

    // 销毁多余的线程
    // 工作线程 * 2 < 存活的线程 && 存活的线程 > 最小线程数量
    if (busyNum * 2 < liveNum && liveNum > pool->min_num)
    {
      pthread_mutex_lock(&pool->lock);
      pool->exit_num = ThreadPool::number;
      pthread_mutex_unlock(&pool->lock);
      for (int i = 0; i < ThreadPool::number; i++)
      {
        pthread_cond_signal(&pool->not_empty);
      }
    }
  }
  return nullptr;
}

// 线程退出
template <typename T>
void ThreadPool<T>::threadExit()
{
  pthread_t tid = pthread_self();
  for (int i = 0; i < this->max_num; i++)
  {
    if (this->thread_id[i] == tid)
    {
      // std::cout << "threadExit() function: thread " << std::to_string(tid) << " exiting..." << std::endl;
      this->thread_id[i] = 0;
      break;
    }
  }
  pthread_exit(NULL);
}