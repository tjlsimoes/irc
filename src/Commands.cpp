#include "../includes/Server.hpp"

bool Server::validChannel(std::string const &name) {
	if (name.size() > 200)
		return false;
	const char *name_c = name.c_str();
	for (size_t i = 0; i < name.size(); i++) {
		if (name_c[i] == ' ' || name_c[i] == ','
			|| name_c[i] == 7)
			return false;
	}
	return true;
}

void Server::joinChannel(std::string input, std::vector<Client>::iterator it)
{
	bool operatorFlag = false;
	std::vector<std::string> const args = argsSplit(input);
	if (args.size() < 2) {
		std::string message = ":ircserver.local 461 " + it->getNickname() + " JOIN: not enough parameters" "\r\n";
		send(it->getClientFd(), message.c_str(), message.length(), 0);
		return ;
	}

	int keys_used = 0;
	std::vector<std::string> const joinChannels = commaSplit(args[1]);
	for (size_t k = 0; k < joinChannels.size(); ++k) {
		std::string const & channelName = joinChannels[k];

		std::vector<Channel>::iterator it_channel = searchChannel(channelName);
		if (it_channel == channels.end()) {
			if (!validChannel(channelName)) {
				send(it->getClientFd(), "Invalid channel name.\r\n", 23, 0);
				return ;
			}
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
			&& !it_channel->isInvited(it->getClientFd())
			&& !it_channel->isOperator(*it)) {
			std::string message = ":ircserver.local 473 " + it->getNickname()
							+ " #" + channelName + " :Cannot join channel (+i)\r\n";
			send(it->getClientFd(), message.c_str(), message.length(), 0);
			return;
		}
		if (it_channel->hasKey()) {
			if (args.size() < 3) {
				std::string message = ":ircserver.local 461 " + it->getNickname() + " JOIN: not enough parameters" "\r\n";
				send(it->getClientFd(), message.c_str(), message.length(), 0);
				return ;
			}
			std::vector<std::string> joinKeys = commaSplit(args[2]);
			if (!it_channel->checkKey(joinKeys[keys_used])) {
				std::string message = ":ircserver.local 475 " + it->getNickname()
								+ " #" + channelName + " :Cannot join channel (+k)\r\n";
				send(it->getClientFd(), message.c_str(), message.length(), 0);
				return;
			}
			keys_used++;
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
		it_channel->removeInvite(it->getClientFd());
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

}

bool Server::uniqueNickname(std::string const & newNickname) {
	std::vector<Client>::iterator it = clients.begin();
	for (; it != clients.end(); ++it) {
		if (it->getNickname() == newNickname) {
			return false;
		}
	}
	return true;
}

void Server::changeNickname(std::string input, std::vector<Client>::iterator it)
{
	std::string const nick = input.substr(5);
	std::string const new_nick = getFirstWord(nick);

	// If empty or same as current, ignore (no change needed)
	if (new_nick.empty() || new_nick == it->getNickname()) {
		return;
	}

	// Nickname already taken by another client
	if (!uniqueNickname(new_nick)) {
		std::string currentNick = it->getNickname();
		if (currentNick.empty())
			currentNick = "*";
		std::string message = ":ircserver.local 433 " + currentNick + " " + new_nick + " :Nickname is already in use\r\n";
		send(it->getClientFd(), message.c_str(), message.size(), 0);
		std::cout << "Client " << it->getClientFd() << " tried to change nickname to an already used nickname: " << new_nick << std::endl;
		return;
	}

	if (!it->getNickname().empty()) {
		std::string message = ":" + it->getNickname() + "!" + it->getUsername() + "@host NICK :" + new_nick + "\r\n";
		send(it->getClientFd(), message.c_str(), message.length(), 0);
	}

	for (size_t i = 0; i < channels.size(); ++i) {
		if (channels[i].hasClient(*it)) {
			std::string message = ":" + it->getNickname() + "!" + it->getUsername() + "@host NICK :" + new_nick + "\r\n";
			for (size_t j = 0; j < channels[i].getClients().size(); ++j) {
				if (channels[i].getClients()[j].getClientFd() == it->getClientFd())
					continue;
				std::cout << "Sending nickname change to user " << channels[i].getClients()[j].getUsername() << std::endl;
				send(channels[i].getClients()[j].getClientFd(), message.c_str(), message.length(), 0);
			}
			channels[i].updateNickname(new_nick, *it);
		}
	}
	
	bool firstNick = !it->getNickname().empty();
	it->setNickname(new_nick);
	if (firstNick && it->isUsernameDefined()) {
		std::string message = ":ircserver.local 001 " + it->getNickname() + " :Welcome to the Internet Relay Network " + it->getNickname() + "!" + it->getUsername() + "@host\r\n";
		send(it->getClientFd(), message.c_str(), message.length(), 0);
	}
	std::cout << "Client " << it->getClientFd() << " changed nickname to " << new_nick << std::endl;
}

bool Server::uniqueUsername(std::string const & newUsername) {
	std::vector<Client>::iterator it = clients.begin();
	for (; it != clients.end(); ++it) {
		if (it->getUsername() == newUsername) {
			return false;
		}
	}
	return true;
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

	for (size_t i = 0; i < channels.size(); ++i) {
		if (channels[i].hasClient(*it)) {
			channels[i].updateUsername(newUser, *it);
		}
	}
	it->setUsernameDefined(true);
	it->setUsername(newUser);
	std::cout << "Client " << it->getClientFd() << " set username to " << newUser << std::endl;
}

void Server::leaveChannel(std::string input, std::vector<Client>::iterator it)
{
	std::vector<std::string> const args = argsSplit(input);
	if (args.size() < 2) {
		std::string message = ":ircserver.local 461 " + it->getNickname() + " JOIN: not enough parameters" "\r\n";
		send(it->getClientFd(), message.c_str(), message.length(), 0);
		return ;
	}
	std::vector<std::string> const partChannels = commaSplit(args[1]);
	for (size_t k = 0; k < partChannels.size(); ++k) {
		std::string channel;
		partChannels[k][0] == '#' ? channel = partChannels[k].substr(1) : channel = partChannels[k];

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
}

void Server::handleWho(std::string input, std::vector<Client>::iterator it)
{
	std::vector<std::string> const args = argsSplit(input);
	if (args.size() < 2) {
		send(it->getClientFd(), "Invalid channel name!\n", 22, 0);
		return;
	}
	std::string const & channelName = args[1].substr(1, args[1].size());
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
	std::vector<std::string> const args = argsSplit(input);
	if (args.size() < 3) {
		std::string message = ":ircserver.local 461 " + it->getNickname() + " INVITE: not enough parameters" "\r\n";
		send(it->getClientFd(), message.c_str(), message.length(), 0);
		return ;
	}
	std::string const & nick = args[1];
	std::string const & chan = args[2].substr(1, args[2].size());
	std::cout << "Invite command: nick=" << nick << ", chan=" << chan << std::endl;

	std::vector<Channel>::iterator const it_channel = searchChannel(chan);
	std::cout << it_channel->getName() << std::endl;
	// Invalid channel
	if (it_channel == channels.end()) {
		// find target client
		std::vector<Client>::iterator const target = searchClientByNick(nick);
		if (target == clients.end()) {
			std::string message = ":ircserver.local 401 " + it->getNickname()
							+ " #" + chan + " :No such nickname\r\n";
			send(it->getClientFd(), message.c_str(), message.length(), 0);
			return;
		}
		// 341 RPL_INVITING
		std::string message = ":ircserver.local 341 " + it->getNickname()
						  + " " + nick + " #" + chan + "\r\n";
		send(it->getClientFd(), message.c_str(), message.length(), 0);

		// Notify the invited user
		message = ":" + it->getNickname()
			+ "!" + it->getUsername() + "@host INVITE " + nick
			+ " :#" + chan + "\r\n";
		send(target->getClientFd(), message.c_str(), message.length(), 0);
		return;
	}

	if (it_channel->isInviteOnly() && !it_channel->isOperator(*it)) {
		std::string message = ":ircserver.local 482 " + it->getNickname()
						+ " #" + chan + " :You're not a channel operator\r\n";
		send(it->getClientFd(), message.c_str(), message.length(), 0);
		return;
	} else if (it_channel->isInviteOnly() && !it_channel->hasClient(*it))
	{
		std::string message = ":ircserver.local 442 " + it->getNickname()
						+ " #" + chan + " :You're not part of the channel\r\n";
		send(it->getClientFd(), message.c_str(), message.length(), 0);
		return;
	}
	for (size_t k = 0; k < it_channel->getClients().size(); ++k) {
		if (it_channel->getClients()[k].getNickname() == nick) {
			std::string message = ":ircserver.local 443 " + it->getNickname()
						+ " #" + chan + " :User already on specified channel\r\n";
			send(it->getClientFd(), message.c_str(), message.length(), 0);
			return;
		}
	}

	// find target client
	std::vector<Client>::iterator target = searchClientByNick(nick);
	if (target == clients.end()) {
		std::string message = ":ircserver.local 401 " + it->getNickname()
						+ " #" + chan + " :No such nickname\r\n";
		send(it->getClientFd(), message.c_str(), message.length(), 0);
		return;
	}

	it_channel->addInvite(target->getClientFd());

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
	std::vector<std::string> const args = argsSplit(input);
	if (args.size() < 2) {
		std::string message = ":ircserver.local 461 " + it->getNickname() + " MODE: not enough parameters" "\r\n";
		send(it->getClientFd(), message.c_str(), message.length(), 0);
		return ;
	}
	std::string const & channelName = args[1].substr(1, args[1].size());
	std::vector<Channel>::iterator it_channel = searchChannel(channelName);
	if (it_channel == channels.end()) {
		std::string message = ":ircserver.local 403 " + it->getNickname() + " MODE: no such channel" "\r\n";
		send(it->getClientFd(), message.c_str(), message.length(), 0);
		return;
	}

	std::string anyChanges;
	args.size() > 2 ? anyChanges = args[2] : anyChanges = "";

	if (anyChanges.empty()) {
		std::string modes;
		std::string params;
		for (size_t i = 0; i < it_channel->getFlags().size(); i++) {
			modes += it_channel->getFlags()[i];
			if (it_channel->hasLimit() && it_channel->getFlags()[i] == 'l') {
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
			std::string message = ":ircserver.local 482 " + it->getNickname()
						+ " #" + channelName + " :You're not a channel operator\r\n";
			send(it->getClientFd(), message.c_str(), message.length(), 0);
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
					std::string key;
					args.size() > 3 ? key = args[3] : key = "";
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
				args.size() > 3 ? targetNick = args[3] : targetNick = "";
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
						send(it->getClientFd(), "MODE +o requires a nickname\n", 28, 0);
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
					std::string limitStr;
					args.size() > 3 ? limitStr = args[3] : limitStr = "";
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
	std::vector<std::string> const args = argsSplit(input);
	std::string reason;
	if (args.size() >= 2) {
		for (unsigned int i = 1; i < args.size(); i++) {
			if (i == 1 && args[i][0] == ':') {
				reason += args[i].substr(1) + " ";
				continue ;
			}
			reason += args[i] + " ";
		}
	}
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
			const std::vector<Client>& channelClients = it_channel->getClients();
			for (size_t i = 0; i < channelClients.size(); ++i) {
				if (channelClients[i].getClientFd() != it->getClientFd()) {
					send(channelClients[i].getClientFd(), message.c_str(), message.length(), 0);
				}
			}
		}
	}

	// Log and remove client
	std::cout << "Client " << it->getClientFd() << " quit: " << reason << std::endl;
}

void Server::handleTopic(std::string input, std::vector<Client>::iterator it)
{
	std::vector<std::string> const args = argsSplit(input);
	if (args.size() == 1) {
		std::string message = ":ircserver.local 461 " + it->getNickname() + " TOPIC: not enough parameters" "\r\n";
		send(it->getClientFd(), message.c_str(), message.length(), 0);
		return ;
	}
	std::string const & channelName = args[1].substr(1, args[1].size());
	std::vector<Channel>::iterator const it_channel = searchChannel(channelName);

	if (it_channel == channels.end()) {
		send(it->getClientFd(), "No such channel.\n", 17, 0);
		return;
	}

	if (!it_channel->hasClient(*it)) {
		std::string message = ":ircserver.local 442 " + it->getNickname() + " :You're not on that channel" "\r\n";
		send(it->getClientFd(), message.c_str(), message.length(), 0);
		return ;
	}

	if (args.size() == 2) {
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

	if (args.size() != 3) {
		std::string message = ":ircserver.local 461 " + it->getNickname() + " TOPIC: not enough parameters" "\r\n";
		send(it->getClientFd(), message.c_str(), message.length(), 0);
		return ;
	}

	std::string newTopic = args[2];
	if (newTopic[0] == ':')
		newTopic = newTopic.substr(1);
	it_channel->setTopic(newTopic, it->getNickname());
	std::string message = ":" + it->getNickname() + "!" + it->getUsername() + "@host TOPIC #" + channelName + " :" + newTopic + "\r\n";
	for (size_t i = 0; i < it_channel->getClients().size(); ++i) {
		std::cout << "Sending topic change to client " << it_channel->getClients()[i].getClientFd() << ": " << message;
		send(it_channel->getClients()[i].getClientFd(), message.c_str(), message.length(), 0);
	}

}

void Server::handleKick(std::string input, std::vector<Client>::iterator it)
{
	std::vector<std::string> const args = argsSplit(input);
	if (args.size() < 3) {
		std::string message = ":ircserver.local 461 " + it->getNickname() + " KICK: not enough parameters" "\r\n";
		send(it->getClientFd(), message.c_str(), message.length(), 0);
		return ;
	}
	std::string const & channelName = args[1].substr(1, args[1].size());;
	std::vector<Channel>::iterator const it_channel = searchChannel(channelName);

	if (it_channel == channels.end()) {
		std::string message = ":ircserver.local 403 " + it->getNickname() + " KICK: no such channel" "\r\n";
		send(it->getClientFd(), message.c_str(), message.length(), 0);
		return;
	}
	
	if (it_channel->isOperator(*it) == false) {
		std::string message = ":ircserver.local 482 " + it->getNickname() + " KICK: not an operator of this channel" "\r\n";
		send(it->getClientFd(), message.c_str(), message.length(), 0);
		return;
	}

	std::string const  & targetNick = args[2];

	std::string reason = "";
	if (args.size() >= 4) {
		for (unsigned int i = 3; i < args.size(); i++) {
			if (i == 3 && args[i][0] == ':') {
				reason += args[i].substr(1) + " ";
				continue ;
			}
			reason += args[i] + " ";
		}
	}
	reason.empty() ? reason =  "No reason specified" : reason;
	reason = reason + "\r\n";

	std::string message;
	bool found = false;
	for (size_t i = 0; i < it_channel->getClients().size(); i++) {
		if (it_channel->getClients()[i].getNickname() == targetNick) {
			message = ":" + it->getNickname() + "!" + it->getUsername() + "@host KICK #" + channelName + " " + 
				targetNick + " :" + reason;
			send(it_channel->getClients()[i].getClientFd(), message.c_str(), message.length(), 0);
			if (it_channel->isOperator(it_channel->getClients()[i])) {
				it_channel->removeOp(it_channel->getClients()[i]);
			}
			it_channel->removeClient(it_channel->getClients()[i]);
			found = true;
		}
	}
	if (!found) {
		message = ":ircserver.local 442 " + it->getNickname() + " KICK: user not found" "\r\n";
		send(it->getClientFd(), message.c_str(), message.length(), 0);
		return ;
	}
	for (size_t i = 0; i < it_channel->getClients().size(); i++) {
		send(it_channel->getClients()[i].getClientFd(), message.c_str(), message.length(), 0);
	}
}
