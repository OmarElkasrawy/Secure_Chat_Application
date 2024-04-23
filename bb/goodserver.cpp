#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <mutex>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

const int PORT = 10101;
const int MAX_CLIENTS = 10;
int client_sockets[MAX_CLIENTS];
std::mutex mtx;
int client_count = 0;

// Function to encrypt a message using Caesar cipher
std::string caesar_encrypt(const std::string& message, int shift) {
    std::string result = "";
    for (char c : message) {
        if (std::isalpha(c)) {
            char shifted = std::islower(c) ? 'a' + (c - 'a' + shift) % 26 : 'A' + (c - 'A' + shift) % 26;
            result += shifted;
        } else {
            result += c;
        }
    }
    return result;
}

void remove_client(int client_socket) {
    std::lock_guard<std::mutex> lock(mtx);
    for(int i = 0; i < MAX_CLIENTS; i++) {
        if (client_sockets[i] == client_socket) {
            client_sockets[i] = -1;
            break;
        }
    }
}

void handle_client(int client_socket, std::ofstream& chat_log) {
    int current_client_number;
    {
        std::lock_guard<std::mutex> lock(mtx);
        client_count++;
        current_client_number = client_count;
    }


    char buffer[1024];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            break;
        }

        // Print the original (unencrypted) message to the server terminal
        std::cout << "Client " << client_socket << ": " << buffer << std::endl;

        std::lock_guard<std::mutex> lock(mtx);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_sockets[i] != -1 && client_sockets[i] != client_socket) {
                send(client_sockets[i], buffer, bytes_received, 0);
            }
        }

        // Encrypt the received message before writing it to the chat log file
        std::string encrypted_message = caesar_encrypt(buffer, 3); // Using Caesar cipher with a shift of 3
        chat_log << "Client " << client_socket << ": " << encrypted_message << std::endl;
    }

    close(client_socket);
    remove_client(client_socket);

    std::lock_guard<std::mutex> lock(mtx);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_sockets[i] == client_socket) {
            client_sockets[i] = -1;
            break;
        }
    }
}

int main() {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        std::cerr << "Error creating socket" << std::endl;
        return 1;
    }

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Error binding socket" << std::endl;
        return 1;
    }

    if (listen(server_socket, 5) < 0) {
        std::cerr << "Error listening on socket" << std::endl;
        return 1;
    }

    std::cout << "Server started on port " << PORT << std::endl;

    memset(client_sockets, -1, sizeof(client_sockets));

    std::ofstream chat_log("chat_log.txt");
    if (!chat_log) {
        std::cerr << "Error opening chat log file" << std::endl;
        return 1;
    }

    while (true) {
        sockaddr_in client_addr;
        socklen_t client_addr_size = sizeof(client_addr);
        int client_socket = accept(server_socket, (sockaddr*)&client_addr, &client_addr_size);
        if (client_socket < 0) {
            std::cerr << "Error accepting connection" << std::endl;
            continue;
        }

        std::lock_guard<std::mutex> lock(mtx);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_sockets[i] == -1) {
                client_sockets[i] = client_socket;
                break;
            }
        }

        std::thread t(handle_client, client_socket, std::ref(chat_log));
        t.detach();
    }

    chat_log.close();
    close(server_socket);
    return 0;
}
