#include <iostream>

#include "Sender.hpp"
#include "Receiver.hpp"

int main() {
	int menu = 0;
	std::cout << "1 for Sender, 2 for Receiver: " << std::endl;
	std::cin >> menu;

	if (menu == 1) {
		Sender sender;
		sender.send();
	}
	else if (menu == 2) {
		Receiver receiver;
		receiver.receive();		
	}
	return 0;
}
