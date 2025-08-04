#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Server.hpp"

class Server;

class Client
{
	private:
		int clientFd;
		struct sockaddr_in clientAddr;
		std::string nickname;
		std::string username;
		bool isAuthenticated;
		bool op;

	public:
		struct pollfd clientPollFd;
		int getClientFd() const;
		void setUsername(const std::string& user);
		std::string getUsername() const;
		std::string getNickname() const;
		void setNickname(const std::string& nick);
		bool isClientAuthenticated() const;
		void changeAuthenticationStatus();
		Client(int fd, struct sockaddr_in addr);
		
};

#endif