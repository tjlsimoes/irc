#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "Server.hpp"

class Server;

class Client;

class Channel 
{
	private:
		std::string name;
		std::vector<Client> clients;
		std::vector<Client> ops;
		Server* server;
		std::vector<char> flags;
		std::string topic;
		std::string topicSetter;
		time_t timestamp;

	public:
		Channel(std::string name, Server* serv);
		~Channel();
		void addClient(Client client);
		void removeClient(Client client);
		std::string getName() const;
		std::vector<Client> getClients() const;
		void addOp(Client client);
		void removeOp(Client client);
		bool isOperator(Client client) const;
		std::vector<char> getFlags() const;
		void addFlag(char flag);
		void removeFlag(char flag);
		void setTopic(const std::string& newTopic, const std::string& setter);
		std::string getTopic() const;
		time_t getTimestamp() const;
		std::string getTopicSetter() const;
};

#endif