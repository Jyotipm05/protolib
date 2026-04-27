/**
 * @file UDP.hpp
 * @brief UDP server socket wrapper for IPv4 datagram communication.
 */

#pragma once


#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>

namespace plib::server {

/** @brief IPv4 endpoint used for UDP send/receive operations. */
struct UdpEndpoint {
    /** @brief Dotted-decimal IPv4 address (for example, 0.0.0.0). */
    std::string address;
    /** @brief UDP port in host byte order. */
    std::uint16_t port{};
};

/** @brief Minimal UDP server API backed by Winsock2. */
class UDP {
public:
    /** @brief Constructs an empty UDP server with no open socket. */
    UDP() = default;
    /** @brief Closes the socket if it is currently open. */
    ~UDP();

    UDP(const UDP&) = delete;
    UDP& operator=(const UDP&) = delete;

    /** @brief Transfers socket ownership from another instance. */
    UDP(UDP&& other) noexcept;
    /** @brief Replaces this socket with ownership from another instance. */
    UDP& operator=(UDP&& other) noexcept;

    /** @brief Opens a UDP socket.
     *  @return True on success, false on failure.
     */
    bool open();
    /** @brief Binds the socket to a local address and port.
     *  @param port Local UDP port.
     *  @param address Local IPv4 address.
     *  @return True if bind succeeds.
     */
    bool bind(std::uint16_t port, const std::string& address = "0.0.0.0");
    /** @brief Closes the currently open UDP socket. */
    void close();

    /** @brief Sends raw bytes to the destination endpoint.
     *  @param endpoint Destination IP and port.
     *  @param data Pointer to bytes to send.
     *  @param size Number of bytes to send.
     *  @return True if send succeeds.
     */
    bool send_to(const UdpEndpoint& endpoint, const std::byte* data, std::size_t size);
    /** @brief Sends text bytes to the destination endpoint.
     *  @param endpoint Destination IP and port.
     *  @param text Message payload.
     *  @return True if send succeeds.
     */
    bool send_to(const UdpEndpoint& endpoint, const std::string& text);

    /** @brief Receives one UDP datagram.
     *  @param sender Output sender endpoint.
     *  @param max_bytes Maximum payload bytes to read.
     *  @return Received payload on success, or std::nullopt on failure/timeout.
     */
    std::optional<std::string> recv_from(UdpEndpoint& sender, int max_bytes = 2048);
    /** @brief Sets socket receive timeout in milliseconds.
     *  @param timeout_ms Timeout value in ms.
     *  @return True if option update succeeds.
     */
    bool set_recv_timeout_ms(int timeout_ms);

    /** @brief Returns the latest Winsock error code observed by this instance. */
    int last_error() const noexcept;

private:
    std::uintptr_t socket_handle_{};
    int last_error_{};
};

} // namespace protolib::server

