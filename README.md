# Socket_Programming_in_C
4. Transfert de données en mode non connecté

Maintenant , on travaillerai sur le protocole UDP au lieu de TCP . On a implémenté une version de serveur et client en UDP dans udpServer.c et udpClient.c et on a fait la capture des packets dans udpsniff_monoserver.pcapng On sait que la UDP est un protocole non orienté-connexion d'où chaque envoie de message contenait le destinataire

Dans la capture , on peut voir l'absence de l'établissement et la fermeture de la connexion . On ne voit que 61 packets . Chaque packet contient le message envoyé par le serveur au client . Mais le premier packet était envoyé par le client au serveur et il contient le message START . C'est comment la communication commence en UDP    

5. Serveur en mode concurrent

On modifie le code du serveur pour obtenir une sorte de serveur en mode concurrent , càd , un serveur qui fait fork() pour les différents processus (des exécutions) du code client et les manipulent simultanément . On observe bien que chaque fois un client entre en communication avec le serveur via le 3-way handshake , le serveur le met en une sorte de file d'attente et à chaque fois envoie un packet de données à l'in d'eux un par un . Quand le serveur finit les 60 messages avec un certain client , il le remouve de la file d'attente et passe au next

Si on ajoute plus de services aux serveurs (on a choisit d'ajouter la fonction d'afficher le nombre de processus actifs et les infos du système) , on remarque que à chaque fois l'un des clients établit la connexion puis il attend que le client lui envoie en PSH+ACK le type de service demandé pour qu'il lui répond directement avec l'information appropriée et si il s'agit d'une demande d'affichage des 60 messages il le met dans la file de replys comme le cas de monoservice
