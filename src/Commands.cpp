#include "../includes/Server.hpp"

void Server::joinChannel(std::string input, std::vector<Client>::iterator it)
{
	bool operatorFlag = false;
	std::string channelName = getFirstWord(input.substr(5));
	if (channelName.empty()) {
		send(it->getClientFd(), "Invalid channel name!\n", 22, 0);
		return ;
	}
	std::vector<Channel>::iterator it_channel = searchChannel(channelName);
	if (it_channel == channels.end()) {
		channels.push_back(Channel(channelName, this));
		operatorFlag = true;
	}
	it_channel = searchChannel(channelName);
	
	//check if client is already in channel
	for (size_t i = 0; i < it_channel->getClients().size(); i++) {
		if (it_channel->getClients()[i].getClientFd() == it->getClientFd()) {
			send(it->getClientFd(), "You are already in this channel!\n", 33, 0);
			return ;
		}
	}
	it_channel->addClient(*it);
	if (operatorFlag) {
		it_channel->addOp(*it);
	}
	std::string message = ":" + it->getNickname() + "!" + it->getUsername() + "@host JOIN :#" + channelName + "\n";
	send(it->getClientFd(), message.c_str(), message.length(), 0);
	std::cout << "Client " << it->getClientFd() << " joined channel " << channelName << std::endl;
	std::string extraMessage = ":" + it->getNickname() + "!" + it->getUsername() + "@host JOIN :" + channelName + "\n";
	std::string allNames;
	for (size_t i = 0; i < it_channel->getClients().size(); i++) {
		if (it_channel->isOperator(it_channel->getClients()[i])) {
			allNames += "@";
		}
		allNames += it_channel->getClients()[i].getNickname() + " ";
		if (it_channel->getClients()[i].getClientFd() != it->getClientFd())
			send(it_channel->getClients()[i].getClientFd(), message.c_str(), message.length(), 0);
	}
	message = ":ircserver.local 353 " + it->getNickname() + " = #" + channelName + " :" + allNames + "\r\n";
	send(it->getClientFd(), message.c_str(), message.length(), 0);
	message = ":ircserver.local 366 " + it->getNickname() + " #" + channelName + " :End of /NAMES list\r\n";
	send(it->getClientFd(), message.c_str(), message.length(), 0);
}

void Server::changeNickname(std::string input, std::vector<Client>::iterator it)
{
	std::string nick = input.substr(5);
	std::string new_nick = getFirstWord(nick);
	if (new_nick.empty()) {
		send(it->getClientFd(), "Invalid nickname!\n", 18, 0);
		return ;
	}
	std::string message = ":" + it->getNickname() + "!" + it->getUsername() + "@host NICK :" + new_nick + "\r\n";
	for (size_t i = 0; i < channels.size(); ++i) {
		for (size_t j = 0; j < channels[i].getClients().size(); ++j) {
			std::cout << "Sending nickname change to user " << channels[i].getClients()[j].getUsername() << std::endl;
			send(channels[i].getClients()[j].getClientFd(), message.c_str(), message.length(), 0);
		}
	}
	it->setNickname(new_nick);
	std::cout << "Client " << it->getClientFd() << " changed nickname to " << new_nick << std::endl;
}

void Server::changeUsername(std::string input, std::vector<Client>::iterator it)
{
	if (it->isUsernameDefined()) {
		send(it->getClientFd(), "Username is already set!\n", 26, 0);
		return;
	}
	std::string user = input.substr(5);
	std::string newUser = getFirstWord(user);
	if (newUser.empty()) {
		send(it->getClientFd(), "Invalid username!\n", 18, 0);
		return ;
	}
	it->setUsernameDefined(true);
	it->setUsername(newUser);
	std::cout << "Client " << it->getClientFd() << " set username to " << newUser << std::endl;
}

