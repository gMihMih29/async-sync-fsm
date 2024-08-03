#include <cassert>
#include <cstdlib>
#include <iostream>
#include <map>
#include <thread>
#include <utility>
#include <vector>

#include "bfs.hpp"

enum class Moves {
    Req,
    Done,
    Ack,
    Alarm,
};


std::mutex console;
std::vector<Moves> keys{Moves::Req, Moves::Done, Moves::Ack, Moves::Alarm};
fsm::CFSM<Moves> user(keys);
fsm::CFSM<Moves> server(keys);
AsyncResearchChannel<Moves>* user_to_server;
AsyncResearchChannel<Moves>* server_to_user;

void init() {
    std::srand(std::time(nullptr));
    user.AddNode("Ready");
    user.AddNode("Wait");
    user.AddNode("Register");
    user.SetInitialNode("Ready");
    user.SetTransition("Ready", "Wait", Moves::Req,
                       fsm::CFSM<Moves>::Action::Transmit);
    user.SetTransition("Wait", "Wait", Moves::Alarm,
                       fsm::CFSM<Moves>::Action::Receive);
    user.SetTransition("Wait", "Ready", Moves::Done,
                       fsm::CFSM<Moves>::Action::Receive);
    user.SetTransition("Ready", "Register", Moves::Alarm,
                       fsm::CFSM<Moves>::Action::Receive);
    user.SetTransition("Register", "Ready", Moves::Ack,
                       fsm::CFSM<Moves>::Action::Transmit);

    server.AddNode("Idle");
    server.AddNode("Service");
    server.AddNode("Fault");
    server.SetInitialNode("Idle");
    server.SetTransition("Idle", "Service", Moves::Req,
                         fsm::CFSM<Moves>::Action::Receive);
    server.SetTransition("Service", "Idle", Moves::Done,
                         fsm::CFSM<Moves>::Action::Transmit);
    server.SetTransition("Idle", "Fault", Moves::Alarm,
                         fsm::CFSM<Moves>::Action::Transmit);
    server.SetTransition("Fault", "Fault", Moves::Req,
                         fsm::CFSM<Moves>::Action::Receive);
    server.SetTransition("Fault", "Idle", Moves::Ack,
                         fsm::CFSM<Moves>::Action::Receive);
    std::cout << "Finished constructing\n";
}

int main() {
    init();
    bfs(user, server);
    return 0;
}
