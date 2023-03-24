#include "TaskQueue.h"

template <typename T>
TaskQueue<T>::TaskQueue()
{
  pthread_mutex_init(&mutex, NULL);
  pthread_cond_init(&cond, NULL);
}

template <typename T>
TaskQueue<T>::~TaskQueue()
{
  pthread_mutex_destroy(&mutex);
  pthread_cond_destroy(&cond);
}

template <typename T>
void TaskQueue<T>::addTask(Task<T> task)
{
  pthread_mutex_lock(&mutex);
  task_queue.push(task);
  pthread_mutex_unlock(&mutex);
}

template <typename T>
void TaskQueue<T>::addTask(callback fn, int arg1, int arg2)
{
  Task<T> task(fn, arg1, arg2);
  addTask(task);
}

template <typename T>
Task<T> *TaskQueue<T>::getTask()
{
  Task<T> *task = nullptr;
  pthread_mutex_lock(&mutex);
  if (!this->task_queue.empty())
  {

    task = task_queue.front();
    task_queue.pop();
  }
  pthread_mutex_unlock(&mutex);
  return task;
}