void Server::leaveChannel(std::string input, std::vector<Client>::iterator it)
{
	std::string channel = getFirstWord(input.substr(6));
	std::cout << channel << std::endl;
	if (channel.empty()) {
		send(it->getClientFd(), "Invalid channel name!\n", 22, 0);
		return ;
	}
	std::vector<Channel>::iterator it_channel = searchChannel(channel);
	if (it_channel == channels.end()) {
		send(it->getClientFd(), "You are not in this channel!\n", 29, 0);
		return ;
	}
	it_channel->removeClient(*it);
	if (it_channel->isOperator(*it)) {
		it_channel->removeOp(*it);
	}
	std::string message = ":" + it->getNickname() + "!" + it->getUsername() + "@host PART #" + channel + "\n";
	send(it->getClientFd(), message.c_str(), message.length(), 0);
	message = ":" + it->getNickname() + "!" + it->getUsername() + "@host PART #" + channel + "\r\n";
	for (size_t i = 0; i < it_channel->getClients().size(); i++) {
		send(it_channel->getClients()[i].getClientFd(), message.c_str(), message.length(), 0);
	}
	std::cout << "Client " << it->getClientFd() << " left channel #" << channel << std::endl;
	if (it_channel->getClients().empty()) {
		channels.erase(it_channel);
		std::cout << "Channel #" << channel << " deleted as it has no clients left." << std::endl;
	}
}

void Server::handleWho(std::string input, std::vector<Client>::iterator it)
{
	std::string channelName = getFirstWord(input.substr(5));
	if (channelName.empty()) {
		send(it->getClientFd(), "Invalid channel name!\n", 22, 0);
		return;
	}
	std::vector<Channel>::iterator it_channel = searchChannel(channelName);
	if (it_channel == channels.end()) {
		send(it->getClientFd(), "No such channel.\n", 17, 0);
		return;
	}

	for (size_t i = 0; i < it_channel->getClients().size(); i++) {
		Client client = it_channel->getClients()[i];
		std::string response = ":ircserver.local 352 " + it->getNickname() + " #" + channelName + " " + client.getUsername() + " localhost ircserver.local "
	   		+ client.getNickname() + " H :0 " + client.getNickname() + "\r\n";
		send(it->getClientFd(), response.c_str(), response.length(), 0);
	}
	std::string message = ":ircserver.local 315 " + it->getNickname() + " #" + channelName + " :End of /WHO list.\r\n";
	send(it->getClientFd(), message.c_str(), message.length(), 0);
}

void Server::handleMode(std::string input, std::vector<Client>::iterator it)
{
	std::string channelName = getFirstWord(input.substr(6));
	if (channelName.empty()) {
		send(it->getClientFd(), "Invalid channel name!\n", 22, 0);
		return;
	}
	std::vector<Channel>::iterator it_channel = searchChannel(channelName);
	if (it_channel == channels.end()) {
		send(it->getClientFd(), "No such channel.\n", 17, 0);
		return;
	}

	std::string anyChanges = input.substr(6 + channelName.length());

	if (anyChanges.empty()) {
		std::string modes;
		for (size_t i = 0; i < it_channel->getFlags().size(); i++) {
			modes += it_channel->getFlags()[i];
		}

		std::string message = ":ircserver.local 324 " + it->getNickname() + " #" + channelName + " +" + modes + "\r\n";
		send(it->getClientFd(), message.c_str(), message.length(), 0);
		std::cout << message << std::endl;
	}
	else {
		if (it_channel->isOperator(*it) == false) {
			send(it->getClientFd(), "You are not an operator of this channel!\n", 40, 0);
			return;
		}
		for (size_t i = 0; i < anyChanges.length(); i++) {
			char flag = anyChanges[i];
			if (flag == 'i') {
				
			}
			else if (flag == 't') {

			}
			else if (flag == 'k') {

			}
			else if (flag == 'o') {
				std::string target = getFirstWord(anyChanges.substr(i + 1));
				if (it_channel->isOperator(*it)) {
					it_channel->removeOp(*it);
				}
				else {
					it_channel->addOp(*it);
				}
			}
			else if (flag == 'l') {

			}
		}
	}
}

