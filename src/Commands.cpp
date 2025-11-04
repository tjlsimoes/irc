#include "../includes/Server.hpp"

void Server::joinChannel(std::string input, std::vector<Client>::iterator it)
{
	bool operatorFlag = false;
	std::string rest = input.substr(5);                 // after "JOIN "
	std::string channelName = getFirstWord(rest);
	std::string providedKey;
	!rest.substr(channelName.length()).empty() ? providedKey = getFirstWord(rest.substr(channelName.length() + 1)) : providedKey = "";
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

	if (it_channel->isInviteOnly()
		&& !it_channel->isInvited(it->getNickname())
		&& !it_channel->isOperator(*it)) {
		std::string message = ":ircserver.local 473 " + it->getNickname()
						+ " #" + channelName + " :Cannot join channel (+i)\r\n";
		send(it->getClientFd(), message.c_str(), message.length(), 0);
		return;
	}
	if (it_channel->hasKey() && !it_channel->checkKey(providedKey)) {
		std::string message = ":ircserver.local 475 " + it->getNickname()
						+ " #" + channelName + " :Cannot join channel (+k)\r\n";
		send(it->getClientFd(), message.c_str(), message.length(), 0);
		return;
	}
	if (it_channel->hasLimit()
		&& it_channel->getClients().size() >= static_cast<size_t>(it_channel->getLimit()))
	{
		std::string message = ":ircserver.local 471 " + it->getNickname()
						+ " #" + channelName + " :Cannot join channel (+l)\r\n";
		send(it->getClientFd(), message.c_str(), message.length(), 0);
		return;
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

std::vector<Client>::iterator Server::searchClientByNick(std::string const & nickname) {
	std::vector<Client>::iterator it = clients.begin();
	for (; it != clients.end(); ++it) {
		if (it->getNickname() == nickname) {
			return it;
		}
	}
	// throw std::runtime_error("Client not found");
	return it;
}


void Server::handleInvite(std::string input, std::vector<Client>::iterator it)
{
	// INVITE nick #channel
	std::string rest = input.substr(7);
	std::string nick = getFirstWord(rest);
	std::string chan = getFirstWord(rest.substr(nick.length() + 1));
	if (nick.empty() || chan.empty()) {
		send(it->getClientFd(), "Usage: INVITE <nick> <channel>\n", 31, 0);
		return;
	}

	std::vector<Channel>::iterator it_channel = searchChannel(chan);
	if (it_channel == channels.end()
		|| !it_channel->isOperator(*it)) {
		send(it->getClientFd(),
			 "You need to be a channel operator to invite.\n", 45, 0);
		return;
		}

	// find target client
	std::vector<Client>::iterator target = searchClientByNick(nick);
	if (target == clients.end()) {
		send(it->getClientFd(),
			 "No such nick.\n", 14, 0);
		return;
	}

	it_channel->addInvite(nick);

	// 341 RPL_INVITING
	std::string message = ":ircserver.local 341 " + it->getNickname()
					  + " " + nick + " #" + chan + "\r\n";
	send(it->getClientFd(), message.c_str(), message.length(), 0);

	// Notify the invited user
	message = ":" + it->getNickname()
		+ "!" + it->getUsername() + "@host INVITE " + nick
		+ " :#" + chan + "\r\n";
	send(target->getClientFd(), message.c_str(), message.length(), 0);
}

void Server::broadcastMessage(std::string const & message, std::vector<Channel>::iterator const it_channel) {
	for (size_t k = 0; k < it_channel->getClients().size(); ++k) {
		std::cout << "Sending mode change to client " << it_channel->getClients()[k].getClientFd() << ": " << message;
		send(it_channel->getClients()[k].getClientFd(), message.c_str(), message.length(), 0);
	}
}

void Server::handleMode(std::string input, std::vector<Client>::iterator it)
{
	std::string rest = input.substr(6);
	std::string channelName = getFirstWord(rest);
	if (channelName.empty()) {
		send(it->getClientFd(), "Invalid channel name!\n", 22, 0);
		return;
	}
	std::vector<Channel>::iterator it_channel = searchChannel(channelName);
	if (it_channel == channels.end()) {
		send(it->getClientFd(), "No such channel.\n", 17, 0);
		return;
	}

	std::string anyChanges;
	!rest.substr(channelName.length()).empty() ? anyChanges = getFirstWord(rest.substr(channelName.length() + 1)) : anyChanges = "";

	if (anyChanges.empty()) {
		std::string modes;
		std::string params;
		for (size_t i = 0; i < it_channel->getFlags().size(); i++) {
			if (it_channel->getFlags()[i] != 'k')
				modes += it_channel->getFlags()[i];
			else if (it_channel->getFlags()[i] != 'l' && it_channel->hasLimit()) {
				std::ostringstream oss;
				oss << it_channel->getLimit();
				params += " " + oss.str();
			}
		}

		std::string message = ":ircserver.local 324 " + it->getNickname() + " #" + channelName + " +" + modes + params + "\r\n";
		send(it->getClientFd(), message.c_str(), message.length(), 0);
		std::cout << message << std::endl;
	}
	else {
		if (it_channel->isOperator(*it) == false) {
			send(it->getClientFd(), "You are not an operator of this channel!\n", 41, 0);
			return;
		}
		bool add = true;
		for (size_t i = 0; i < anyChanges.length(); i++) {
			char const flag = anyChanges[i];

			if (flag == '-') {
				add = false;
				continue ;
			}
			else if (flag == '+')
				continue ;


			if (flag == 'i') {
				it_channel->setInviteOnly(add);
				std::string message = ":" + it->getNickname()
											+ "!" + it->getUsername() + "@host MODE #" + channelName
											+ " " + (add ? "+" : "-") + "i\r\n";
				broadcastMessage(message, it_channel);
				std::cout << message << std::endl;
			}
			else if (flag == 't') {
				it_channel->setTopicLock(add);
				std::string message = ":" + it->getNickname()
									+ "!" + it->getUsername() + "@host MODE #" + channelName
									+ " " + (add ? "+" : "-") + "t\r\n";
				broadcastMessage(message, it_channel);
			}
			else if (flag == 'k') {
				if (add) {
					std::string key = getFirstWord(input.substr(6 + channelName.length() + anyChanges.length() + 2));
					if (key.empty()) {
						send(it->getClientFd(), "MODE +k requires a key\n", 23, 0);
						continue;
					}
					it_channel->setKey(key, add);
				} else
					it_channel->setKey("", add);

				std::string message = ":" + it->getNickname()
									+ "!" + it->getUsername() + "@host MODE #" + channelName
									+ " " + (add ? "+" : "-") + "k"
									+ (add ? " " + it_channel->getKey() : "") + "\r\n";
				broadcastMessage(message, it_channel);

			}
			else if (flag == 'o') {
				std::string targetNick;
				!rest.substr(channelName.length() + anyChanges.length() + 2).empty() ? targetNick = getFirstWord(input.substr(6 + channelName.length() + anyChanges.length() + 2)) : targetNick = "";
				if (!add) {
					if (targetNick.empty()) {
						send(it->getClientFd(), "MODE -o requires a nickname\n", 28, 0);
						continue;
					}
					std::vector<Client>::iterator targetIt = searchClientByNick(targetNick);
					if (targetIt == clients.end()
						|| !it_channel->hasClient(*targetIt)) {
						send(it->getClientFd(), "No such nickname in channel\n", 28, 0);
						continue;
					}

					it_channel->removeOp(*targetIt);

					std::string message = ":" + it->getNickname()
						+ "!" + it->getUsername() + "@host MODE #" + channelName
						+ " -o " + targetNick + "\r\n";
					broadcastMessage(message, it_channel);
				}
				else {
					if (targetNick.empty()) {
						send(it->getClientFd(), "MODE -o requires a nickname\n", 28, 0);
						continue;
					}
					std::vector<Client>::iterator targetIt = searchClientByNick(targetNick);
					if (targetIt == clients.end()
						|| !it_channel->hasClient(*targetIt)) {
						send(it->getClientFd(), "No such nickname in channel\n", 28, 0);
						continue;
						}
					it_channel->addOp(*targetIt);

					std::string message = ":" + it->getNickname()
						+ "!" + it->getUsername() + "@host MODE #" + channelName
						+ " +o " + targetNick + "\r\n";
					broadcastMessage(message, it_channel);
				}
			}
			else if (flag == 'l') {
				if (add) {
					std::string limitStr = getFirstWord(input.substr(6 + channelName.length() + anyChanges.length() + 2));
					if (limitStr.empty()) {
						send(it->getClientFd(), "MODE +l requires a limit\n", 25, 0);
						continue;
					}
					int limit = std::atoi(limitStr.c_str());
					if (limit <= 0) {
						send(it->getClientFd(), "Limit must be > 0\n", 18, 0);
						continue;
					}
					it_channel->setLimit(limit, add);
				} else
					it_channel->setLimit(0, add);

				std::string limitParam;
				if (add && it_channel->hasLimit()) {
					std::ostringstream oss;
					oss << it_channel->getLimit();
					limitParam = " " + oss.str();
				}

				std::string message = ":" + it->getNickname()
									+ "!" + it->getUsername() + "@host MODE #" + channelName
									+ " " + (add ? "+" : "-") + "l"
									+ limitParam + "\r\n";
				broadcastMessage(message, it_channel);
			}
		}
	}
}

void Server::handleQuit(std::string input, std::vector<Client>::iterator it)
{
	std::string reason;
	input.substr(4).empty() ? reason = "" : reason = getFirstWord(input.substr(5));

	std::string message = ":" + it->getNickname() + "!" + it->getUsername() +
                          "@host QUIT " + reason + "\r\n";

    // Send QUIT to all channels the client is in
    for (std::vector<Channel>::iterator it_channel = channels.begin(); it_channel != channels.end(); ++it_channel) {
        if (it_channel->hasClient(*it)) {
            it_channel->removeClient(*it);
            if (it_channel->isOperator(*it)) {
                it_channel->removeOp(*it);
            }

            // Send QUIT to all remaining clients in this channel
            const std::vector<Client>& clients = it_channel->getClients();
            for (size_t i = 0; i < clients.size(); ++i) {
                if (clients[i].getClientFd() != it->getClientFd()) {
                    send(clients[i].getClientFd(), message.c_str(), message.length(), 0);
                }
            }
        }
    }

    // Log and remove client
    std::cout << "Client " << it->getClientFd() << " quit: " << reason << std::endl;
    removeClient(it->getClientFd());
}

void Server::handleTopic(std::string input, std::vector<Client>::iterator it)
{
	std::string channelName = getFirstWord(input.substr(7));
	std::vector<Channel>::iterator it_channel = searchChannel(channelName);

	if (it_channel == channels.end()) {
		send(it->getClientFd(), "No such channel.\n", 17, 0);
		return;
	}

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

	if (it_channel->isTopicLocked() && !it_channel->isOperator(*it)) {
		std::string message = ":ircserver.local 482 " + it->getNickname()
						+ " #" + channelName + " :You're not a channel operator\r\n";
		send(it->getClientFd(), message.c_str(), message.length(), 0);
		return;
	}

	std::string newTopic = input.substr(7 + channelName.length() + 2);
	it_channel->setTopic(newTopic, it->getNickname());
	std::string message = ":" + it->getNickname() + "!" + it->getUsername() + "@host TOPIC #" + channelName + " :" + newTopic + "\r\n";
	for (size_t i = 0; i < it_channel->getClients().size(); ++i) {
		std::cout << "Sending topic change to client " << it_channel->getClients()[i].getClientFd() << ": " << message;
		send(it_channel->getClients()[i].getClientFd(), message.c_str(), message.length(), 0);
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
		send(it->getClientFd(), "You are not an operator of this channel!\n", 41, 0);
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