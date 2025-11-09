#include "Queue.hpp"
#include "Messages.hpp"
#include "workers/IOWorker.hpp"
#include "MESManager.hpp"
#include "protocol/Protocol.hpp"
#include <iostream>
#include <thread>
#include <format>
#include <atomic>

int main(){
    const size_t MAIN_INBOX_CAP = 2000;
    const size_t IO_IN_CAP      = 2000;
    const size_t MESMGR_IN_CAP  = 2000;

    MPSCQueue<InboxMsg> main_inbox{MAIN_INBOX_CAP};
    SPSCQueue<IOCmd>    io_in{IO_IN_CAP};
    SPSCQueue<ProtoPtr> mesmgr_in{MESMGR_IN_CAP}; 

    IOWorker  io{main_inbox, io_in};
    MESManager mesmgr{main_inbox, mesmgr_in};

    io.start();
    mesmgr.start();

    std::atomic<int> seq{0};

    std::jthread hub([&](std::stop_token st){
        while (!st.stop_requested()) {
            InboxMsg m;
            if (!main_inbox.wait_pop(st, m)) break;

            if (auto* ev = std::get_if<IOEvent>(&m)) {
                std::cout << "[HUB] IOEvent " << ev->msgId << " " << ev->signal << "\n";

                int n = ++seq;
                MsgId rid = std::format("REQ-{:04}", n);
                auto p = std::make_unique<MESReqMsg>(rid, "reportSignal", ev->signal);
                mesmgr_in.push(std::move(p));
            }
            else if (auto* rep = std::get_if<MESReply>(&m)) {
                std::cout << "[HUB] MESReply " << rep->msgId
                          << " ok=" << std::boolalpha << rep->success
                          << " detail=" << rep->detail << "\n";
            }
        }
    });

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(6s);

    hub.request_stop(); hub.join();
    io.stop(); mesmgr.stop();
    return 0;
}
