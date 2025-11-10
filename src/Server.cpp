#include "../includes/Server.hpp"

Server::Server()
{
	serverFd = -1;
	port = 0;
	password = "";
	helpMessage = 
		"=== IRC Server Commands ===\r\n"
        "\r\n"
        "OPERATOR COMMANDS:\r\n"
        "* KICK - Eject a client from the channel\r\n"
        "* INVITE - Invite a client to a channel\r\n"
        "* TOPIC - Change or view the channel topic\r\n"
        "* MODE - Change the channel's mode:\r\n"
        "  · i: Set/remove Invite-only channel\r\n"
        "  · t: Set/remove the restrictions of the TOPIC command to channel operators\r\n"
        "  · k: Set/remove the channel key (password)\r\n"
        "  · o: Give/take channel operator privilege\r\n"
        "  · l: Set/remove the user limit to channel\r\n"
        "\r\n"
        "BASIC COMMANDS:\r\n"
        "* JOIN #channel - Join a channel\r\n"
        "* PART #channel - Leave a channel\r\n"
        "* PRIVMSG #channel :message - Send message to channel\r\n"
        "* PRIVMSG nickname :message - Send private message\r\n"
        "* NICK newnick - Change your nickname\r\n"
        "* QUIT - Disconnect from server\r\n"
        "\r\n"
        "Type *Guide* for this help again.\r\n"
        "===============================\r\n";
}

int Server::getPort()
{
	return port;
}

void Server::setPort(int num)
{
	if (num == 0)
		throw std::invalid_argument("Invalid Port");
	port = num;
}

std::string Server::getPassword()
{
	return password;
}

void Server::setPassword(std::string str)
{
	password = str;
}

std::vector<Client>::iterator Server::searchClient(int client_fd)
{
	std::vector<Client>::iterator it = clients.begin();
	for (; it != clients.end(); ++it) {
		if (it->getClientFd() == client_fd) {
			return it;
		}
	}
	throw std::runtime_error("Client not found");
	return it;
}

std::vector<Channel>::iterator Server::searchChannel(const std::string& channelName)
{
	std::vector<Channel>::iterator it = channels.begin();
	for (; it != channels.end(); ++it) {
		if (it->getName() == channelName) {
			return it;
		}
	}
	return it;
}

bool Server::checkCommands(std::string input, std::vector<Client>::iterator it)
{
	if (input == "*Guide*") {
		send(it->getClientFd(), helpMessage.c_str(), helpMessage.length(), 0);
		return true;
	}
	else if (input.compare(0, 5, "NICK ") == 0) {
		changeNickname(input, it);
		return true;
	}
	else if (input.compare(0, 5, "JOIN ") == 0) {
		joinChannel(input, it);
		return true;
	}
	else if (input.compare(0, 5, "USER ") == 0) {
		changeUsername(input, it);
		return true;
	}
	else if (input.compare(0, 5, "PART ") == 0) {
		leaveChannel(input, it);
		return true;
	}
	else if (input.compare(0, 4, "WHO ") == 0) {
		handleWho(input, it);
		return true;
	}
	else if (input.compare(0, 5, "MODE ") == 0) {
		handleMode(input, it);
		return true;
	}
	else if (input.compare(0, 5, "QUIT ") == 0
			|| (input.length() == 4 && input.compare(0, 4, "QUIT") == 0)) {
		handleQuit(input, it);
		return true;
	}
	else if (input.compare(0, 6, "TOPIC ") == 0) {
		handleTopic(input, it);
		return true;
	}
	else if (input.compare(0, 5, "KICK ") == 0) {
		handleKick(input, it);
		return true;
	}
	else if (input.compare(0, 7, "INVITE ") == 0) {
		handleInvite(input, it);
		return true;
	}
	return false;
}

void Server::sendMessageToAllClients(const std::string& message, int sender_fd)
{
	if (message.size() < 10 || message.substr(0, 9) != "PRIVMSG #") {
		return; // Not a channel message
	}
	std::string channelName = getFirstWord(message.substr(9));
	std::string msgContent = message.substr(9 + channelName.length() + 2);
	std::vector<Channel>::iterator it_channel = searchChannel(channelName);
	if (it_channel == channels.end()) {
		return;
	}
	std::vector<Client>::iterator it_client = searchClient(sender_fd);

	for (size_t i = 0; i < it_channel->getClients().size(); i++) {
		if (it_channel->getClients()[i].getClientFd() == sender_fd) {
			continue;
		}
		std::string fMessage = ":" + it_client->getNickname() + "!" + it_client->getUsername() + "@host"
		+ " PRIVMSG #" + channelName + " :" + msgContent + "\n";
		std::cout << fMessage << std::endl;
		send(it_channel->getClients()[i].getClientFd(), fMessage.c_str(), fMessage.length(), 0);
	}
}

std::vector<std::string> Server::split(const std::string& str)
{
	std::vector<std::string> result;
	std::istringstream iss(str);
	std::string token;
	while (std::getline(iss, token, '\n')) {
		if (!token.empty()) {
			result.push_back(token);
		}
	}
	return result;
}

