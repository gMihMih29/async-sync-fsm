#pragma once

#include "Channel.hpp"
#include "DeterministicFSM.hpp"

namespace fsm {
template <typename Key>
class CFSM {
public:
    enum class Action {
        NotDefined,
        Nothing,
        Receive,
        Transmit,
    };

    struct ChannelConnection {
        Channel<Key>* channel;
        Action action;
        Key key;
    };

    explicit CFSM(const std::vector<Key>& keys) : fsm_(keys) {}

    void SetTransition(const std::string& node_name1,
                       const std::string& node_name2, const Key& key,
                       Action act) {
        fsm_.SetTransition(node_name1, node_name2, key);
        actions_[node_name1][node_name2] = {act, key};
    }

    void AddNode(const std::string& node_name) { fsm_.AddNode(node_name); }

    void SetInitialNode(const std::string& node_name) {
        fsm_.SetInitialNode(node_name);
    }

    bool Move(const Key& key) {
        std::string next_node_name = fsm_.GetCurrentNode().GetTransition(key);
        auto act = actions_[fsm_.GetState()][next_node_name];
        if (act.first == Action::NotDefined || act.first == Action::Receive) {
            return false;
        }
        if (act.first == Action::Transmit) {
            input_channel_->SendMessage(key);
        }
        fsm_.Move(key);
        return true;
    }

    std::optional<Key> LookUpMessage() const {
        return input_channel_->LookUpMsg();
    }

    std::list<Key> LookUpMoves() {
        std::list<Key> ans;
        std::string cur_state = fsm_.GetState();
        for (const auto& i : actions_[cur_state]) {
            if (i.second.first == Action::Transmit) {
                ans.push_back(i.second.second);
            }
        }
        return ans;
    }

    bool WaitForMsgAndMove() {
        bool has_receive_action = false;
        for (auto& i : actions_[fsm_.GetState()]) {
            if (i.second.first == Action::Receive) {
                has_receive_action = true;
                break;
            }
        }
        if (!has_receive_action) {
            return false;
        }
        fsm_.Move(input_channel_->ReceiveMessage());
        return true;
    }

    const std::string& GetState() const { return fsm_.GetState(); }

    void SetInputChannel(Channel<Key>* ch) { input_channel_ = ch; }

    void SetOutputChannel(Channel<Key>* ch) { output_channel_ = ch; }

private:
    DeterministicFSM<Key> fsm_;
    std::map<std::string, std::map<std::string, std::pair<Action, Key>>>
        actions_;
    Channel<Key>* input_channel_;
    Channel<Key>* output_channel_;
};
}  // namespace fsm
