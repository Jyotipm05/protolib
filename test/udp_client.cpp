#include <cstdint>
#include <iostream>
#include <string>

#include "protolib/client/UDP.hpp"

int main(int argc, char** argv) {
    std::string server_host = "127.0.0.1";
    std::uint16_t server_port = 9000;

    if (argc >= 2) {
        server_host = argv[1];
    }

    if (argc >= 3) {
        const int parsed_port = std::stoi(argv[2]);
        if (parsed_port > 0 && parsed_port <= 65535) {
            server_port = static_cast<std::uint16_t>(parsed_port);
        }
    }

    plib::client::UDP client;
    if (!client.open()) {
        std::cerr << "Failed to open UDP client socket. WSA error: " << client.last_error() << std::endl;
        return 1;
    }


    plib::client::UdpEndpoint server_endpoint{server_host, server_port};

    std::cout << "UDP client connected to " << server_host << ":" << server_port << std::endl;
    std::cout << "Type messages and press Enter to send. Use /quit to exit." << std::endl;

    while (true) {
        std::cout << "message> ";
        std::string message;
        if (!std::getline(std::cin, message)) {
            break;
        }

        if (message == "/quit") {
            break;
        }

        if (!client.send_to(server_endpoint, message)) {
            std::cerr << "Send failed. WSA error: " << client.last_error() << std::endl;
            continue;
        }

        plib::client::UdpEndpoint sender{};
        auto reply = client.recv_from(sender, 2048);
        if (!reply.has_value()) {
            std::cerr << "No response (or receive failed). WSA error: " << client.last_error() << std::endl;
            continue;
        }

        std::cout << "reply< [" << sender.address << ":" << sender.port << "] " << reply.value() << std::endl;
    }

    return 0;
}

