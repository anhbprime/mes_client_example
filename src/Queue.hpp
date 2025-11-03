#pragma once
#include <vector>
#include <optional>
#include <mutex>
#include <condition_variable>
#include <stop_token>
#include <chrono>
#include <thread>

// -------------------- MPSC (Multi-Producer, Single-Consumer) --------------------
template <class T>
class MPSCQueue {
public:
    explicit MPSCQueue(size_t capacity)
    : cap_(capacity), buf_(capacity) {}

    // Drop-Oldest: 꽉 찼으면 head를 한 칸 앞으로 (가장 오래된 항목 폐기)
    void push(T v) {
        std::lock_guard lk(mu_);
        if (cap_ == 0) return; // 방어
        if (size_ == cap_) {
            head_ = (head_ + 1) % cap_;
            // size_ 그대로(cap) 유지: 가장 오래된 것 덮어씀
        } else {
            ++size_;
        }
        buf_[tail_] = std::move(v);
        tail_ = (tail_ + 1) % cap_;
        cv_.notify_one();
    }

    std::optional<T> try_pop() {
        std::lock_guard lk(mu_);
        if (size_ == 0) return std::nullopt;
        T out = std::move(buf_[head_]);
        head_ = (head_ + 1) % cap_;
        --size_;
        return out;
    }

    template <class StopToken>
    bool wait_pop(StopToken st, T& out) {
        std::unique_lock lk(mu_);
        cv_.wait(lk, st, [&]{ return size_ > 0; });
        if (st.stop_requested()) return false;
        out = std::move(buf_[head_]);
        head_ = (head_ + 1) % cap_;
        --size_;
        return true;
    }

    size_t capacity() const { return cap_; }
    size_t size() const { std::lock_guard lk(mu_); return size_; }

private:
    const size_t cap_;
    std::vector<T> buf_;
    size_t head_ = 0;
    size_t tail_ = 0;
    size_t size_ = 0;
    mutable std::mutex mu_;
    std::condition_variable_any cv_;
};

// -------------------- SPSC (Single Producer, Single Consumer) --------------------
template <class T>
class SPSCQueue {
public:
    explicit SPSCQueue(size_t capacity)
    : cap_(capacity), buf_(capacity) {}

    void push(T v) {
        std::lock_guard lk(mu_);
        if (cap_ == 0) return;
        if (size_ == cap_) {
            head_ = (head_ + 1) % cap_; // drop oldest
        } else {
            ++size_;
        }
        buf_[tail_] = std::move(v);
        tail_ = (tail_ + 1) % cap_;
        cv_.notify_one();
    }

    std::optional<T> try_pop() {
        std::lock_guard lk(mu_);
        if (size_ == 0) return std::nullopt;
        T out = std::move(buf_[head_]);
        head_ = (head_ + 1) % cap_;
        --size_;
        return out;
    }

    template <class StopToken>
    bool wait_pop(StopToken st, T& out) {
        std::unique_lock lk(mu_);
        cv_.wait(lk, st, [&]{ return size_ > 0; });
        if (st.stop_requested()) return false;
        out = std::move(buf_[head_]);
        head_ = (head_ + 1) % cap_;
        --size_;
        return true;
    }

    size_t capacity() const { return cap_; }
    size_t size() const { std::lock_guard lk(mu_); return size_; }

private:
    const size_t cap_;
    std::vector<T> buf_;
    size_t head_ = 0;
    size_t tail_ = 0;
    size_t size_ = 0;
    mutable std::mutex mu_;
    std::condition_variable_any cv_;
};
