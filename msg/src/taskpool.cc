#include "taskpool.h"
#include <mutex>
#include <condition_variable>
#include <functional>
#include <queue>
#include <thread>

namespace msg{

taskpool& taskpool::instance(){
    static taskpool tp(std::thread::hardware_concurrency());
    return tp;
}

struct taskpool::meta {
    std::mutex mtx_;
    std::condition_variable cond_;
    bool is_shutdown_ = false;
    std::queue<task_cb> tasks_;
};

taskpool::taskpool(size_t nr_thread) : meta_(std::make_shared<meta>()) {
    for (size_t i = 0; i < nr_thread; i++) {
        std::thread([this]{
            std::unique_lock<std::mutex> lk(meta_->mtx_);
            while(!meta_->is_shutdown_) {
                if (!meta_->tasks_.empty()) {
                    auto current = std::move(meta_->tasks_.front());
                    meta_->tasks_.pop();
                    lk.unlock();
                    current();
                    lk.lock();
                } else {
                    meta_->cond_.wait(lk);
                }
            }
        }).detach();
    }
}

taskpool::~taskpool() {
    if ((bool) meta_) {
        {
            std::lock_guard<std::mutex> lk(meta_->mtx_);
            meta_->is_shutdown_ = true;
        }
        meta_->cond_.notify_all();
    }
}

void taskpool::execute(const task_cb& task) {
    {
        std::lock_guard<std::mutex> lk(meta_->mtx_);
        meta_->tasks_.emplace(task);
    }
    meta_->cond_.notify_one();
}


}