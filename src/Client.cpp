#include "../includes/Client.hpp"

Client::Client(int fd, struct sockaddr_in addr)
	: clientFd(fd), clientAddr(addr)
{
	std::stringstream ss;
	ss << fd;
	nickname = "Guest " + ss.str();
	username = "User " + ss.str();
	channel = "nonexistent";
	isAuthenticated = false;
	op = false;
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
