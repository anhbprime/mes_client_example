#pragma once
#include <string>
#include <thread>
#include <chrono>
#include <functional>

struct MESClient {
    using Callback = std::function<void(std::string, bool, std::string)>;
    Callback cb;

    MESClient(Callback c) : cb(std::move(c)) {}

    void sendAsync(std::string corr, std::string payload) {
        std::thread([corr=std::move(corr), payload, cb=this->cb]{
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            cb(corr, true, "OK");
        }).detach();
    }
};
