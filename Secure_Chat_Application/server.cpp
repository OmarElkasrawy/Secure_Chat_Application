#include <iostream>
#include <thread>
#include <mutex>
#include <cctype>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <fstream>
#include <algorithm> // can be removed


using namespace std; // dont have to do std:: everytime


const int PORT = 10101;
const int MAX_CLIENTS = 10; // can be changed without any issues

// store client socket desc.
int client_sockets[MAX_CLIENTS];

// used to protect shared data from being accessed by multiple threads for sync
mutex mtx;

// counter so its not (client 1 talking to client 1)
int client_count = 0;

// caesar cipher
string caesar_encrypt(const string& message, int shift) {
	string result = "";
	for (char c : message) {
		if (isalpha(c)) {
			char shifted = islower(c) ? 'a' + (c - 'a' + shift) % 26 : 'A' + (c - 'A' + shift) % 26;
			result += shifted;
		}
		else {
			result += c;
		}
	}
	return result;
}


// remove client from client_sockets
void remove_client(int client_socket) {
	lock_guard<mutex> lock(mtx);
	for (int i = 0; i < MAX_CLIENTS; i++) {
		if (client_sockets[i] == client_socket) {
			client_sockets[i] = -1;
			break;
		}
	}
}

// for proper authentication trim whitespace from string
string trim(const string& str) {
	size_t start = str.find_first_not_of(" \t\r\n");
	size_t end = str.find_last_not_of(" \t\r\n");
	if (start == string::npos || end == string::npos)
		return "";
	return str.substr(start, end - start + 1);
}

// now function to authenticate user by user&pass
bool authenticate(const string& username, const string& password) {
	ifstream credentials_file("credentials.txt");
	if (!credentials_file) {
		cerr << "Error opening the creds file!" << endl;
		return false;
	}

	string stored_username, stored_password;
	while (credentials_file >> stored_username >> stored_password) {
		// debug
		cout << "Read username: " << stored_username << ", Read password: " << stored_password << endl;
		if (stored_username == username && stored_password == password) {
			//debug
			cout << "Entered username: " << username << ", Entered password: " << password << endl;
			cout << "Authenticated!" << endl;
			return true;
		}
	}

	// entered and cond. invalid 
	cout << "Entered username: " << username << ", Entered password: " << password << endl;
	cout << "Invalid credentials!";
	return false;
}

// handle client connection
void handle_client(int client_socket, ofstream& chat_log) {
	int current_client_number;
	{
		lock_guard<mutex> lock(mtx);
		client_count++;
		current_client_number = client_count;
	}


	char buffer[1024];

	// flag
	bool authenticated = false;
	string username, password;
	while (!authenticated) {
		// recv username length
		int username_length;
		int bytes_received = recv(client_socket, &username_length, sizeof(username_length), 0);
		if (bytes_received <= 0) {
			close(client_socket);
			return;
		}

		//debug
		cout << "Received username length: " << username_length << endl;


		// recv username
		bytes_received = recv(client_socket, buffer, username_length, 0);
		if (bytes_received <= 0) {
			close(client_socket);
			return;
		}

		buffer[bytes_received] = '\0';
		username = string(buffer, username_length);
		// debug
		cout << "Read username: " << username << ", Read username length: " << bytes_received << :endl;

		// recv password length
		int password_length;
		bytes_received = recv(client_socket, &password_length, sizeof(password_length), 0);
		if (bytes_received <= 0) {
			close(client_socket);
			return;
		}

		// debug recv password length
		cout << "Received password length: " << password_length << endl;

		// recv password
		bytes_received = recv(client_socket, buffer, password_length, 0);
		if (bytes_received <= 0) {
			close(client_socket);
			return;
		}

		buffer[bytes_received] = '\0'; // null terminate
		password = string(buffer, password_length);
		// debug
		cout << "Read password: " << password << ", Read password length: " << bytes_received << endl;

		// check creds
		authenticated = authenticate(username, password);
		// debug
		cout << "Entered username: " << username << ", Entered password: " << password << endl;

		// send auth to client
		const char* auth_result = authenticated ? "OK" : "ERROR";
		send(client_socket, auth_result, strlen(auth_result), 0);

		if (!authenticated) {
			close(client_socket);
			return;
		}
	}

	//while loop to recv msg, send msg to other clients
	while (true) {
		memset(buffer, 0, sizeof(buffer));
		int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
		if (bytes_received <= 0) {
			break;
		}

		// print unencrypted msg on server
		cout << "Client " << client_socket << ": " << buffer << endl;

		lock_guard<mutex> lock(mtx);
		for (int i = 0; i < MAX_CLIENTS; i++) {
			if (client_sockets[i] != -1 && client_sockets[i] != client_socket) {
				send(client_sockets[i], buffer, bytes_received, 0);
			}
		}

		// encrypt recv msg before writing to chat log
		string encrypted_message = caesar_encrypt(buffer, 3); // Using Caesar cipher with a shift of 3
		chat_log << "Client " << client_socket << ": " << encrypted_message << endl;
	}

	close(client_socket);
	remove_client(client_socket);

	lock_guard<mutex> lock(mtx);
	for (int i = 0; i < MAX_CLIENTS; i++) {
		if (client_sockets[i] == client_socket) {
			client_sockets[i] = -1;
			break;
		}
	}
}









int main() {
	// create socket
	int server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket < 0) {
		cerr << "Error creating socket" << endl;
		return 1;
	}

	// bind socket to port
	sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
		cerr << "Error binding socket" << endl;
		return 1;
	}

	// listen for incoming con.
	if (listen(server_socket, 5) < 0) {
		cerr << "Error listening on socket" << endl;
		return 1;
	}

	cout << "Server started on port " << PORT << endl;

	// init. client_sockets array
	memset(client_sockets, -1, sizeof(client_sockets));

	// open chat log
	ofstream chat_log("chat_log.txt");
	if (!chat_log) {
		cerr << "Error opening chat log file" << std::endl;
		return 1;
	}

	// accept incoming con. handle them in separate threads
	while (true) {
		sockaddr_in client_addr;
		socklen_t client_addr_size = sizeof(client_addr);
		int client_socket = accept(server_socket, (sockaddr*)&client_addr, &client_addr_size);
		if (client_socket < 0) {
			cerr << "Error accepting connection" << endl;
			continue;
		}

		lock_guard<mutex> lock(mtx);
		for (int i = 0; i < MAX_CLIENTS; i++) {
			if (client_sockets[i] == -1) {
				client_sockets[i] = client_socket;
				break;
			}
		}

		thread t(handle_client, client_socket, ref(chat_log));
		t.detach();
	}

	// close chat log file and server socket
	chat_log.close();
	close(server_socket);
	return 0;
}









/* REFERENCES
* https://en.cppreference.com/w/cpp/thread/mutex
* https://cplusplus.com/reference/thread/thread/
* https://www.geeksforgeeks.org/socket-programming-in-cpp/
* https://github.com/cjchirag7/chatroom-cpp
* https://github.com/nnnyt/chat
* https://yusufsefasezer.github.io/ysSocketChat/
* https://simpledevcode.wordpress.com/2016/06/16/client-server-chat-in-c-using-sockets/
* https://tldp.org/LDP/LG/issue74/tougher.html
*/