#include "../includes/Channel.hpp"

Channel::Channel(std::string name, Server* server) : name(name), server(server)
{
    
}

void Channel::addClient(Client client) {
    clients.push_back(client);
}

void Channel::removeClient(Client client) {
    clients.erase(std::remove(clients.begin(), clients.end(), client), clients.end());
}

std::string Channel::getName() const {
    return name;
}

std::vector<Client> Channel::getClients() const {
    return clients;
}