void Server::handleQuit(std::string input, std::vector<Client>::iterator it)
{
	std::string channelName = getFirstWord(input.substr(5));

	std::cout << "Client " << it->getClientFd() << " quit safely " << std::endl;
	for (std::vector<Channel>::iterator it_channel = channels.begin(); it_channel != channels.end(); ++it_channel) {
		it_channel->removeClient(*it);
		if (it_channel->isOperator(*it)) {
			it_channel->removeOp(*it);
		}
	}
	
	std::vector<Channel>::iterator it_channel = searchChannel(channelName);
	std::string message = ":" + it->getNickname() + "!" + it->getUsername() + "@host PART #" + channelName + "\r\n";
	for (size_t i = 0; i < it_channel->getClients().size(); i++) {
		send(it_channel->getClients()[i].getClientFd(), message.c_str(), message.length(), 0);
	}
	removeClient(it->getClientFd());
}

void Server::handleTopic(std::string input, std::vector<Client>::iterator it)
{
	std::string channelName = getFirstWord(input.substr(7));
	std::vector<Channel>::iterator it_channel = searchChannel(channelName);

	if (input.length() <= 7 + channelName.length()) {
		std::string message = ":ircserver.local 332 " + it->getNickname() + " #" + channelName + " :" + it_channel->getTopic() + "\r\n";
		send(it->getClientFd(), message.c_str(), message.length(), 0);
		if (!it_channel->getTopicSetter().empty()) {
			message = ":ircserver.local 333 " + it->getNickname() + " #" + channelName + " :" + it_channel->getTopicSetter() + " " +
				timeToString(it_channel->getTimestamp()) + "\r\n";
			send(it->getClientFd(), message.c_str(), message.length(), 0);
		}
		return;
	}
	else {
		std::string newTopic = input.substr(7 + channelName.length() + 2);
		it_channel->setTopic(newTopic, it->getNickname());
		std::string message = ":" + it->getNickname() + "!" + it->getUsername() + "@host TOPIC #" + channelName + " :" + newTopic + "\r\n";
		for (size_t i = 0; i < it_channel->getClients().size(); ++i) {
			std::cout << "Sending topic change to client " << it_channel->getClients()[i].getClientFd() << ": " << message;
			send(it_channel->getClients()[i].getClientFd(), message.c_str(), message.length(), 0);
		}
	}

}

void Server::handleKick(std::string input, std::vector<Client>::iterator it)
{
	std::string channelName = getFirstWord(input.substr(6));
	std::vector<Channel>::iterator it_channel = searchChannel(channelName);

	if (it_channel == channels.end()) {
		send(it->getClientFd(), "No such channel.\n", 17, 0);
		return;
	}
	
	if (it_channel->isOperator(*it) == false) {
		send(it->getClientFd(), "You are not an operator of this channel!\n", 40, 0);
		return;
	}

	std::string targetNick = getFirstWord(input.substr(6 + channelName.length() + 1));
	if (targetNick.empty()) {
		send(it->getClientFd(), "Invalid target nickname!\n", 25, 0);
		return ;
	}
	std::string reason = (input.length() > 6 + channelName.length() + 1 + targetNick.length()) ? 
		input.substr(6 + channelName.length() + 1 + targetNick.length() + 1) : " No reason specified";
	reason = reason + "\r\n";

	std::string message;
	for (size_t i = 0; i < it_channel->getClients().size(); i++) {
		if (it_channel->getClients()[i].getNickname() == targetNick) {
			message = ":" + it->getNickname() + "!" + it->getUsername() + "@host KICK #" + channelName + " " + 
				targetNick + " :" + reason;
			send(it_channel->getClients()[i].getClientFd(), message.c_str(), message.length(), 0);
			if (it_channel->isOperator(it_channel->getClients()[i])) {
				it_channel->removeOp(it_channel->getClients()[i]);
			}
			it_channel->removeClient(it_channel->getClients()[i]);
		}
	}
	for (size_t i = 0; i < it_channel->getClients().size(); i++) {
		send(it_channel->getClients()[i].getClientFd(), message.c_str(), message.length(), 0);
	}
	
}