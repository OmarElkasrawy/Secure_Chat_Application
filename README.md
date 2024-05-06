# Secure_Chat_Application

This repository contains a Secure Chat Application program for KH5062CEM Programming & Algorithms 2 module coursework #2 implemented in C++ Sublime Text Kali Linux that communicates over a network to enable real-time messaging with encrypted transmission. 

## Table of Contents
- [Introduction] (#introduction)
- [Features] (#features)
- [Usage] (#usage)
- [Installation] (#installation)
- [License] (#license)

## Introduction

Secure communication is crucial, especially in digital environments where privacy and confidentiality are paramount. This project provides a secure chat application where users can exchange messages securely over a network. The communication between the server and clients is encrypted using Caesar cipher encryption.

## Features

1. User Authentication: Users can authenticate themselves using a username and password.
2. Real-time Messaging: Users can exchange messages in real time.
3. Encryption: Messages are encrypted using Caesar cipher encryption for secure transmission.
4. Multithreading: The server handles multiple client connections concurrently using multithreading.
5. Error Handling: The application includes error handling mechanisms to ensure smooth operation.

## Usage

- Clone this repository to your local machine (must use Kali Linux)
- Compile both server and client files
- Start the server by running the compiled 'server' executable
- Start one or more clients by running the compiled 'client' executable
- Upon launching the client, you will be prompted to enter your username and password for authentication (use the credentials in the credentials.txt
- Once authenticated, you can start sending and receiving messages in the chat

## Installation

To install and run this application, you can follow these steps:

- Clone the repository:
```bash
git clone https://github.com/OmarElkasrawy/Secure_Chat_Application.git
```
- Compile the server and client programs:
```bash
g++ server.cpp -o server
g++ client.cpp -o client
```

- NOTE: if compiling client does not work and shoots 'undefined reference to' compile the client.cpp as follows:
```bash
g++ -std=c++11 -pthread client.cpp -o client -lstdc++
```
- Start the server:
```bash
./server
```
- Start the client:
```bash
./client
```
- Follow the prompts to authenticate using the credentials in 'credentials.txt' and start chatting securely. (You can simply add more users)

## License
This project is licensed under the MIT License.

# Creator
Omar Mostafa Ali
<br>
202101689
