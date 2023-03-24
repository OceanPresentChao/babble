#ifndef TASK_QUEUE_H
#define TASK_QUEUE_H

#include <queue>
#include <pthread.h>

using callback = void (*)(int arg1, int arg2);

template <typename T>
class Task
{
public:
  callback function;
  T arg1;
  T arg2;

public:
  Task()
  {
    function = nullptr;
    arg1 = -1;
    arg2 = -1;
  };
  ~Task() = default;
  Task(callback fn, int arg1, int arg2)
  {
    this->function = fn;
    this->arg1 = arg1;
    this->arg2 = arg2;
  };
};

template <typename T>
class TaskQueue
{
public:
  TaskQueue();
  ~TaskQueue();
  void addTask(Task<T> task);
  void addTask(callback fn, int arg1, int arg2);
  Task<T> *getTask();
  inline size_t getTaskNum()
  {
    return task_queue.size();
  }

private:
  std::queue<Task<T>> task_queue;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
};

#endif