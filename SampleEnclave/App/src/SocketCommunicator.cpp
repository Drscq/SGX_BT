#include "SocketCommunicator.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/tcp.h>  // For TCP_NODELAY
#include <unistd.h>
#include <cstring>
#include <iostream>

SocketCommunicator::SocketCommunicator() : sockfd(-1) {}

SocketCommunicator::~SocketCommunicator() {
    if (sockfd != -1) {
        close(sockfd);
    }
}

void SocketCommunicator::setSocketBufferSize(int sockfd, int buffer_size) {
    if (setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &buffer_size, sizeof(buffer_size)) < 0) {
        std::cerr << "Error: Failed to set socket send buffer size" << std::endl;
    }
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &buffer_size, sizeof(buffer_size)) < 0) {
        std::cerr << "Error: Failed to set socket receive buffer size" << std::endl;
    }
}


bool SocketCommunicator::isConnected() {
    return sockfd != -1;
}

bool SocketCommunicator::connectToServer(const std::string& host, int port) {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    int flag = 1;
    if (setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(int)) == -1) {
        std::cerr << "Error: Failed to set TCP_NODELAY" << std::endl;
        // Handle error as appropriate
    }
    // // Set optimal buffer size for the socket
    // setSocketBufferSize(sockfd, this->optimizedBufferSize);

    if (sockfd == -1) {
        std::cerr << "Error: Unable to create socket" << std::endl;
        return false;
    }
    
    int optval = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, host.c_str(), &serv_addr.sin_addr) <= 0) {
        std::cerr << "Error: Invalid address/ Address not supported" << std::endl;
        return false;
    }

    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Error: Connection Failed" << std::endl;
        return false;
    }
    return true;
}

bool SocketCommunicator::listenOnPort(int port) {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        std::cerr << "Error: Unable to create socket" << std::endl;
        return false;
    }
    //  // Set optimal buffer size for the socket
    // setSocketBufferSize(sockfd, this->optimizedBufferSize);

    int flag = 1;
    if (setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(int)) == -1) {
        std::cerr << "Error: Failed to set TCP_NODELAY" << std::endl;
        // Handle error as appropriate
    }


    int optval = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Error: Binding Failed" << std::endl;
        return false;
    }

    if (listen(sockfd, 3) < 0) {
        std::cerr << "Error: Listening Failed" << std::endl;
        return false;
    }

    return true;
}

int SocketCommunicator::acceptConnection() {
    sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_sock = accept(sockfd, (struct sockaddr*)&client_addr, &client_len);
    if (client_sock < 0) {
        std::cerr << "Error: Accepting Connection Failed" << std::endl;
        return -1;
    }
    
    return client_sock;
}

ssize_t SocketCommunicator::sendData(int sockfd, const std::vector<char>& data) {
    size_t totalSent = 0;
    ssize_t sent;
    while (totalSent < data.size()) {
        sent = send(sockfd, data.data() + totalSent, data.size() - totalSent, 0);
        if (sent == -1) {
            std::cerr << "Error: Sending Data Failed" << std::endl;
            return -1;
        }
        totalSent += sent;
    }
    return totalSent;
}

// ssize_t SocketCommunicator::sendData(int sockfd, const void* data, size_t size) {
//     size_t totalSent = 0;
//     ssize_t sent;
//     while (totalSent < size) {
//         sent = send(sockfd, static_cast<const char*>(data) + totalSent, size - totalSent, 0);
//         if (sent == -1) {
//             std::cerr << "Error: Sending Data Failed" << std::endl;
//             return -1;
//         }
//         totalSent += sent;
//     }
//     return totalSent;
// }
ssize_t SocketCommunicator::sendData(int sockfd, const void* data, size_t size) {
    if (!sendAll(sockfd, static_cast<const char*>(data), size)) {
        std::cerr << "Error: Sending Data Failed" << std::endl;
        return -1;
    }
    return size;
}

ssize_t SocketCommunicator::receiveData(int sockfd, std::vector<char>& buffer, size_t size) {
    buffer.resize(size);
    size_t totalReceived = 0;
    ssize_t received;
    while (totalReceived < size) {
        received = recv(sockfd, buffer.data() + totalReceived, size - totalReceived, 0);
        if (received == -1) {
            std::cerr << "Error: Receiving Data Failed" << std::endl;
            return -1;
        }
        totalReceived += received;
    }
    return totalReceived;
}

// ssize_t SocketCommunicator::receiveData(int sockfd, void* buffer, size_t size) {
//     size_t totalReceived = 0;
//     ssize_t received;
//     while (totalReceived < size) {
//         received = recv(sockfd, static_cast<char*>(buffer) + totalReceived, size - totalReceived, 0);
//         if (received == -1) {
//             std::cerr << "Error: Receiving Data Failed" << std::endl;
//             return -1;
//         }
//         totalReceived += received;
//     }
//     return totalReceived;
// }

ssize_t SocketCommunicator::receiveData(int sockfd, void* buffer, size_t size) {
    if (!recvAll(sockfd, static_cast<char*>(buffer), size)) {
        std::cerr << "Error: Receiving Data Failed" << std::endl;
        return -1;
    }
    return size;
}


ssize_t SocketCommunicator::sendCommand(int sockfd, int command) {
    ssize_t sent = send(sockfd, &command, sizeof(command), 0);
    if (sent == -1) {
        std::cerr << "Error: Sending Command Failed" << std::endl;
        return -1;
    }
    return sent;
}

ssize_t SocketCommunicator::receiveCommand(int sockfd, int& command) {
    ssize_t received = recv(sockfd, &command, sizeof(command), 0);
    if (received == -1) {
        std::cerr << "Error: Receiving Command Failed" << std::endl;
        return -1;
    }
    return received;
}

int SocketCommunicator::getSockfd() const {
    return sockfd;
}

void SocketCommunicator::closeSocket() {
    if (sockfd != -1) {
        close(sockfd);
        sockfd = -1;
    }
}

ssize_t SocketCommunicator::sendDataWithoutKnownSize(int sockfd, const std::vector<char>& data) {
    size_t dataLength = data.size();
    if (!sendAll(sockfd, reinterpret_cast<const char*>(&dataLength), sizeof(dataLength))) {
        return false;
    }

    return sendAll(sockfd, data.data(), data.size());
}

ssize_t SocketCommunicator::receiveDataWithoutKnownSize(int sockfd, std::vector<char>& buffer) {
    size_t dataLength;
    if (!recvAll(sockfd, reinterpret_cast<char*>(&dataLength), sizeof(dataLength))) {
        return false;
    }

    buffer.resize(dataLength);
    return recvAll(sockfd, buffer.data(), dataLength);
}
bool SocketCommunicator::sendAll(int sockfd, const char* buf, size_t len) {
    size_t total = 0;
    while (total < len) {
        ssize_t n = send(sockfd, buf + total, len - total, 0);
        if (n == -1) { return false; }
        total += n;
    }
    return true;
}

bool SocketCommunicator::recvAll(int sockfd, char* buf, size_t len) {
    size_t total = 0;
    while (total < len) {
        ssize_t n = recv(sockfd, buf + total, len - total, 0);
        if (n == -1 || n == 0) { return false; }
        total += n;
    }
    return true;
}
