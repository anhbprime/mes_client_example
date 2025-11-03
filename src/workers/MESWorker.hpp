#pragma once
#include "../Queue.hpp"
#include "../Messages.hpp"
#include "../MESClient.hpp"
#include <thread>

struct MESWorker {
    MPSCQueue<InboxMsg>& main_inbox;
    SPSCQueue<MESCmd>&   mes_in;
    std::jthread th;
    MESClient client;

    MESWorker(MPSCQueue<InboxMsg>& inbox, SPSCQueue<MESCmd>& in)
        : main_inbox(inbox), mes_in(in),
          client([this](auto corr, bool ok, std::string detail){
              main_inbox.push(MESReply{corr, ok, std::move(detail)});
          }) {}

    void start(){ th = std::jthread([this](std::stop_token st){ loop(st); }); }
    void stop(){ th.request_stop(); if (th.joinable()) th.join(); }

private:
    void loop(std::stop_token st){
        while (!st.stop_requested()) {
            MESCmd cmd;
            if (!mes_in.wait_pop(st, cmd)) break;
            if (auto* req = std::get_if<MESReq>(&cmd)) {
                client.sendAsync(req->msgId, req->payload);
            }
        }
    }
};
