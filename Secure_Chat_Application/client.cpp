#include <iostream>
#include <string>
#include <thread>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

using namespace std;

// port and address
const int PORT = 10101;
const string SERVER_ADDRESS = "127.0.0.1"; // localhost

// function recv message from server.cpp
void receive_messages(int sock) { // sock because socket cant be used
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
	// create socket
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		std::cerr << "Error creating socket" << std::endl;
		return 1;
	}

	// server address setup
	sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	inet_pton(AF_INET, SERVER_ADDRESS.c_str(), &server_addr.sin_addr);

	// connection
	if (connect(sock, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
		std::cerr << "Error connecting to server" << std::endl;
		return 1;
	}

	// auth process
	string username, password;
	//flag
	bool authenticated = false;
	while (!authenticated) {
		// request user and pass
		cout << "Enter username: ";
		getline(cin, username);

		cout << "Enter password: ";
		getline(cin, password);

		// clear input buffer test cin.ignore(numeric_limits<streamsize>::max(), '\n');

		// send user and pass lengths to server
		int username_length = username.size();
		send(sock, &username_length, sizeof(username_length), 0);
		// debug
		cout << "Sent username length: " << username_length << endl;

		send(sock, username.c_str(), username_length, 0);

		int password_length = password.size();
		send(sock, &password_length, sizeof(password_length), 0);
		
		send(sock, password.c_str(), password.size(), 0);

		// recv auth result from server
		char auth_result[1024];
		recv(sock, auth_result, sizeof(auth_result), 0);
		auth_result[sizeof(auth_result) - 1] = '\0'; // null terminate
		if (string(auth_result) == "OK") {
			authenticated = true;
			cout << "Authentication successful!" << endl;
		}
		else {
			cout << "Authentication failed!" << endl;
		}
	}



	//start thread to recv messages from server
	thread t(receive_messages, sock);
	t.detach();

	// loop to send msgs to server and recv responses
	string input;
	while (getline(cin, input)) {
		send(sock, input.c_str(), input.size(), 0);
	}

	//close socket
	close(sock);
	return 0;
}