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
    while (true) {
        char buffer[1024];
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) {
            break;
        }

        // null-terminate
        buffer[bytes_received] = '\0';
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

    // auth
    std::string username, password;
    bool authenticated = false;
    while(!authenticated) {
        std::cout << "Enter username: ";
        std::getline(std::cin, username);
        std::cout << "Username length: " << username.length() << std::endl;

        std::cout << "Enter password: ";
        std::getline(std::cin, password);
        std::cout << "Password length: " << password.length() << std::endl;


        // clear input buffer
        // std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');


        int username_length = username.size();
        send(sock, &username_length, sizeof(username_length), 0);
        // debug print length of username sent to server
        std::cout << "Sent username length: " << username_length << std::endl;

        send(sock, username.c_str(), username_length, 0);


        int password_length = password.size();
        send(sock, &password_length, sizeof(password_length), 0);
        // debug print length password sent to server
        std::cout << "Sent password length: " << password_length << std::endl;

        send(sock, password.c_str(), password.size(), 0);

        char auth_result[1024];
        recv(sock, auth_result, sizeof(auth_result), 0);
        // null-terminate
        auth_result[sizeof(auth_result) - 1] = '\0';
        if(std::string(auth_result) == "OK") {
            authenticated = true;
            std::cout << "Authentication successful!" << std::endl;
        } else {
            std::cout << "Authentication failed!" << std::endl;
        }
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