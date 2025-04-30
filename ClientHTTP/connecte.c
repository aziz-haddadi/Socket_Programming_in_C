#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 80
#define BUFFER_SIZE 4096
#define ADDR "18.205.89.57" // httpbin

int main() {
  int sock = 0;
  struct sockaddr_in serv_addr;
  char buff[BUFFER_SIZE] = {0};
  char req[BUFFER_SIZE];

  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Socket creation error");
    exit(EXIT_FAILURE);
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);

  if (inet_pton(AF_INET, ADDR, &serv_addr.sin_addr) <= 0) {
    perror("Invalid address");
    exit(EXIT_FAILURE);
  }

  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    perror("Connection failed");
    exit(EXIT_FAILURE);
  }

  printf(
      "Entrez votre requête HTTP (terminez par deux retours à la ligne) :\n");
  memset(req, 0, BUFFER_SIZE);
  size_t len = 0;
  char line[BUFFER_SIZE];

  while (fgets(line, BUFFER_SIZE, stdin) != NULL) {
    strcat(req, line);
    if (strlen(line) == 1 && line[0] == '\n') {
      if (len > 0 && req[len - 1] == '\n') {
        break;
      }
    }
    len = strlen(req);
  }
  printf("Requête lue :\n%s\n", req);

  send(sock, req, strlen(req), 0);
  printf("Requête envoyée.\n");

  int bytes_read;
  printf("Réponse du serveur :\n");
  while ((bytes_read = read(sock, buff, BUFFER_SIZE - 1)) > 0) {
    buff[bytes_read] = '\0';
    printf("%s", buff);
    fflush(stdout);
    memset(buff, 0, BUFFER_SIZE);
  }

  if (bytes_read < 0) {
    perror("Erreur lors de la lecture de la réponse");
  }

  close(sock);
  return 0;
}
