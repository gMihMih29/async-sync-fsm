#pragma once

#include <cassert>
#include <list>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace fsm {
template <typename Key>
class Node {
public:
    Node() : name_(""), transitions_({}) {}

    Node(const std::string& name, std::vector<Key> keys) : name_(name) {
        for (int i = 0; i < keys.size(); ++i) {
            transitions_.emplace_back(keys[i], name);
        }
    }

    void SetTransition(const Key& key, const std::string& node_name) {
        std::pair<Key, std::string>& transition = FindTransition_(key);
        transition.second = node_name;
    }

    const std::string& GetTransition(const Key& key) const {
        return FindTransition_(key).second;
    }

    const std::string& GetName() const { return name_; }

private:
    std::pair<Key, std::string>& FindTransition_(Key key) {
        for (auto& i : transitions_) {
            if (i.first == key) {
                return i;
            }
        }
        assert(false);
    }

    const std::pair<Key, std::string>& FindTransition_(Key key) const {
        for (auto& i : transitions_) {
            if (i.first == key) {
                return i;
            }
        }
        assert(false);
    }

    std::list<std::pair<Key, std::string>> transitions_;
    std::string name_;
};

template <typename Key>
class DeterministicFSM {
public:
    explicit DeterministicFSM(const std::vector<Key>& keys) : keys_(keys) {}

    void SetTransition(const std::string& node_name1,
                       const std::string& node_name2, const Key& key) {
        assert(nodes_.find(node_name1) != nodes_.end() &&
               "There is no node with node_name1");
        assert(nodes_.find(node_name2) != nodes_.end() &&
               "There is no node with node_name2");
        nodes_[node_name1].SetTransition(key, node_name2);
    }

    void AddNode(const std::string& node_name) {
        nodes_[node_name] = Node<Key>(node_name, keys_);
    }

    void SetInitialNode(const std::string& node_name) {
        assert(nodes_.find(node_name) != nodes_.end() &&
               "There is no node with node_name");
        current_node_ = &nodes_[node_name];
    }

    const std::string& GetState() const { return current_node_->GetName(); }

    const Node<Key>& GetCurrentNode() const { return *current_node_; }

    void Move(const Key& key) {
        current_node_ = &nodes_[current_node_->GetTransition(key)];
    }

private:
    std::map<std::string, Node<Key>> nodes_;
    std::vector<Key> keys_;
    Node<Key>* current_node_;
};
}  // namespace fsm
