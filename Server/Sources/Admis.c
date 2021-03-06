#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/wait.h>

#define TAILLE_TYPE 50 // Taille du char* type mime = 50
char **recupereListeTypeMime(int *nbType)
{
     *nbType = 0;                             // on considère pour le moment qu'il n'y a pas plus de 3 types mime dans le fichier txt
     char **typeSSSMime = (char **)malloc(0); // Tableau de types mime
     char typeMime[TAILLE_TYPE];
     /* Récupération des types mime (ici, typeMime) dans un tableau (ici, typeSSSMime) (en haut niveau) 
     (pour le bas niveau, voir la partie commentée ci-dessous) */

     /*
     Le fichier MimeTypes.txt ici est dans le même répertoire que le fichier Admis.c 
     */

     FILE *types = NULL;
     types = fopen("./MimeTypes.txt", "r");

     if (types != NULL)
     {

          while (!feof(types))
          {
               typeMime[0] = '\0';
               fgets(typeMime, TAILLE_TYPE, types);

               if (typeMime[strlen(typeMime) - 1] == '\n')
               {
                    typeMime[strlen(typeMime) - 1] = '\0';
               }

               typeSSSMime = (char **)realloc(typeSSSMime, sizeof(char *) * (*nbType + 1));
               typeSSSMime[*nbType] = malloc(sizeof(char) * strlen(typeMime));
               strcpy(typeSSSMime[*nbType], typeMime);
               *nbType += 1;
          }
          fclose(types);
     }
     else
     {
          printf("Lecture du fichier des Types Mime impossible.\n");
     }
     return typeSSSMime;
}
int admissible(char *nomImage)
{
     int nbType = 0;
     char **listeTypeMime = recupereListeTypeMime(&nbType);
     int bool = 0;
     char cheminImageTmp[306];
     char cheminImageFinal[306];
     sprintf(cheminImageTmp, "./tmp/%s", nomImage);
     sprintf(cheminImageFinal, "./FilesServeur/%s", nomImage);

     int f[2];
     pipe(f);
     printf("debut du test d'admissibilité\n");
     switch (fork())
     {
     case -1:
          fprintf(stderr, "Erreur de fork\n");
          exit(-1);
     case 0:
          //comportement du fils // Vérification d'admissibilité du fichier
          close(1); //redirection de la sortie standard
          dup(f[1]);
          close(f[0]); //fermeture du descripteur inutilisé
          close(f[1]);
          // commande à être exécutée par le processus fils
          // pour récupérer le type mime du fichier
          execlp("file", "file", "--mime-type", cheminImageTmp, (char *)0);
          //la sortie de "file" (a.k.a. le type mime du fichier) ira dans le pipe
     default:
          switch (fork())
          {
          case -1:
               exit(-1);
          case 0:

               //comportement du père
               close(f[1]); //fermeture du descripteur inutilisé

               char typeFichierRecup[100];
               read(f[0], typeFichierRecup, sizeof(typeFichierRecup));

               char *recup = strrchr(typeFichierRecup, ' '); // Récupération du type en récupérant tout se qui est situé après la dernière occurence du caractère 'espace'

               char *delim = strtok(typeFichierRecup, "\n");
               char *tablOutput[2];
               int i = 0;
               while (delim != NULL)
               {
                    tablOutput[i++] = delim;
                    delim = strtok(NULL, " ");
               }

               printf("recup : %s\n", recup);
               printf("tableoutput : %s\n", tablOutput[0]);
               int l;

               for (l = 1; l < strlen(recup); l++)
               {
                    recup[l - 1] = recup[l]; // Suppression de l'espace en début de la chaîne de caractères
               }
               recup[l - 1] = '\0';
               printf("type lu : %s\n", recup);

               int j = 0;
               while (j < nbType && bool == 0)
               {
                    /* vérification de l'existance de ce type dans le tableau parcouru 
                    des types mime admissibles */
                    if (strcmp(recup, listeTypeMime[j]) == 0)
                    {
                         printf("type admissible\n bool = %d\n", bool);
                         bool = 1;
                    }
                    j++;
               }
               if (bool) {
                    printf("Fichier déplacé dans FilesServeur\n");
                    rename(cheminImageTmp, cheminImageFinal);
               } else {
                    printf("pas le bon type\n bool = %d\n", bool);
                    if (unlink(cheminImageTmp) == 0) {
                         printf("Fichier supprimer dans tmp\n");
                    }
               }
               exit(0);
          default:
               while (wait(NULL) != -1)
                    ;
          }
     }
     if (listeTypeMime != NULL)
     {
          free(listeTypeMime);
     }
}
