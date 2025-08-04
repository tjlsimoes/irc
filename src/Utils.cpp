#include "../includes/Server.hpp"

std::string getFirstWord(const std::string& input) {
	size_t space_pos = input.find(' ');
    std::cout << space_pos << std::endl;
	if (space_pos != std::string::npos) {
		return input.substr(0, space_pos);
	}
	return input;
}
