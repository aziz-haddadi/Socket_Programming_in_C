#include "ex_tcp.h"

int main() {
    int sock_fd, client_sock;
    struct sockaddr_in srv_addr;
    int opt = 1;
    int addr_len = sizeof(srv_addr);
    char msg[1024] = {0};
    time_t now;
    struct tm *time_data;

    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    srv_addr.sin_family = AF_INET;
    srv_addr.sin_addr.s_addr = INADDR_ANY;
    srv_addr.sin_port = htons(PORT);

    if (bind(sock_fd, (struct sockaddr *)&srv_addr, sizeof(srv_addr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(sock_fd, MAX_CLIENTS) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Serveur en attente de connexions...\n");

    while (1) {
        if ((client_sock = accept(sock_fd, (struct sockaddr *)&srv_addr, (socklen_t*)&addr_len)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        printf("Client connecté : %s:%d\n", inet_ntoa(srv_addr.sin_addr), ntohs(srv_addr.sin_port));
        for (int i = 0; i < 60; i++) {
            time(&now);
            time_data = localtime(&now);
            strftime(msg, sizeof(msg), "Il est %H:%M:%S !\n", time_data);
            send(client_sock, msg, strlen(msg), 0);
            sleep(1);
        }
        strcpy(msg, "Bye Bye !\n");
        send(client_sock, msg, strlen(msg), 0);
        close(client_sock);
    }

    return 0;
}