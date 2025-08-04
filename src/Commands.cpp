#include "../includes/Server.hpp"

void Server::joinChannel(std::string input, std::vector<Client>::iterator it)
{
	std::string channel = input.substr(5);
	if (channel.empty()) {
		send(it->getClientFd(), "Invalid channel name!\n", 22, 0);
		return ;
	}
	std::vector<Channel>::iterator it_channel = searchChannel(channel);
	if (it_channel->getName() != channel) {
		channels.push_back(Channel(channel, this));
	}
	it_channel = searchChannel(channel);
	it_channel->addClient(*it);
	std::string message = ":" + it->getNickname() + "!" + it->getUsername() + "@host JOIN :#" + channel + "\n";
	send(it->getClientFd(), message.c_str(), message.length(), 0);
	std::cout << "Client " << it->getClientFd() << " joined channel " << channel << std::endl;
}

void Server::changeNickname(std::string input, std::vector<Client>::iterator it)
{
	std::string nick = input.substr(5);
	std::string new_nick = getFirstWord(nick);
	if (new_nick.empty()) {
		send(it->getClientFd(), "Invalid nickname!\n", 18, 0);
		return ;
	}
	it->setNickname(new_nick);
	std::cout << "Client " << it->getClientFd() << " changed nickname to " << new_nick << std::endl;
}

void Server::changeUsername(std::string input, std::vector<Client>::iterator it)
{
	std::string user = input.substr(5);
	std::string newUser = getFirstWord(user);
	if (newUser.empty()) {
		send(it->getClientFd(), "Invalid username!\n", 18, 0);
		return ;
	}
	it->setUsername(newUser);
	std::cout << "Client " << it->getClientFd() << " set username to " << newUser << std::endl;
}