#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <mutex>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <algorithm>
#include <cctype>

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

// trim leading and trailing whitespace
std::string trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    size_t end = str.find_last_not_of(" \t\r\n");
    if (start == std::string::npos || end == std::string::npos)
        return "";
    return str.substr(start, end - start + 1);
}

bool authenticate(const std::string& username, const std::string& password) {
    std::ifstream credentials_file("credentials.txt");
    if(!credentials_file) {
        std::cerr << "Error opening the creds file!" << std::endl;
        return false;
    }

    std::string stored_username, stored_password;
    while (credentials_file >> stored_username >> stored_password) {
        // debug
        std::cout << "Read username: " << stored_username << ", Read password: " << stored_password << std::endl;
        if (stored_username == username && stored_password == password) {
            //debug
            std::cout << "Entered username: " << username << ", Entered password: " << password << std::endl;
            std::cout << "Authenticated!" << std::endl;
            return true;
        }
    }

    std::cout << "Entered username: " << username << ", Entered password: " << password << std::endl;
    std::cout << "Invalid credentials!";
    return false;
}

void handle_client(int client_socket, std::ofstream& chat_log) {
    int current_client_number;
    {
        std::lock_guard<std::mutex> lock(mtx);
        client_count++;
        current_client_number = client_count;
    }


    char buffer[1024];

    bool authenticated = false;
    std::string username, password;
    while (!authenticated) {
        // recv username length
        int username_length;
        int bytes_received = recv(client_socket, &username_length, sizeof(username_length), 0);
        if (bytes_received <= 0) {
            close(client_socket);
            return;
        }


        // debug recv username length
        std::cout << "Received username length: " << username_length << std::endl;






        // recv username
        bytes_received = recv(client_socket, buffer, username_length, 0);
        if (bytes_received <= 0) {
            close(client_socket);
            return;
        }

        buffer[bytes_received] = '\0';
        username = std::string(buffer, username_length);
        // debug
        std::cout << "Read username: " << username << ", Read username length: " << bytes_received << std::endl;

        // recv password length
        int password_length;
        bytes_received = recv(client_socket, &password_length, sizeof(password_length), 0);
        if (bytes_received <= 0) {
            close(client_socket);
            return;
        }

        // debug recv password length
        std::cout << "Received password length: " << password_length << std::endl;

        // recv password
        bytes_received = recv(client_socket, buffer, password_length, 0);
        if (bytes_received <= 0) {
            close(client_socket);
            return;
        }

        buffer[bytes_received] = '\0';
        password = std::string(buffer, password_length);
        // debug
        std::cout << "Read password: " << password << ", Read password length: " << bytes_received << std::endl;

        // check creds
        authenticated = authenticate(username, password);
        // debug
        std::cout << "Entered username: " << username << ", Entered password: " << password << std::endl;

        // now send auth to client
        const char* auth_result = authenticated ? "OK" : "ERROR";
        send(client_socket, auth_result, strlen(auth_result), 0);

        if (!authenticated) {
            close(client_socket);
            return;
        }

    }

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
