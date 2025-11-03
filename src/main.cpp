#include "Queue.hpp"
#include "Messages.hpp"
#include "workers/IOWorker.hpp"
#include "workers/MESWorker.hpp"
#include <iostream>
#include <thread>
#include <stop_token>

int main(){
    const size_t MAIN_INBOX_CAP = 2000;  // 워커 -> 메인 (MPSC)
    const size_t IO_IN_CAP      = 2000;  // 메인 -> IO (SPSC)
    const size_t MES_IN_CAP     = 2000;  // 메인 -> MES (SPSC)

    MPSCQueue<InboxMsg> main_inbox{MAIN_INBOX_CAP};
    SPSCQueue<IOCmd>    io_in{IO_IN_CAP};
    SPSCQueue<MESCmd>   mes_in{MES_IN_CAP};

    IOWorker  io{main_inbox, io_in};
    MESWorker mes{main_inbox, mes_in};

    io.start();
    mes.start();

    std::jthread hub([&](std::stop_token st){
        while (!st.stop_requested()) {
            InboxMsg m;
            if (!main_inbox.wait_pop(st, m)) break;

            if (auto* ev = std::get_if<IOEvent>(&m)) {
                std::cout << "[HUB] IOEvent " << ev->msgId << " " << ev->signal << "\n";
                mes_in.push(MESReq{ev->msgId, "payload-" + ev->signal});
            }
            else if (auto* rep = std::get_if<MESReply>(&m)) {
                std::cout << "[HUB] MESReply " << rep->msgId << " ok=" << std::boolalpha << rep->success
                          << " detail=" << rep->detail << "\n";
            }
        }
    });

    std::this_thread::sleep_for(std::chrono::seconds(6));
    hub.request_stop(); hub.join();
    io.stop(); mes.stop();
    return 0;
}
