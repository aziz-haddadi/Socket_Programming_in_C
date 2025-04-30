#include "ex_tcp.h"

int main() {
    int sockfd = 0;
    struct sockaddr_in server;
    char buf[BUFFER_SIZE] = {0};

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    if (inet_pton(AF_INET, ADDR, &server.sin_addr) <= 0) {
        perror("Invalid address");
        exit(EXIT_FAILURE);
    }

    if (connect(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    int msg_count = 0;
    int bytes_read;
    while ((bytes_read = read(sockfd, buf, BUFFER_SIZE - 1)) > 0) {
        buf[bytes_read] = '\0';
        printf("%s", buf);
        msg_count++;
        memset(buf, 0, BUFFER_SIZE);
    }

    printf("Nombre de messages reçus : %d\n", msg_count);
    close(sockfd);
    return 0;
}