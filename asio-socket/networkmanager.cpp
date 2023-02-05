#include "networkmanager.hpp"
 

void bahiart::NetworkManager::TcpSocket::setupAddress(std::string HOST_NAME, unsigned short PORT)
{

    try {
        this->endPoint = std::make_shared<boost::asio::ip::tcp::endpoint>(boost::asio::ip::address::from_string(HOST_NAME),  PORT);
    }
    catch (boost::system::system_error &e) {
        std::cout 
        << "Error occured! Error code = " << e.code()
        << ". Message " << e.what() <<std::endl;

        return;
    }
}

void bahiart::NetworkManager::TcpSocket::openConnection()
{
    try {
        
        boost::asio::io_service ios;

        this->socket = std::make_shared<boost::asio::ip::tcp::socket>(ios, this->endPoint->protocol());
        this->socket->connect(*(this->endPoint));
    }
    catch (boost::system::system_error &e) {
        std::cout 
        << "Error occured! Error code = " << e.code()
        << ". Message " << e.what() <<std::endl;

        return;
    }
}

void bahiart::NetworkManager::TcpSocket::sendMessage(std::string message)
{
    std::size_t bytesWritten {};
    try{
        while( bytesWritten != message.length()){

            bytesWritten += this->socket->write_some(boost::asio::buffer(message.c_str()+bytesWritten, message.length() - bytesWritten));
        }
    }
      catch (boost::system::system_error &e) {
        std::cout 
        << "Error occured! Error code = " << e.code()
        << ". Message " << e.what() <<std::endl;

        return;
      }

}

bahiart::NetworkManager::TcpSocket::~TcpSocket(){
    this->socket->close();

}

void bahiart::NetworkManager::UdpSocket::setupAddress(std::string HOST_NAME, unsigned short PORT)
{   
    
    try{

    this->endPoint = std::make_shared<boost::asio::ip::udp::endpoint>(boost::asio::ip::address::from_string(HOST_NAME), PORT);
    }
    catch (boost::system::system_error &e) {
        std::cout 
        << "Error occured! Error code = " << e.code()
        << ". Message " << e.what() <<std::endl;

        return;
      }

}

void bahiart::NetworkManager::UdpSocket::openConnection()
{   
    try{


        boost::asio::io_service io_service;
        this->socket = std::make_shared<boost::asio::ip::udp::socket>(io_service);
        this->socket->open(boost::asio::ip::udp::v4());

        



    }
   catch (boost::system::system_error &e) {
        std::cout 
        << "Error occured! Error code = " << e.code()
        << ". Message " << e.what() <<std::endl;

        return;
   }

}

void bahiart::NetworkManager::UdpSocket::sendMessage(std::string message)
{
    try{
        boost::system::error_code err;
        
        auto sent = this->socket->send_to(boost::asio::buffer(message), *(this->endPoint), 0, err);

        
    }
      catch (boost::system::system_error &e) {
        std::cout
        << "Error occured! Error code = " << e.code()
        << ". Message " << e.what() <<std::endl;

        return;
      }


}

bahiart::NetworkManager::UdpSocket::~UdpSocket(){
    this->socket->close();
}

bahiart::NetworkManager::Socket::~Socket(){}

