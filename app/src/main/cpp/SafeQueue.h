//
// Created by Administrator on 2020/9/3.
//

#ifndef MY_APPLICATION_SAFEQUEUE_H
#define MY_APPLICATION_SAFEQUEUE_H

#include <pthread.h>
#include <queue>

#ifdef C11
#include <thread>
#endif

using namespace std;

template<typename T> //模板.类似泛型？
class SafeQueue {

    typedef void (*ReleaseCallback)(T &);//函数指针

    typedef void (*SyncHandle)(queue<T> &);

public:
    SafeQueue() {
#ifdef C11
#else
        //初始化
        pthread_mutex_init(&mutex, nullptr);
        pthread_cond_init(&cond, nullptr);
#endif
    }

    ~SafeQueue() {
#ifdef C11
#else
        //回收内存
        pthread_cond_destroy(&cond);
        pthread_mutex_destroy(&mutex);
#endif
    }

    void enQueue(T new_value) {
        //锁 类似智能指针， 自动释放
        push(new_value);
    }

    int deQueue(T &value) {
        int ret = pop(value);
        return ret;
    }

    void push(T new_value) {
#ifdef C11
        //锁 和智能指针原理类似，自动释放
        lock_guard<mutex> lk(mt);
        if (work) {
            q.push(new_value);
            cv.notify_one();
        }
#else
        pthread_mutex_lock(&mutex);
        if (work) {
            q.push(new_value);
            pthread_cond_signal(&cond);
        } else {
            releaseCallback(new_value);
        }
        pthread_mutex_unlock(&mutex);
#endif
    }

    int pop(T &value) {
        int ret = 0;
#ifdef C11
        //占用空间相对lock_guard 更大一点且相对更慢一点，但是配合条件必须使用它，更灵活
        unique_lock<mutex> lk(mt);
        //第二个参数 lambda表达式：false则不阻塞 往下走
        cv.wait(lk,[this]{return !work || !q.empty();});
        if (!q.empty()) {
            value = q.front();
            q.pop();
            ret = 1;
        }
#else
        pthread_mutex_lock(&mutex);
        while (work && q.empty()) {
            pthread_cond_wait(&cond, &mutex);
        }
        if (!q.empty()) {
            value = q.front();
            q.pop();
            ret = 1;
        }
        pthread_mutex_unlock(&mutex);
#endif
        return ret;
    }

    void setWork(int work) {
#ifdef C11
        lock_guard<mutex> lk(mt);
        this->work = work;
#else
        pthread_mutex_lock(&mutex);
        this->work = work;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
#endif

    }

    int empty() {
        return q.empty();
    }

    int size() {
        return q.size();
    }

    void clear() {
#ifdef C11
        lock_guard<mutex> lk(mt);
        int size = q.size();
        for (int i = 0; i < size; ++i) {
            T value = q.front();
            releaseCallback(value);
            q.pop();
        }
#else
        pthread_mutex_lock(&mutex);
        int size = q.size();
        for (int i = 0; i < size; ++i) {
            T value = q.front();
            releaseCallback(value);
            q.pop();
        }
        pthread_mutex_unlock(&mutex);
#endif
    }

    void sync() {
#ifdef C11
        lock_guard<mutex> lk(mt);
        syncHandle(q);
#else
        pthread_mutex_lock(&mutex);
        syncHandle(q);
        pthread_mutex_unlock(&mutex);
#endif
    }

    void setReleaseCallback(ReleaseCallback r) {
        releaseCallback = r;
    }

    void setSyncHandle(SyncHandle s) {
        syncHandle = s;
    }

private:
#ifdef C11
    mutex mt;
    condition_variable cv;
#else
    pthread_cond_t cond;
    pthread_mutex_t mutex;
#endif

    queue<T> q;
    int work;
    ReleaseCallback releaseCallback;
    SyncHandle syncHandle;
};


#endif //MY_APPLICATION_SAFEQUEUE_H
