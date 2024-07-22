#include <cstdlib>
#include <ctime>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#include "CommunicatingFSM.hpp"

enum class Moves {
    Req,
    Done,
    Ack,
    Alarm,
};

std::mutex console;
std::vector<Moves> keys{Moves::Req, Moves::Done, Moves::Ack, Moves::Alarm};
std::unique_ptr<fsm::CFSM<Moves>> user(new fsm::CFSM<Moves>(keys));
std::unique_ptr<fsm::CFSM<Moves>> server(new fsm::CFSM<Moves>(keys));

void run_user() {
    while (true) {
        console.lock();
        std::cout << "User: " << user->GetState() << "\n";
        console.unlock();
        std::list<Moves> moves = user->LookUpMoves();
        if (moves.empty()) {
            user->WaitForMsgAndMove();
        } else {
            user->Move(moves.front());
        }
        sleep(1);
    }
}

void run_server() {
    while (true) {
        console.lock();
        std::cout << "Server: " << server->GetState() << "\n";
        console.unlock();
        auto moves = server->LookUpMoves();
        if (moves.empty()) {
            server->WaitForMsgAndMove();
        } else {
            if (std::rand() % 3 != 0) {
                server->Move(moves.front());
            } else {
                server->WaitForMsgAndMove();
            }
        }
        sleep(1);
    }
}

int main() {
    std::srand(std::time(nullptr));
    // std::unique_ptr<fsm::CFSM<Moves>> user(new fsm::CFSM<Moves>(keys));
    // std::unique_ptr<fsm::CFSM<Moves>> server(new fsm::CFSM<Moves>(keys));
    fsm::Channel<Moves>* user_to_server = new fsm::AsyncChannel<Moves>();
    fsm::Channel<Moves>* server_to_user = new fsm::AsyncChannel<Moves>();
    user->AddNode("Ready");
    user->AddNode("Wait");
    user->AddNode("Register");
    user->SetInputChannel(server_to_user);
    user->SetOutputChannel(user_to_server);
    user->SetInitialNode("Ready");
    user->SetTransition("Ready", "Wait", Moves::Req,
                        fsm::CFSM<Moves>::Action::Transmit);
    user->SetTransition("Wait", "Wait", Moves::Alarm,
                        fsm::CFSM<Moves>::Action::Receive);
    user->SetTransition("Wait", "Ready", Moves::Done,
                        fsm::CFSM<Moves>::Action::Receive);
    user->SetTransition("Ready", "Register", Moves::Alarm,
                        fsm::CFSM<Moves>::Action::Receive);
    user->SetTransition("Register", "Ready", Moves::Ack,
                        fsm::CFSM<Moves>::Action::Transmit);

    server->AddNode("Idle");
    server->AddNode("Service");
    server->AddNode("Fault");
    server->SetInputChannel(user_to_server);
    server->SetOutputChannel(server_to_user);
    server->SetInitialNode("Idle");
    server->SetTransition("Idle", "Service", Moves::Req,
                          fsm::CFSM<Moves>::Action::Receive);
    server->SetTransition("Service", "Idle", Moves::Done,
                          fsm::CFSM<Moves>::Action::Transmit);
    server->SetTransition("Idle", "Fault", Moves::Alarm,
                          fsm::CFSM<Moves>::Action::Transmit);
    server->SetTransition("Fault", "Fault", Moves::Req,
                          fsm::CFSM<Moves>::Action::Receive);
    server->SetTransition("Fault", "Idle", Moves::Ack,
                          fsm::CFSM<Moves>::Action::Transmit);
    std::cout << "Finished constructing\n";
    std::thread t1(run_user);
    std::thread t2(run_server);
    t1.join();
    t2.join();
    return 0;
}
