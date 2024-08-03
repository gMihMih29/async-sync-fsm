#include <cassert>
#include <cstdlib>
#include <iostream>
#include <map>
#include <thread>
#include <utility>
#include <vector>

#include "CommunicatingFSM.hpp"

template <typename MessageType>
class AsyncResearchChannel : public fsm::Channel<MessageType> {
public:
    void SendMessage(MessageType msg) override { messages_.push(msg); }

    std::optional<MessageType> ReceiveMessage() override {
        if (messages_.empty()) {
            return {};
        }
        MessageType msg = messages_.front();
        messages_.pop();
        return msg;
    }

    bool HasMessage() const override { return !messages_.empty(); }

    std::optional<MessageType> LookUpMsg() const override {
        if (messages_.empty()) {
            return {};
        }
        return {messages_.front()};
    }

    bool operator==(const AsyncResearchChannel& other) const {
        return other.messages_ == messages_;
    }

private:
    std::queue<MessageType> messages_;
};

template<class T>
class SearchState {
public:
    SearchState(const fsm::CFSM<T>& first, const fsm::CFSM<T>& second,
                const AsyncResearchChannel<T>& first_channel,
                const AsyncResearchChannel<T>& sec_channel)
        : fsm_1(first),
          fsm_2(second),
          channel_1_2(first_channel),
          channel_2_1(sec_channel) {
        fsm_1.SetInputChannel(&channel_2_1);
        fsm_1.SetOutputChannel(&channel_1_2);
        fsm_2.SetInputChannel(&channel_1_2);
        fsm_2.SetOutputChannel(&channel_2_1);
    }

    void init() {
        fsm_1.SetInputChannel(&channel_2_1);
        fsm_1.SetOutputChannel(&channel_1_2);
        fsm_2.SetInputChannel(&channel_1_2);
        fsm_2.SetOutputChannel(&channel_2_1);
    }

    bool operator==(const SearchState<T>& other) const {
        bool cmp_states = fsm_1.GetState() == other.fsm_1.GetState() &&
                          fsm_2.GetState() == other.fsm_2.GetState();
        bool cmp_channel = channel_1_2 == other.channel_1_2 &&
                           channel_2_1 == other.channel_2_1;
        return cmp_states && cmp_channel;
    }

    fsm::CFSM<T> fsm_1;
    fsm::CFSM<T> fsm_2;
    AsyncResearchChannel<T> channel_1_2;
    AsyncResearchChannel<T> channel_2_1;
};

template<class T>
int index_of_state(const SearchState<T>& state,
                   const std::vector<SearchState<T>>& memory) {
    for (int i = 0; i < memory.size(); ++i) {
        if (state == memory[i]) {
            return i;
        }
    }
    return -1;
}

template<class T>
void bfs(fsm::CFSM<T> first, fsm::CFSM<T> second) {
    SearchState begin(first, second, AsyncResearchChannel<T>(),
                      AsyncResearchChannel<T>());
    begin.init();
    std::queue<SearchState<T>> q;
    std::vector<std::vector<int>> edges;
    q.push(begin);
    std::cout << "Begin bfs\n";
    std::vector<SearchState<T>> memory;
    std::vector<bool> used;
    memory.push_back(begin);
    used.push_back(false);
    edges.push_back({});
    while (!q.empty()) {
        SearchState<T> s = q.front();
        q.pop();
        s.init();
        int cur = index_of_state(s, memory);
        assert(cur != -1 && "cur == -1");
        if (used[cur]) {
            continue;
        }
        used[cur] = true;
        std::cout << s.fsm_1.GetState() << " " << s.fsm_2.GetState() << ": "
                  << cur << "\n";
        auto first_moves = s.fsm_1.LookUpMoves();
        auto second_moves = s.fsm_2.LookUpMoves();
        for (auto i : first_moves) {
            SearchState next(s.fsm_1, s.fsm_2, s.channel_1_2, s.channel_2_1);
            next.fsm_1.Move(i);
            q.push(next);
            int next_ind = index_of_state(next, memory);
            if (next_ind == -1) {
                next_ind = memory.size();
                memory.push_back(next);
                edges.push_back({});
                used.push_back(false);
            }
            std::cout << "moved to " << next.fsm_1.GetState() << " "
                      << next.fsm_2.GetState() << " " << next_ind << "\n";
            edges[cur].push_back(next_ind);
        }
        for (auto i : second_moves) {
            SearchState<T> next(s.fsm_1, s.fsm_2, s.channel_1_2, s.channel_2_1);
            next.fsm_2.Move(i);
            q.push(next);
            int next_ind = index_of_state(next, memory);
            if (next_ind == -1) {
                next_ind = memory.size();
                memory.push_back(next);
                edges.push_back({});
                used.push_back(false);
            }
            std::cout << "moved to " << next.fsm_1.GetState() << " "
                      << next.fsm_2.GetState() << " " << next_ind << "\n";
            edges[cur].push_back(next_ind);
        }
        if (s.fsm_1.LookUpMessage().has_value()) {
            SearchState<T> next(s.fsm_1, s.fsm_2, s.channel_1_2, s.channel_2_1);
            next.fsm_1.WaitForMsgAndMove();
            q.push(next);
            int next_ind = index_of_state(next, memory);
            if (next_ind == -1) {
                next_ind = memory.size();
                memory.push_back(next);
                edges.push_back({});
                used.push_back(false);
            }
            std::cout << "moved to " << next.fsm_1.GetState() << " "
                      << next.fsm_2.GetState() << " " << next_ind << "\n";
            edges[cur].push_back(next_ind);
        }
        if (s.fsm_2.LookUpMessage().has_value()) {
            SearchState<T> next(s.fsm_1, s.fsm_2, s.channel_1_2, s.channel_2_1);
            next.fsm_2.WaitForMsgAndMove();
            q.push(next);
            int next_ind = index_of_state(next, memory);
            if (next_ind == -1) {
                next_ind = memory.size();
                memory.push_back(next);
                edges.push_back({});
                used.push_back(false);
            }
            std::cout << "moved to " << next.fsm_1.GetState() << " "
                      << next.fsm_2.GetState() << " " << next_ind << "\n";
            edges[cur].push_back(next_ind);
        }
    }
    std::cout << "End of bfs\n\n\n";
    std::cout << "Possible states:\n";
    std::vector<std::string> states;
    for (int i = 0; i < memory.size(); ++i) {
        states.push_back(memory[i].fsm_1.GetState() + " " +
                         memory[i].fsm_2.GetState() + " " + std::to_string(i));
    }
    for (int i = 0; i < states.size(); ++i) {
        std::cout << states[i] << "\n";
    }
    std::cout << "Connections:\n";
    for (int i = 0; i < edges.size(); ++i) {
        std::cout << i << ": ";
        for (auto j : edges[i]) {
            std::cout << j << " ";
        }
        std::cout << "\n";
    }
}
