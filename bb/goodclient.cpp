#include <iostream>
#include <string>
#include <thread>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

const int PORT = 10101;
const std::string SERVER_ADDRESS = "127.0.0.1";

void receive_messages(int sock) {
    char buffer[1024];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(sock, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            break;
        }

        std::cout << "Received: " << buffer << std::endl;
    }
}

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Error creating socket" << std::endl;
        return 1;
    }

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_ADDRESS.c_str(), &server_addr.sin_addr);

    if (connect(sock, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Error connecting to server" << std::endl;
        return 1;
    }

    std::thread t(receive_messages, sock);
    t.detach();

    std::string input;
    while (std::getline(std::cin, input)) {
        send(sock, input.c_str(), input.size(), 0);
    }

    close(sock);
    return 0;
}