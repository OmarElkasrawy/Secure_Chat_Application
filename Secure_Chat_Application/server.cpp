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