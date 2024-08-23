#include <iostream>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

const int PORT = 8080;

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024] = { 0 };
    std::string message;

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    // Connect to the server
    connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    while (true) {
        std::cout << "Enter message: ";
        std::getline(std::cin, message);

        // Send message to server
        send(sock, message.c_str(), message.length(), 0);

        // Receive echo message from server
        int valread = read(sock, buffer, 1024);
        std::cout << "Server: " << buffer << std::endl;
    }

    close(sock);

    return 0;
}
