#include "SocketClient.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

SocketClient::SocketClient(const std::string& host, int port)
    : host(host), port(port), sockfd(-1), connected(false) {}

SocketClient::~SocketClient() {
    disconnect();
}
bool SocketClient::connectToServer() {
    if (SocketClient::connected) {
        std::cerr << "Already connected to server.\n";
        return false;
    }

    SocketClient::sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (SocketClient::sockfd < 0) {
        std::cerr << "Error creating socket.\n";
        return false;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SocketClient::port);
    inet_pton(AF_INET, SocketClient::host.c_str(), &server_addr.sin_addr);

    if (connect(SocketClient::sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Error connecting to server.\n";
        close(SocketClient::sockfd);
        SocketClient::sockfd = -1;
        return false;
    }

    SocketClient::connected = true;
    return true;
}

void SocketClient::disconnect() {
    if (SocketClient::connected) {
        close(SocketClient::sockfd);
        SocketClient::sockfd = -1;
        SocketClient::connected = false;
    }
}

bool SocketClient::sendMessage(const std::string& message) {
    if (!SocketClient::connected) {
        std::cerr << "Not connected to server.\n";
        return false;
    }

    ssize_t bytes_sent = send(SocketClient::sockfd, message.c_str(), message.size(), 0);
    if (bytes_sent < 0) {
        std::cerr << "Error sending message.\n";
        return false;
    } else if (static_cast<size_t>(bytes_sent) < message.size()) {
        std::cerr << "Partial message sent.\n";
        return false;
    }

    return true;
}
