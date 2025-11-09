#pragma once
#include <string>
#include <memory>

using MsgId = std::string;

// ===== Base =====
class ProtocolMessage {
public:
    explicit ProtocolMessage(MsgId id) : id_(std::move(id)) {}
    virtual ~ProtocolMessage() = default;

    const MsgId& id() const { return id_; }

    static bool hasPrefix(const MsgId& id, const char* prefix) {
        return id.rfind(prefix, 0) == 0; // starts_with
    }

private:
    MsgId id_;
};

class IOEventMsg : public ProtocolMessage {
public:
    IOEventMsg(MsgId id, std::string signal)
        : ProtocolMessage(std::move(id)), signal_(std::move(signal)) {}
    const std::string& signal() const { return signal_; }
private:
    std::string signal_;
};

class MESReqMsg : public ProtocolMessage {
public:
    MESReqMsg(MsgId id, std::string op, std::string arg)
        : ProtocolMessage(std::move(id)), op_(std::move(op)), arg_(std::move(arg)) {}
    const std::string& op()  const { return op_;  }
    const std::string& arg() const { return arg_; }
private:
    std::string op_;
    std::string arg_;
};

class MESReplyMsg : public ProtocolMessage {
public:
    MESReplyMsg(MsgId id, bool ok, std::string detail)
        : ProtocolMessage(std::move(id)), ok_(ok), detail_(std::move(detail)) {}
    bool ok() const { return ok_; }
    const std::string& detail() const { return detail_; }
private:
    bool ok_;
    std::string detail_;
};

using ProtoPtr = std::unique_ptr<ProtocolMessage>;
