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

		// null terminate
		buffer[bytes_received] = '\0';
		cout << "Received: " << buffer << endl;
	}
}
















int main() {

	return 0;
}