void Server::createSocket()
{
	serverFd = socket(AF_INET, SOCK_STREAM, 0);
	if (serverFd < 0) {
		throw std::runtime_error("Failed to create socket: " + std::string(strerror(errno)));
	}

	int opt = 1;
	if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
		throw std::runtime_error("Failed to set socket options: " + std::string(strerror(errno)));
	}
}

void Server::bindSocket()
{
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(port);

	if (bind(serverFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1)
		throw std::runtime_error("Failed to bind socket: " + std::string(strerror(errno)));

	std::cout << "Socket bound to port " << port << std::endl;
}

void Server::listenSocket()
{
	if (listen(serverFd, 10) == -1)
		throw std::runtime_error("Failed to listen on socket: " + std::string(strerror(errno)));

	std::cout << "Server listening on port " << port << std::endl;

	struct pollfd server_poll_fd;
	server_poll_fd.fd = serverFd;
	server_poll_fd.events = POLLIN;
	server_poll_fd.revents = 0;
	pollFds.push_back(server_poll_fd);
}

void Server::acceptNewConnection()
{
	struct sockaddr_in clientAddr;
	socklen_t client_len = sizeof(clientAddr);

	int client_fd = accept(serverFd, (struct sockaddr*)&clientAddr, &client_len);
	if (client_fd == -1) {
		if (errno != EAGAIN && errno != EWOULDBLOCK)
			std::cerr << "Failed to accept connection: " << strerror(errno) << std::endl;
		return;
	}
	Client client(client_fd, clientAddr);
	clients.push_back(client);

	client.clientPollFd.fd = client_fd;
	client.clientPollFd.events = POLLIN;
	client.clientPollFd.revents = 0;
	pollFds.push_back(client.clientPollFd);

	std::cout << "New client connected (fd: " << client_fd << ") from " 
			  << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << std::endl;
	send(client_fd, "Input password!\n", 16, 0);
}

void Server::handleClientData(int client_fd)
{
	char buffer[1024];
	ssize_t bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
	
	if (bytes_read <= 0) {
		if (bytes_read == 0 || (errno != EAGAIN && errno != EWOULDBLOCK)) {
			removeClient(client_fd);
		}
		return;
	}
	buffer[bytes_read] = '\0';

	
	std::vector<Client>::iterator it = searchClient(client_fd);
	send(client_fd, "\n", 1, 0);
	std::vector<std::string> lines = split(buffer);

	for (size_t i = 0; i < lines.size(); i++) {
		std::string input(lines[i]);
		if (!input.empty() && input[input.length() - 1] == '\n') {
			input = input.substr(0, input.length() - 1);
		}
		if (!input.empty() && input[input.length() - 1] == '\r') {
			input = input.substr(0, input.length() - 1);
		}
		std::cout << "Received from client " << client_fd << ": " << input << std::endl;
		if (std::string("CAP LS 302") == input) {
			send(client_fd, "CAP * LS :ircv3 multi-prefix\n", 30, 0);
			continue;
		}
		if (std::string(";") == input) {
			continue;
		}

		if (!it->isClientAuthenticated()) {
			if (input != ("PASS " + password)) {
				send(client_fd, "Wrong password!\n", 16, 0);
				removeClient(client_fd);
				std::cout << "Client disconnected due to wrong password (fd: " << client_fd << ")" << std::endl;
			}
			else {
				it->changeAuthenticationStatus();
				send(client_fd, "Welcome\n Type *Guide* for more information.\n", 44, 0);
			}
			continue;
		}
		if (checkCommands(input, it) == false)
			sendMessageToAllClients(input, client_fd);
	}
	
}

void Server::removeClient(int client_fd)
{
	std::cout << "Client disconnected (fd: " << client_fd << ")" << std::endl;
	
	for (std::vector<struct pollfd>::iterator it = pollFds.begin(); it != pollFds.end(); ++it) {
		if (it->fd == client_fd) {
			pollFds.erase(it);
			clients.erase(searchClient(client_fd));
			break;
		}
	}
	
	close(client_fd);
}

void Server::runServerLoop()
{
	running = true;
	std::cout << "Server started. Waiting for connections..." << std::endl;

	while (running && !Server::SignalHandler::quit) {
		int poll_result = poll(&pollFds[0], pollFds.size(), -1);
		
		if (pollFds.empty()) {
			continue;
		}
		if (poll_result == -1) {
			if (errno == EINTR)
				continue;
			throw std::runtime_error("Poll failed: " + std::string(strerror(errno)));
		}

		for (size_t i = 0; i < pollFds.size(); ++i) {
			if (pollFds[i].revents & POLLIN) {
				if (pollFds[i].fd == serverFd) {
					acceptNewConnection();
				} else {
					handleClientData(pollFds[i].fd);
				}
			}
			
			if (pollFds[i].revents & (POLLHUP | POLLERR)) {
				if (pollFds[i].fd != serverFd) {
					removeClient(pollFds[i].fd);
					--i;
				}
			}
		}
	}
}

void Server::startServer()
{
	try {
		createSocket();
		bindSocket();
		listenSocket();
		runServerLoop();
	} catch (const std::exception& e) {
		std::cerr << "Error during server operation: " << e.what() << std::endl;
	}

}
