#pragma once
#include "Queue.hpp"
#include "Messages.hpp" 
#include "MESClient.hpp"
#include "protocol/Protocol.hpp"
#include <thread>
#include <memory>
#include <iostream>

struct MESManager {
    MPSCQueue<InboxMsg>& main_inbox;
    SPSCQueue<ProtoPtr>& mesmgr_in;
    std::jthread th;
    MESClient client;

    MESManager(MPSCQueue<InboxMsg>& inbox, SPSCQueue<ProtoPtr>& in)
        : main_inbox(inbox), mesmgr_in(in),
          client([this](auto corr, bool ok, std::string detail){
              main_inbox.push(MESReply{corr, ok, detail});
          }) {}

    void start(){ th = std::jthread([this](std::stop_token st){ loop(st); }); }
    void stop(){ th.request_stop(); if (th.joinable()) th.join(); }

private:
    void loop(std::stop_token st) {
        while (!st.stop_requested()) {
            ProtoPtr msg;
            if (!mesmgr_in.wait_pop(st, msg)) break;
            if (!msg) continue;

            const auto& id = msg->id();

            if (ProtocolMessage::hasPrefix(id, "REQ-")) {
                if (auto* req = dynamic_cast<MESReqMsg*>(msg.get())) {
                    std::string payload = req->op() + ":" + req->arg();
                    client.sendAsync(req->id(), payload);
                } else {
                    std::cerr << "[MESManager] REQ- prefix but not MESReqMsg\n";
                }
            }
            else if (ProtocolMessage::hasPrefix(id, "EVT-")) {
                if (auto* ev = dynamic_cast<IOEventMsg*>(msg.get())) {
                    client.sendAsync(ev->id(), "report:" + ev->signal());
                } else {
                    std::cerr << "[MESManager] EVT- prefix but not IOEventMsg\n";
                }
            }
            else {
                std::cerr << "[MESManager] Unknown id prefix: " << id << "\n";
            }
        }
    }
};
