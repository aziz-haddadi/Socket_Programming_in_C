#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT 5000
#define MAX_CLIENTS 5
#define BUFFER_SIZE 64

void handle_client(int client_fd) {
    char buffer[BUFFER_SIZE];
    time_t now;
    struct tm *tm_info;

    for (int i = 0; i < 60; i++) {
        now = time(NULL);
        tm_info = localtime(&now);
        strftime(buffer, sizeof(buffer), "Il est %H:%M:%S !\n", tm_info);
        send(client_fd, buffer, strlen(buffer), 0);
        sleep(1);
    }

    close(client_fd);
    exit(0);
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    signal(SIGCHLD, SIG_IGN); // avoid zombie processes

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); exit(EXIT_FAILURE); }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind"); close(server_fd); exit(EXIT_FAILURE);
    }

    listen(server_fd, MAX_CLIENTS);
    printf("Concurrent TCP server listening on port %d...\n", PORT);

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
        if (client_fd < 0) {
            perror("accept"); continue;
        }

        if (fork() == 0) {
            // Child process
            close(server_fd);
            handle_client(client_fd);
        } else {
            // Parent process
            close(client_fd);
        }
    }

    close(server_fd);
    return 0;
}
