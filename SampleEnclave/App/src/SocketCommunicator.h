#ifndef SOCKETCOMMUNICATOR_H
#define SOCKETCOMMUNICATOR_H

#include <string>
#include <vector>

class SocketCommunicator {
public:
    SocketCommunicator();
    ~SocketCommunicator();

    bool connectToServer(const std::string& host, int port);
    bool listenOnPort(int port);
    int acceptConnection();
    int getSockfd() const;
    void setSocketBufferSize(int sockfd, int buffer_size);

    ssize_t sendCommand(int sockfd, int command);
    ssize_t receiveCommand(int sockfd, int& command);
    ssize_t sendData(int sockfd, const std::vector<char>& data);
    ssize_t sendData(int sockfd, const void* data, size_t size);
    ssize_t receiveData(int sockfd, std::vector<char>& buffer, size_t size);
    ssize_t receiveData(int sockfd, void* buffer, size_t size);
    ssize_t sendDataWithoutKnownSize(int sockfd, const std::vector<char>& data);
    ssize_t receiveDataWithoutKnownSize(int sockfd, std::vector<char>& buffer);
    // Utility methods
    bool sendAll(int sockfd, const char* buf, size_t len);
    bool recvAll(int sockfd, char* buf, size_t len);
    bool isConnected();
    int optimizedBufferSize = 1024 * 1024 * 10; // 10 MB

private:
    int sockfd; // Socket file descriptor

    void closeSocket();
};

#endif // SOCKETCOMMUNICATOR_H
