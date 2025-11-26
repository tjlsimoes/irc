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
		bool inviteOnly;
		std::vector<int> inviteList;
		bool topicLocked;
		std::string key;
		int limit;

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

		void changeFlag(char flag, bool add);
		bool hasFlag(char flag) const;

		bool isInviteOnly() const;
		void setInviteOnly(bool add);
		void addInvite(int clientId);
		void removeInvite(int clientId);
		bool isInvited(int clientId) const;

		bool isTopicLocked() const;
		void setTopicLock(bool add);

		bool hasKey() const;
		void setKey(std::string const & newKey, bool add);
		std::string getKey() const;
		bool checkKey(std::string const &attempted) const;

		bool hasLimit() const;
		void setLimit(int newLimit, bool add);
		int	getLimit() const;

		bool hasClient(const Client& client) const;

		void updateNickname(std::string const & newUsername, const Client & client);
		void updateUsername(std::string const & newUsername, const Client & client);
};

#endif