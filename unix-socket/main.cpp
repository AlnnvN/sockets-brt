#include "networkmanager.hpp"
#include <memory>

int main(int argc, char *argv[])
{
    std::unique_ptr<bahiart::NetworkManager::Socket> socket = std::make_unique<bahiart::NetworkManager::TcpSocket>();
    std::string message;
    try
    {
        socket->setupAddress(argv[1], argv[2]);
        socket->openConnection();

        std::cout << "Write a message: ";
        std::getline(std::cin,message);

        socket->sendMessage(message);
    }
    catch (std::exception exception)
    {
        std::cout << "std::exception - Default exception at main()" << std::endl;
        return 0;
    }
    return 0;
}