#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

/**
 * @brief Traitement effectué lors de la réception du signal SIGCHLD
 * 
 * @param s //Signal
 */
void handler(int s)
{
     wait(NULL);
}

/**
 * @brief Fonction main du programme
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, char const *argv[])
{
     //Définition du comportement pour le traitement du signal SIGCHLD
     struct sigaction ac;
     ac.sa_handler = handler;
     ac.sa_flags = SA_RESTART;

     //Création d'une socket d'écoute
     int socketEcoute = socket(AF_INET, SOCK_STREAM, 0);
     if (socketEcoute == -1)
     {
          perror("socket()");
          exit(-1);
     }
     else
     {
          printf("Tout va bien création socket\n");
     }

     //Attachement de la socket à un port
     struct sockaddr_in socketClient;

     socketClient.sin_family = AF_INET; //Pointeur structure : (->) pour accès aux champs
     socketClient.sin_port = htons(6067);
     socketClient.sin_addr.s_addr = htonl(INADDR_ANY);

     if (bind(socketEcoute, (struct sockaddr *)&socketClient, sizeof(socketClient)) == -1)
     {
          perror("bind()");
          exit(-1);
     }
     else
     {
          printf("Tout va bien attachement socket\n");
     }

     //Ouverture du service
     //Maximum 5 connexions en attente
     if (listen(socketEcoute, 5) == -1)
     {
          perror("listen()");
          exit(-1);
     }
     else
     {
          printf("Tout va bien ouverture\n");
     }

     //Acceptation de la connection
     int len = sizeof(socketClient);
     while (1)
     {
          int socketService;
          if ((socketService = accept(socketEcoute, (struct sockaddr *)&socketClient, &len)) == -1)
          { /* /!\ L'appel de la fonction accept est bloquant /!\ */
               perror("accept()");
               exit(-1);
          }
          else
          {
               printf("Accepte connection ok\n"); // Ici on a eu aucun affichage car on avait pas mit de \n
                                                  // penser à en mettre pour flush le buffer et afficher sur la sortie standard
          }

          int tampon = -1;
          switch (fork())
          {
          case -1:
               //Erreur
               perror("fork()");
               exit(-1);
               break;

          case 0:
               //Comportement du fils

               close(socketEcoute); //Fermeture du descripteur de fichier

               signal(SIGCHLD, SIG_DFL); //Redéfinition (à défaut) du comportement du signal SIGCHLD pour ne pas hériter de celui du père = réinitialisation

               read(socketService, &tampon, sizeof(int));
               printf("Valeur lue : %d\n", tampon);

               tampon += 3;
               write(socketService, &tampon, sizeof(int));

               int nbFichiersALire = 0;
               while (read(socketService, &nbFichiersALire, sizeof(int)) == -1);
               //printf("Nb de fichiers à lire : %d\n", nbFichiersALire); ok

               int i = 0;
               int tailleTampon;
               char** tamponString = malloc(sizeof(char*)*5);
               while (i < nbFichiersALire)
               {
                    while (read(socketService, &tailleTampon, sizeof(int)) == -1);
                    tamponString[i] = malloc(sizeof(char) * tailleTampon);
                    while (read(socketService, tamponString[i], sizeof(char) * tailleTampon) == -1);
                    printf("Valeur lue : %s\n", tamponString[i]);
                    i++;
               }
               printf("Enregistrement terminé\n");
               exit(0);
               break;

          default:
               //Comportement du père
               break;
          }
     }

     return 0;
}
