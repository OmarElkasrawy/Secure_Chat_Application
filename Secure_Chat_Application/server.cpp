#include <iostream>
#include <thread>
#include <mutex>
#include <cctype>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
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
		cerr << "Error opening creds file" << endl;
		return false;
	}

	string stored_username, stored_password;
	while (credentials_file >> stored_username >> stored_password) {
		// debug
		cout << "Read username: " << stored_username << ", Read password: " << stored_password << endl;
		if (stored_username == username && stored_password == password) {
			// debug
			cout << "Entered username: " << username << ", Entered password: " << password << endl;
			cout << "Authenticated!" << endl;
			return true;
		}
	}

	// entered and cond. invalid 
	cout << "Entered username: " << username << ", Entered password: " << password << endl;
	cout << "Invalid creds!";
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
	while (!authenticate) {
		// recv user length
		int username_length;
		int bytes_received = recv(client_socket, &username_length, size(username_length), 0);
		if (bytes_received <= 0) {
			close(client_socket);
			return;
		}


		// recv username
		bytes_received = recv(client_socket, buffer, username_length, 0);
		if (bytes_received <= 0) {
			close(client_socket);
			return;
		}

		buffer[bytes_received] = '\0';
		username = string(buffer, username_length);

		// recv password length
		int password_length;
		bytes_received = recv(client_socket, &password_length, sizeof(password_length), 0);
		if (bytes_received <= 0) {
			close(client_socket);
			return;
		}

		// recv password
		bytes_received = recv(client_socket, buffer, password_length, 0);
		if (bytes_received <= 0) {
			close(client_socket);
			return;
		}

		buffer[bytes_received] = '\0'; //null terminate
		password = string(buffer, password_length);

		// check creds
		authenticated = authenticate(username, password);

		// send auth to client
		const char* auth_result = authenticated ? "OK" : "ERROR";
		send(client_socket, auth_result, strlen(auth_result), 0);

		if (!authenticated) {
			close(client_socket);
			return;
		}
	}

	//while loop to recv msg, send msg to other clients
}











int main() {
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
* 
*/