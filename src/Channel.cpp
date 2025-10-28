#include "../includes/Channel.hpp"

Channel::Channel(std::string name, Server* serv) : name(name), server(serv)
{
	topic = "";
	topicSetter = "";
	inviteOnly = false;
	topicLocked = false;
	limit = 0; // Default value for no limit
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

bool Channel::isTopicLocked() const {
	return topicLocked;
}

void Channel::setTopicLock(bool const add) {
	topicLocked = add;
	changeFlag('t', add);
}

bool Channel::hasKey() const {
	return !key.empty();
}

bool Channel::checkKey(std::string const &attempted) const {
		return key == attempted;
}

std::string Channel::getKey() const {
	return key;
}

void Channel::setKey(std::string const & newKey, bool const add) {
	if (add && !newKey.empty()) {
		key = newKey;
		changeFlag('k', add);
	}
	else {
		key.clear();
		changeFlag('k', add);
	}
}

bool Channel::hasLimit() const {
	return limit > 0;
}

void Channel::setLimit(int const newLimit, bool const add) {
	if (add && newLimit > 0)
		limit = newLimit;
	else
		limit = 0;
	changeFlag('l', add);
}

int Channel::getLimit() const {
	return limit;
}
