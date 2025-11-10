#include "../includes/Server.hpp"

bool Server::SignalHandler::quit = false;

bool Server::SignalHandler::setup() {
    struct sigaction sa;
    std::memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = handler;
    sa.sa_flags = SA_SIGINFO | SA_RESTART;
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGINT);
    sigaddset(&sa.sa_mask, SIGTERM);
    sigaddset(&sa.sa_mask, SIGQUIT);

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        std::cerr << "Error: cannot set SIGINT handler\n";
        return false;
    }

    if (sigaction(SIGQUIT, &sa, NULL) == -1) {
        std::cerr << "Error: cannot set SIGQUIT handler\n";
        return false;
    }
    return true;
}

void Server::SignalHandler::handler(int, siginfo_t *, void *) {
    quit = true;
}
