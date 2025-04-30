#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 5000
#define MAX_CLIENTS 5

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char buffer[64];
    time_t now;
    struct tm *tm_info;

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // Accept any IP
    server_addr.sin_port = htons(PORT);

    // Bind
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Listen
    listen(server_fd, MAX_CLIENTS);
    printf("Server listening on port %d...\n", PORT);

    // Accept one client
    client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
    if (client_fd < 0) {
        perror("accept failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Client connected.\n");

    // Send time 60 times
    for (int i = 0; i < 60; i++) {
        now = time(NULL);
        tm_info = localtime(&now);
        strftime(buffer, sizeof(buffer), "Il est %H:%M:%S !\n", tm_info);
        send(client_fd, buffer, strlen(buffer), 0);
        sleep(1);
    }

    close(client_fd);
    close(server_fd);
    printf("Connection closed.\n");
    return 0;
}
