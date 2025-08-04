#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "Server.hpp"

class Server;

class Channel 
{
	private:
		std::string name;
		std::vector<Client> clients;
		Server* server;

	public:
		Channel(std::string name, Server* server);
		void addClient(Client client);
		void removeClient(Client client);
		std::string getName() const;
		std::vector<Client> getClients() const;
};

#endif