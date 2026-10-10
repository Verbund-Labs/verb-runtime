#pragma once

#include <vector>
#include <deque>
#include <functional>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <atomic>
#include <future>
#include "verbrt/value.hpp"

namespace verbrt {

enum class PromiseState {
    Pending,
    Fulfilled,
    Rejected
};

class Promise : public std::enable_shared_from_this<Promise> {
public:
    using ResolveFn = std::function<void(const Value&)>;
    using RejectFn = std::function<void(const Value&)>;

    PromiseState state = PromiseState::Pending;
    Value result;
    Value reason;
    std::vector<std::function<void(const Value&)>> fulfill_callbacks;
    std::vector<std::function<void(const Value&)>> reject_callbacks;
    std::mutex mtx;

    Promise() = default;

    void resolve(const Value& val) {
        std::lock_guard<std::mutex> lock(mtx);
        if (state != PromiseState::Pending) return;
        state = PromiseState::Fulfilled;
        result = val;
        for (auto& cb : fulfill_callbacks) {
            cb(result);
        }
        fulfill_callbacks.clear();
    }

    void reject(const Value& err) {
        std::lock_guard<std::mutex> lock(mtx);
        if (state != PromiseState::Pending) return;
        state = PromiseState::Rejected;
        reason = err;
        for (auto& cb : reject_callbacks) {
            cb(reason);
        }
        reject_callbacks.clear();
    }

    std::shared_ptr<Promise> then(std::function<Value(const Value&)> on_fulfilled) {
        auto next_promise = std::make_shared<Promise>();
        std::lock_guard<std::mutex> lock(mtx);
        if (state == PromiseState::Fulfilled) {
            try {
                Value res = on_fulfilled(result);
                next_promise->resolve(res);
            } catch (const std::exception& e) {
                next_promise->reject(Value(e.what()));
            }
        } else if (state == PromiseState::Pending) {
            fulfill_callbacks.push_back([next_promise, on_fulfilled](const Value& val) {
                try {
                    Value res = on_fulfilled(val);
                    next_promise->resolve(res);
                } catch (const std::exception& e) {
                    next_promise->reject(Value(e.what()));
                }
            });
        }
        return next_promise;
    }

    Value await_result() {
        while (true) {
            {
                std::lock_guard<std::mutex> lock(mtx);
                if (state == PromiseState::Fulfilled) return result;
                if (state == PromiseState::Rejected) throw std::runtime_error("Promise rejected: " + reason.to_string());
            }
            std::this_thread::yield();
        }
    }
};

class EventLoop {
public:
    static EventLoop& instance() {
        static EventLoop loop;
        return loop;
    }

    void queue_microtask(std::function<void()> task) {
        std::lock_guard<std::mutex> lock(mtx_);
        microtasks_.push_back(std::move(task));
    }

    void post_task(std::function<void()> task) {
        std::lock_guard<std::mutex> lock(mtx_);
        macrotasks_.push_back(std::move(task));
    }

    template <typename F>
    std::shared_ptr<Promise> spawn_async(F&& fn) {
        auto promise = std::make_shared<Promise>();
        std::thread([promise, fn = std::forward<F>(fn)]() {
            try {
                Value res = fn();
                promise->resolve(res);
            } catch (const std::exception& e) {
                promise->reject(Value(e.what()));
            }
        }).detach();
        return promise;
    }

    void run_until_complete() {
        while (true) {
            std::function<void()> task;
            {
                std::lock_guard<std::mutex> lock(mtx_);
                if (!microtasks_.empty()) {
                    task = std::move(microtasks_.front());
                    microtasks_.pop_front();
                } else if (!macrotasks_.empty()) {
                    task = std::move(macrotasks_.front());
                    macrotasks_.pop_front();
                } else {
                    break;
                }
            }
            if (task) {
                task();
            }
        }
    }

private:
    std::deque<std::function<void()>> microtasks_;
    std::deque<std::function<void()>> macrotasks_;
    std::mutex mtx_;
};

} // namespace verbrt
