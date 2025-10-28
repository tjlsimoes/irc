#include "../includes/Channel.hpp"

Channel::Channel(std::string name, Server* serv) : name(name), server(serv)
{
	topic = "";
	topicSetter = "";
	inviteOnly = false;
}

Channel::~Channel()
{
	clients.clear();
	ops.clear();
	flags.clear();
}

void Channel::addClient(Client client) {
	clients.push_back(client);
}

void Channel::removeClient(Client client) {
	for (std::vector<Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
		if (it->getClientFd() == client.getClientFd()) {
			clients.erase(it);
			break;
		}
	}
}

std::string Channel::getName() const
{
	return name;
}

std::vector<Client> Channel::getClients() const
{
	return clients;
}

void Channel::addOp(Client client)
{
	ops.push_back(client);
}

void Channel::removeOp(Client client)
{
	for (std::vector<Client>::iterator it = ops.begin(); it != ops.end(); ++it) {
		if (it->getClientFd() == client.getClientFd()) {
			ops.erase(it);
			break;
		}
	}
}

std::vector<char> Channel::getFlags() const
{
	return flags;
}

void Channel::addFlag(char flag)
{
	flags.push_back(flag);
}

void Channel::removeFlag(char flag)
{
	for (std::vector<char>::iterator it = flags.begin(); it != flags.end(); ++it) {
		if (*it == flag) {
			flags.erase(it);
			break;
		}
	}
}

bool Channel::isOperator(Client client) const
{
	for (size_t i = 0; i < ops.size(); i++) {
		if (ops[i].getClientFd() == client.getClientFd()) {
			return true;
		}
	}
	return false;
}

void Channel::setTopic(const std::string& newTopic, const std::string& setter)
{
	topic = newTopic;
	timestamp = time(0);
	topicSetter = setter;
}

std::string Channel::getTopic() const
{
	return topic;
}

time_t Channel::getTimestamp() const
{
	return timestamp;
}

std::string Channel::getTopicSetter() const
{
	return topicSetter;
}

void Channel::changeFlag(char const flag, bool const add) {
	if (add)
		addFlag(flag);
	else
		removeFlag(flag);
}

bool Channel::isInviteOnly() const {
	return inviteOnly;
}

void Channel::setInviteOnly(bool const add) {
	inviteOnly = add;
	changeFlag('i', add);
}

void Channel::addInvite(std::string const &nickname) {
	inviteList.push_back(nickname);
}

void Channel::removeInvite(std::string const &nickname) {
	for (std::vector<std::string>::iterator it = inviteList.begin(); it != inviteList.end(); ++it) {
		if (*it == nickname) {
			inviteList.erase(it);
			break;
		}
	}
}

bool Channel::isInvited(std::string const &nickname) {
	return std::count(inviteList.begin(), inviteList.end(), nickname);
}
