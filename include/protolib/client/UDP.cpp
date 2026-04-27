/**
 * @file UDP.cpp
 * @brief Winsock2-backed implementation of the UDP client wrapper.
 */

#include "UDP.hpp"

#include <cstring>
#include <mutex>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>

namespace {

constexpr std::uintptr_t kInvalidSocketHandle = INVALID_SOCKET;

/** @brief Ensures WSAStartup is executed once for the process. */
bool ensure_winsock_started(int& error_code) {
    static std::once_flag once;
    static int startup_result = 0;

    std::call_once(once, [] {
        WSADATA data{};
        startup_result = ::WSAStartup(MAKEWORD(2, 2), &data);
    });

    error_code = startup_result;
    return startup_result == 0;
}

} // namespace

namespace plib::client {

/** @brief Releases the socket on object destruction. */
UDP::~UDP() {
    close();
}

/** @brief Moves socket ownership from @p other. */
UDP::UDP(UDP&& other) noexcept : socket_handle_(other.socket_handle_), last_error_(other.last_error_) {
    other.socket_handle_ = 0;
    other.last_error_ = 0;
}

/** @brief Move-assigns socket ownership from @p other. */
UDP& UDP::operator=(UDP&& other) noexcept {
    if (this == &other) {
        return *this;
    }

    close();
    socket_handle_ = other.socket_handle_;
    last_error_ = other.last_error_;

    other.socket_handle_ = 0;
    other.last_error_ = 0;
    return *this;
}

/** @brief Opens a datagram socket after ensuring Winsock startup. */
bool UDP::open() {
    close();

    if (!ensure_winsock_started(last_error_)) {
        return false;
    }

    const SOCKET socket_fd = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (socket_fd == INVALID_SOCKET) {
        last_error_ = ::WSAGetLastError();
        return false;
    }

    socket_handle_ = static_cast<std::uintptr_t>(socket_fd);
    last_error_ = 0;
    return true;
}

/** @brief Closes the current socket handle if it is valid. */
void UDP::close() {
    if (socket_handle_ == 0 || socket_handle_ == kInvalidSocketHandle) {
        socket_handle_ = 0;
        return;
    }

    ::closesocket(socket_handle_);
    socket_handle_ = 0;
}

/** @brief Sends a raw UDP payload to a destination endpoint. */
bool UDP::send_to(const UdpEndpoint& endpoint, const std::byte* data, std::size_t size) {
    if (socket_handle_ == 0) {
        last_error_ = WSAENOTSOCK;
        return false;
    }

    sockaddr_in destination{};
    destination.sin_family = AF_INET;
    destination.sin_port = ::htons(endpoint.port);

    const int ip_parse = ::InetPtonA(AF_INET, endpoint.address.c_str(), &destination.sin_addr);
    if (ip_parse != 1) {
        last_error_ = WSAEINVAL;
        return false;
    }

    const int sent = ::sendto(
        socket_handle_,
        reinterpret_cast<const char*>(data),
        static_cast<int>(size),
        0,
        reinterpret_cast<const sockaddr*>(&destination),
        sizeof(destination));

    if (sent == SOCKET_ERROR) {
        last_error_ = ::WSAGetLastError();
        return false;
    }

    last_error_ = 0;
    return true;
}

/** @brief Sends a text UDP payload to a destination endpoint. */
bool UDP::send_to(const UdpEndpoint& endpoint, const std::string& text) {
    return send_to(endpoint, reinterpret_cast<const std::byte*>(text.data()), text.size());
}

/** @brief Receives one UDP datagram and captures sender metadata. */
std::optional<std::string> UDP::recv_from(UdpEndpoint& sender, int max_bytes) {
    if (socket_handle_ == 0 || max_bytes <= 0) {
        last_error_ = WSAEINVAL;
        return std::nullopt;
    }

    std::vector<char> buffer(static_cast<std::size_t>(max_bytes));

    sockaddr_in from{};
    int from_length = sizeof(from);
    const int received = ::recvfrom(
        socket_handle_,
        buffer.data(),
        static_cast<int>(buffer.size()),
        0,
        reinterpret_cast<sockaddr*>(&from),
        &from_length);

    if (received == SOCKET_ERROR) {
        last_error_ = ::WSAGetLastError();
        return std::nullopt;
    }

    char address_buffer[INET_ADDRSTRLEN]{};
    const char* text_address = ::InetNtopA(AF_INET, &from.sin_addr, address_buffer, INET_ADDRSTRLEN);
    sender.address = text_address != nullptr ? text_address : "";
    sender.port = ::ntohs(from.sin_port);

    last_error_ = 0;
    return std::string(buffer.data(), static_cast<std::size_t>(received));
}

/** @brief Applies SO_RCVTIMEO to bound blocking receive duration. */
bool UDP::set_recv_timeout_ms(int timeout_ms) {
    if (socket_handle_ == 0 || timeout_ms < 0) {
        last_error_ = WSAEINVAL;
        return false;
    }

    const int result = ::setsockopt(
        socket_handle_,
        SOL_SOCKET,
        SO_RCVTIMEO,
        reinterpret_cast<const char*>(&timeout_ms),
        sizeof(timeout_ms));

    if (result == SOCKET_ERROR) {
        last_error_ = ::WSAGetLastError();
        return false;
    }

    last_error_ = 0;
    return true;
}

/** @brief Returns the most recent Winsock error recorded by the wrapper. */
int UDP::last_error() const noexcept {
    return last_error_;
}

} // namespace protolib::client
