# Socket_Programming_in_C
## 2. Développement d’un client HTTP
### 2.1. Vérification de la présence du serveur HTTP
On veut envoyer une requête GET sur le port 80 de la machine 10.250.101.1. Cependant, le serveur semble être en panne. On essaiera donc avec la machine 18.205.89.57, qui correspond au service *httpbin.org*.
-	On obtient l'adresse ip: `nslookup httpbin.org`
-	On envoie une requête GET: `curl -X GET -i 18.205.89.57:80`

Réponse: La réponse http a le code **200 OK** suivi des en‑têtes et du contenu HTML de la page d’accueil du serveur.
```http
HTTP/1.1 200 OK
Date: Thu, 20 Apr 2025 23:10:45 GMT
Content-Type: text/html; charset=utf-8
Content-Length: 9593
Connection: keep-alive
Server: gunicorn/19.9.0
Access-Control-Allow-Origin: *
Access-Control-Allow-Credentials: true

<!DOCTYPE html>
...
```
-	On envoie une requête HEAD: `curl -X HEAD -i 18.205.89.57:80`

Réponse: La réponse http a le code **200 OK** suivi des en‑têtes seulement, sans contenu.
```http
HTTP/1.1 200 OK
Date: Thu, 20 Apr 2025 23:17:05 GMT
Content-Type: text/html; charset=utf-8
Content-Length: 9593
Connection: keep-alive
Server: gunicorn/19.9.0
Access-Control-Allow-Origin: *
Access-Control-Allow-Credentials: true
```
### 2.2. Client HTTP en mode connecté
On écrit un programme en C d'un client em mode connecté qui lit une requête HTTP saisie par l'utilisateur et renvoie la réponse:
```C
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#define PORT 80
#define BUFFER_SIZE 4096
#define ADDR "18.205.89.57" //httpbin

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

    printf("Entrez votre requête HTTP (terminez par deux retours à la ligne) :\n");
    memset(req, 0, BUFFER_SIZE);
    size_t len = 0;
    char line[BUFFER_SIZE];
    
    while (fgets(line, BUFFER_SIZE, stdin) != NULL) {
        strcat(req, line);
        if (strlen(line) == 1 && line[0] == '\n') {
            if (len > 0 && req[len-1] == '\n') {
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
```
**Compilation**:
```bash
gcc connecte.c -o connecte
```
**Observation avec Wireshark** (ficher *pcapng* attaché):
-	Établissement TCP en trois-way handshake (SYN, SYN-ACK, ACK).
-	Envoi de la requête HTTP une fois la connexion établie.
-	Le **serveur** renvoie la réponse, puis ferme la connexion (FIN).
**Chronogramme (TCP/HTTP) :**
```
Client                    Serveur
  |--- SYN ---------->|
  |<-- SYN+ACK -------|
  |--- ACK ---------->|
  |--- GET / HTTP/1.1 -->|
  |                    |--- 200 OK + contenu
  |<-- 200 OK --------|
  |--- ACK ---------->|
  |                    |--- FIN
  |<-- FIN -----------|
  |--- ACK ---------->|
```
**Ports utilisés :** client: un port éphémère (>1024), serveur: 80.
**Fermeture :** Le serveur initie la fermeture (envoie FIN), le client ACK, puis renvoie un FIN.

---

**Question supplémentaire :**

Refaites la même capture en remplaçant votre client HTTP par celui de la question 2.1. Que constatez-vous ? Expliquez !

**Réponse :**
Le client HTTP de la question 2.1 (telnet) envoie la requête de manière interactive, ce qui se traduit par plusieurs paquets TCP distincts, souvent un paquet par ligne tapée (grâce à l'entrée utilisateur ligne par ligne). En revanche, le client de la question 2.2 envoie toute la requête HTTP d’un seul coup via `send()`, ce qui résulte généralement en un seul segment TCP contenant toute la requête (si elle est petite).

Avec Wireshark, on observe donc :
- Telnet : plusieurs petits paquets TCP contenant chacun une ligne de la requête.
- Client HTTP en C : un seul paquet TCP contenant l'intégralité de la requête HTTP.

## 3. Transfert de messages en mode connecté
### 3.1. Programme Client (station 1)
```c
//clienttcp.c
#include "ex_tcp.h" //des "includes"

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
```
### 3.2. Programme Serveur (station 2)
```c
//serveurtcp.c
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
```

**Compilation :**
```bash
gcc tcp_server.c -o tcp_server
gcc tcp_client.c -o tcp_client
```

#### Questions et Observations:
- **Comptage des messages échangés :**
  - Sans `sleep(1)`, le client reçoit 60 messages très rapidement (~quelques ms), aucun perdu.
  - Avec `sleep(1)` côté serveur, le rythme est ~1 message/s, toujours 60 messages reçus.
- **Segments TCP vs écritures** :
  - Wireshark montre que le kernel agrège ou segmente selon MTU, pas un segment = un `send()` forcément.
- **Débranchement réseau :**
  - Si câble débranché, `recv()` bloque ou renvoie 0/erreur après timeout selon options.
  - Débrancher/rebrancher rapide : la connexion TCP peut se rétablir ou timeout si trop long.
- **File des connexions pendantes :**
  - Tant qu’on n’accepte qu’un client, les autres se mettent en attente (QUEUE).
  - Une fois pleine, `connect()` côté client retourne `ECONNREFUSED`.
  - 
## 4. Transfert de données en mode non connecté

Maintenant , on travaillerai sur le protocole UDP au lieu de TCP . On a implémenté une version de serveur et client en UDP dans udpServer.c et udpClient.c et on a fait la capture des packets dans udpsniff_monoserver.pcapng On sait que la UDP est un protocole non orienté-connexion d'où chaque envoie de message contenait le destinataire

Dans la capture , on peut voir l'absence de l'établissement et la fermeture de la connexion . On ne voit que 61 packets . Chaque packet contient le message envoyé par le serveur au client . Mais le premier packet était envoyé par le client au serveur et il contient le message START . C'est comment la communication commence en UDP    

## 5. Serveur en mode concurrent

On modifie le code du serveur pour obtenir une sorte de serveur en mode concurrent , càd , un serveur qui fait fork() pour les différents processus (des exécutions) du code client et les manipulent simultanément . On observe bien que chaque fois un client entre en communication avec le serveur via le 3-way handshake , le serveur le met en une sorte de file d'attente et à chaque fois envoie un packet de données à l'in d'eux un par un . Quand le serveur finit les 60 messages avec un certain client , il le remouve de la file d'attente et passe au next

Si on ajoute plus de services aux serveurs (on a choisit d'ajouter la fonction d'afficher le nombre de processus actifs et les infos du système) , on remarque que à chaque fois l'un des clients établit la connexion puis il attend que le client lui envoie en PSH+ACK le type de service demandé pour qu'il lui répond directement avec l'information appropriée et si il s'agit d'une demande d'affichage des 60 messages il le met dans la file de replys comme le cas de monoservice
