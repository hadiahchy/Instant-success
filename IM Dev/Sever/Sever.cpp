#include <iostream>
#include <thread>
#include <vector>
#include <netinet/in.h>
#include <unistd.h>

const int PORT = 8080;

void handleClient(int clientSocket) {
    char buffer[1024] = { 0 };
    while (true) {
        int valread = read(clientSocket, buffer, 1024);
        if (valread > 0) {
            std::cout << "Message received: " << buffer << std::endl;
            send(clientSocket, buffer, strlen(buffer), 0);  // Echo back the message
        }
    }
    close(clientSocket);
}

int main() {
    int server_fd, clientSocket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Create socket file descriptor
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    // Forcefully attach socket to the port
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket to the network address and port
    bind(server_fd, (struct sockaddr*)&address, sizeof(address));

    // Listen for incoming connections
    listen(server_fd, 3);

    std::vector<std::thread> threads;

    // Accept incoming connections and handle clients in separate threads
    while (true) {
        clientSocket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        std::cout << "New connection established." << std::endl;
        threads.push_back(std::thread(handleClient, clientSocket));
    }

    for (auto& th : threads) {
        th.join();
    }

    return 0;
}
