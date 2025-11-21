#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <iomanip>
#include <string>
#include <cerrno>
#include <sstream>
#include <cmath>
#include <limits>
#include <algorithm>
#include <vector>
#include <climits>
#include <stack>
#include <fstream>
#include <utility>
#include <ctime>
#include <list>
#include <poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cctype>
#include <exception>
#include <fcntl.h>
#include <stdexcept>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include "Client.hpp"
#include "Channel.hpp"

class Channel;

class Client;

//template<typename T>
class Server
{
	private:
	int port;
	std::string password;
	int serverFd;
	struct sockaddr_in serverAddr;
	std::vector<struct pollfd> pollFds;
	bool running;
	std::vector<Client> clients;
	std::vector<Channel> channels;
	std::string helpMessage;

	public:
	Server();

	class SignalHandler {
	public:
		static bool quit;
		static bool setup();
		static void handler(int, siginfo_t*, void*);
	};

	int getPort();
	void setPort(int num);
	std::string getPassword();
	void setPassword(std::string str);
	void startServer();
	void createSocket();
	void bindSocket();
	void listenSocket();
	void runServerLoop();
	void acceptNewConnection();
	void handleClientData(int client_fd);
	void removeClient(int client_fd);
	bool checkCommands(std::string input, std::vector<Client>::iterator it);
	void sendMessageToAllClients(const std::string& message, int sender_fd);
	void suddenQuit(std::vector<Client>::iterator it);
	std::vector<std::string> split(const std::string& str);

	std::vector<Channel>::iterator searchChannel(const std::string& channelName);
	std::vector<Client>::iterator searchClient(int client_fd);
	std::vector<Client>::iterator searchClientByNick(std::string const & nickname);


	// commands
	static bool validChannel(std::string const &name);
	bool uniqueNickname(std::string const & newNickname);
	void changeNickname(std::string input, std::vector<Client>::iterator it);
	bool uniqueUsername(std::string const & newUsername);
	void changeUsername(std::string input, std::vector<Client>::iterator it);
	void joinChannel(std::string input, std::vector<Client>::iterator it);
	void leaveChannel(std::string input, std::vector<Client>::iterator it);
	void handleWho(std::string input, std::vector<Client>::iterator it);
	void handleMode(std::string input, std::vector<Client>::iterator it);
	void handleQuit(std::string input, std::vector<Client>::iterator it);
	void handleTopic(std::string input, std::vector<Client>::iterator it);
	void handleKick(std::string input, std::vector<Client>::iterator it);
	void handleInvite(std::string input, std::vector<Client>::iterator it);
	static void broadcastMessage(std::string const &message, std::vector<Channel>::iterator it_channel);

};


std::string getFirstWord(const std::string& input);
std::string timeToString(time_t time);
std::vector<std::string> argsSplit(const std::string & s);
std::vector<std::string> commaSplit(const std::string & s);

#endif