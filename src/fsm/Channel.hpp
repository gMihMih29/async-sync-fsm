#pragma once

#include <barrier>
#include <iostream>
#include <latch>
#include <mutex>
#include <optional>
#include <queue>
#include <utility>

namespace fsm {

template <typename MessageType>
class Channel {
public:
    virtual void SendMessage(MessageType msg) = 0;
    virtual MessageType ReceiveMessage() = 0;
    virtual bool HasMessage() const = 0;
    virtual std::optional<MessageType> LookUpMsg() const = 0;
};

template <typename MessageType>
class AsyncChannel : public Channel<MessageType> {
public:
    AsyncChannel() { mutex_.lock(); }

    void SendMessage(MessageType msg) override {
        messages_.push(msg);
        mutex_.unlock();
    }

    MessageType ReceiveMessage() override {
        if (messages_.empty()) {
            mutex_.lock();
            mutex_.unlock();
        }
        MessageType msg = messages_.front();
        messages_.pop();
        if (messages_.empty()) {
            mutex_.lock();
        }
        return msg;
    }

    bool HasMessage() const override { return !messages_.empty(); }

    std::optional<MessageType> LookUpMsg() const override {
        if (messages_.empty()) {
            return {};
        }
        return {messages_.front()};
    }

private:
    std::queue<MessageType> messages_;
    std::mutex mutex_;
};

template <typename MessageType>
class SyncChannel : public Channel<MessageType> {
    struct Empty {
        void operator()() {}
    };

public:
    SyncChannel() : sync_point_(2, Empty()) {
        mutex_recv_.lock();
        mutex_send_.lock();
    }

    void SendMessage(MessageType msg) override {
        last_message_ = {msg};
        sync_point_.arrive_and_wait();
    }

    MessageType ReceiveMessage() override {
        sync_point_.arrive_and_wait();
        MessageType msg = last_message_.value();
        return msg;
    }

    bool HasMessage() const override { return last_message_.has_value(); }

    std::optional<MessageType> LookUpMsg() const override {
        return last_message_;
    }

private:
    std::optional<MessageType> last_message_;
    std::mutex mutex_send_;
    std::mutex mutex_recv_;
    std::barrier<Empty> sync_point_;
};
}  // namespace fsm
