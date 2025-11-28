#include "../includes/Server.hpp"

int main(int argc, char **argv)
{
	if (argc < 3 || argc > 3) {
		std::cout << "Wrong amount of arguments" << std::endl;
		return (1);
	}

	if (!Server::SignalHandler::setup())
		exit(1);

	Server server;

	try {
		server.setPort(std::atoi(argv[1]));
	} catch (std::exception& e) {
		std::cout << "Invalid Port" << std::endl;
		return (1);
	}
	try {
		server.setPassword(std::string(argv[2]));
	} catch (std::exception& e) {
		std::cout << "Invalid Password" << std::endl;
		return (1);
	}
	
	std::cout << server.getPort() << " " << server.getPassword() << std::endl;

	server.startServer();
}

// to connect via terminal "nc ip port"