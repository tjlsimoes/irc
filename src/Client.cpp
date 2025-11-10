#include "../includes/Client.hpp"

Client::Client(int fd, struct sockaddr_in addr)
	: clientFd(fd), clientAddr(addr)
{
	std::stringstream ss;
	ss << fd;
	nickname = "Guest" + ss.str();
	username = "User" + ss.str();
	isAuthenticated = false;
	serverOperator = false;
	usernameDefined = false;
}

int Client::getClientFd() const
{
	return clientFd;
}

bool Client::isClientAuthenticated() const
{
	return isAuthenticated;
}

void Client::changeAuthenticationStatus()
{
	isAuthenticated = !isAuthenticated;
}

void Client::setUsername(const std::string& user)
{
	if (!user.empty()) {
		username = user;
	}
}

std::string Client::getUsername() const
{
	return username;
}

std::string Client::getNickname() const
{
	return nickname;
}

void Client::setNickname(const std::string& nick)
{
	if (!nick.empty()) {
		nickname = nick;
	}
}

bool Client::isServerOperator() const
{
	return serverOperator;
}

void Client::changeOperatorStatus()
{
	serverOperator = !serverOperator;
}

void Client::setUsernameDefined(bool status)
{
	usernameDefined = status;
}

bool Client::isUsernameDefined() const
{
	return usernameDefined;
}

struct sockaddr_in Client::getClientAddr() const
{
	return clientAddr;
}