#pragma once
#include <string>

class SocketClient {
private:
    std::string host;
    int port;
    int sockfd;
    bool connected;
public:
    SocketClient(){};
    SocketClient(const std::string& host, int port);
    ~SocketClient();

    void setHost(const std::string& host) { this->host = host; }
    void setPort(int port) { this->port = port; }

    bool connectToServer();
    void disconnect();
    bool sendMessage(const std::string& message);
};
