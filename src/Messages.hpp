#pragma once
#include <string>
#include <chrono>
#include <variant>

using MsgId = std::string;

struct IOEvent {
    MsgId msgId;
    std::string signal;
    std::chrono::system_clock::time_point ts;
};

struct MESReq {
    MsgId msgId;
    std::string payload;
};

struct MESReply {
    MsgId msgId;
    bool success;
    std::string detail;
};

using InboxMsg = std::variant<IOEvent, MESReply>;
using IOCmd    = std::variant<std::monostate>;
using MESCmd   = std::variant<MESReq>;
