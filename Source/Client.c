#ifndef CLIENT_C
#define CLIENT_C
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include "Affichage.h"
#include "Reception.h"
#endif
/**
 * @brief Demande à l'utilisateur ce qu'il souhaite effectuer et renvoie le code d'action correspondant
 * 
 * @return int 
 */
int choixAction()
{
     int choix;
     printf("***** Que voulez-vous faire ? *****\n1- Déposer des fichiers \n2- Récupérer des fichiers\n3- Quitter\n");
     scanf("%d", &choix);
     if (choix != 1 && choix != 2 && choix != 3)
     {
          choix = 3;
     }
     return choix;
}

/**
 * @brief Coeur du programme CLIENT il crée la socket permettant de discuter avec le serveur
 * puis interagi avec l'utilisateur afin de satisfaire ses demandes
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, char const *argv[])
{

     //Création d'une socket communication client
     int socketCommClient = socket(AF_INET, SOCK_STREAM, 0);
     if (socketCommClient == -1)
     {
          // fprintf(stderr, "Erreur dans la creation de la socket");
          perror("socket()");
          exit(-1);
     }
     else
     {
          printf("Tout va bien création socket\n");
     }

     //Récupère les informations du serveur grâce au nom de la machine
     struct hostent *infoServeur = gethostbyname("localhost");
     struct sockaddr_in socketServeur;

     socketServeur.sin_family = AF_INET;
     socketServeur.sin_port = htons(6067);
     memcpy(&socketServeur.sin_addr.s_addr, infoServeur->h_addr_list[0], infoServeur->h_length); //Affectation à l'aide de memcpy

     printf("Adresse serveur : %s\n", inet_ntoa(socketServeur.sin_addr));

     //Connection du client au serveur
     if (connect(socketCommClient, (struct sockaddr *)&socketServeur, sizeof(socketServeur)) == -1)
     {
          perror("connect()");
     }
     else
     {
          printf("Connection établie\n");
     }

     int affichage;
     while ((affichage = choixAction()) != 3)     //Tant que l'utilisateur ne souhaite pas arrêter
     {
          clear();
          switch (affichage)
          {
          case 1:
               envoieFichier(socketCommClient);
               break;
          case 2:
               receptionFichier();
               break;
          }
          clear();
     }
     printf("Fin du programme\n");
     return 0;
}
