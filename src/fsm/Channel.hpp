#pragma once

#include <barrier>
#include <mutex>
#include <optional>
#include <queue>
#include <iostream>

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
public:
    SyncChannel() {
        mutex_read_.lock();
    }

    void SendMessage(MessageType msg) override {
        mutex_write_.lock();
        last_message_ = {msg};
        std::cout << "Sent msg\n";
        mutex_read_.unlock();
        mutex_write_.lock();
        mutex_read_.lock();
    }

    MessageType ReceiveMessage() override {
        std::cout << "Receiving msg\n";
        mutex_read_.lock();
        MessageType msg = last_message_.value();
        mutex_write_.unlock();
        return msg;
    }

    bool HasMessage() const override { return last_message_.has_value(); }

    std::optional<MessageType> LookUpMsg() const override {
        return last_message_;
    }

private:
    std::optional<MessageType> last_message_;
    std::mutex mutex_write_;
    std::mutex mutex_read_;
};
}  // namespace fsm
