#pragma once
#include "../Queue.hpp"
#include "../Messages.hpp"
#include <thread>
#include <atomic>
#include <format>
#include <chrono>

struct IOWorker {
    MPSCQueue<InboxMsg>& main_inbox;
    SPSCQueue<IOCmd>&    io_in;
    std::jthread th;
    std::atomic<int> seq{0};

    IOWorker(MPSCQueue<InboxMsg>& inbox, SPSCQueue<IOCmd>& in)
        : main_inbox(inbox), io_in(in) {}

    void start(){ th = std::jthread([this](std::stop_token st){ loop(st); }); }
    void stop(){ th.request_stop(); if (th.joinable()) th.join(); }

private:
    void loop(std::stop_token st){
        using namespace std::chrono_literals;
        while (!st.stop_requested()) {
            std::this_thread::sleep_for(500ms);
            int id = ++seq;
            IOEvent event{
                .msgId  = std::format("MSG-{:04}", id),
                .signal = (id % 2 ? "LOAD" : "UNLOAD"),
                .ts     = std::chrono::system_clock::now()
            };
            main_inbox.push(std::move(event));
        }
    }
};
