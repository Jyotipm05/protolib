#include <cstdint>
#include <iostream>
#include <string>

#include "protolib/server/UDP.hpp"

int main(int argc, char** argv) {
    std::uint16_t port = 9000;
    if (argc >= 2) {
        const int parsed_port = std::stoi(argv[1]);
        if (parsed_port > 0 && parsed_port <= 65535) {
            port = static_cast<std::uint16_t>(parsed_port);
        }
    }

    plib::server::UDP server;
    if (!server.open()) {
        std::cerr << "Failed to open UDP server socket. WSA error: " << server.last_error() << std::endl;
        return 1;
    }

    if (!server.bind(port, "0.0.0.0")) {
        std::cerr << "Failed to bind UDP server on port " << port << ". WSA error: " << server.last_error() << std::endl;
        return 1;
    }

    std::cout << "UDP server listening on 0.0.0.0:" << port << std::endl;
    std::cout << "Type reply text after each incoming message. Use /quit to exit." << std::endl;

    while (true) {
        plib::server::UdpEndpoint sender{};
        auto incoming = server.recv_from(sender, 2048);
        if (!incoming.has_value()) {
            std::cerr << "Receive failed. WSA error: " << server.last_error() << std::endl;
            continue;
        }

        std::cout << "[" << sender.address << ":" << sender.port << "] " << incoming.value() << std::endl;

        std::cout << "reply> ";
        std::string reply;
        if (!std::getline(std::cin, reply)) {
            break;
        }

        if (reply == "/quit") {
            break;
        }

        if (!reply.empty() && !server.send_to(sender, reply)) {
            std::cerr << "Send failed. WSA error: " << server.last_error() << std::endl;
        }
    }

    return 0;
}

