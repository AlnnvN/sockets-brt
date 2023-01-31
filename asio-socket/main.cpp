#include "networkmanager.hpp"
#include <memory>

int main(int argc, char * argv[]) {

    std::unique_ptr<bahiart::NetworkManager::Socket> socket = std::make_unique<bahiart::NetworkManager::UdpSocket>();
    unsigned short port = std::stoi(argv[2]);

    std::string message {};

    socket->setupAddress(argv[1], port);
    socket->openConnection();
    std::cout<< "Write your message: ";
    std::getline(std::cin, message);
    socket->sendMessage(message);


    std::cout << "Successful." << std::endl;

    return 0;
}