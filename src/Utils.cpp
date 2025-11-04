#include "../includes/Server.hpp"

std::string getFirstWord(const std::string& input) {
	size_t space_pos = input.find(' ');
	if (space_pos != std::string::npos) {
		return input.substr(0, space_pos);
	}
	return input;
}

std::string timeToString(time_t time)
{
	std::stringstream ss;
	ss << time;
	return ss.str();
}

std::vector<std::string> argsSplit(const std::string& s)
{
	std::vector<std::string> result;
	std::istringstream iss(s);
	std::string token;

	while (iss >> token) {
		if (token.empty())
			continue;
		result.push_back(token);
	}
	return result;
}