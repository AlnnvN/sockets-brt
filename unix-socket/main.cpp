#include "networkmanager.hpp"
#include <memory>

int main(int argc, char *argv[])
{
    auto socket = std::make_unique<bahiart::NetworkManager::UdpSocket>();
    std::string message;
    try
    {
        socket->setupAddress(argv[1], argv[2]);
        //socket->openConnection();

        std::cout << "Write a message: ";
        std::getline(std::cin,message);
        socket->sendMessage(message);
        
        while (socket->receiveMessage())
            std::cout << "Actual Message: " << socket->getBuffer().data() << std::endl;
        
    }
    catch (const std::exception& exception)
    {
        std::cout << "std::exception - Default exception at main()" << std::endl;
        return 0;
    }
    return 0;
}
