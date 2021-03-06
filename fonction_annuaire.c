#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<unistd.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<errno.h>
#include <dirent.h>
#include"fonction_annuaire.h"

#define BUFFERSIZE 2048

int info_fichier(char * message, char * nom_fichier){
  
  printf("\n---Lecture dans %s---\n", nom_fichier);
  
  char buffer[BUFFERSIZE];
  char *pointeur = NULL;
  char chemin[40], mess[100];
  bzero(chemin, 40);
  bzero(mess, 200);
  sprintf(chemin, "./Fichier/%s.txt", nom_fichier);
  int file=open(chemin, O_RDONLY);

  if(file==-1){
    perror("Erreur ouverture du fichier ");
    return EXIT_FAILURE;
  }

  read(file, buffer, BUFFERSIZE);
  pointeur=strstr(buffer, "-");

  pointeur+=1;
  while(*pointeur != '-'){
      while (*pointeur != '\n' && *pointeur != '-'){
        pointeur++;
      }
      pointeur++;
      if (*pointeur != ' '){
        while(*pointeur != ' ' && *pointeur != '-'){
          sprintf(message,"%s%c", message, *pointeur);
          pointeur++;
        }
        sprintf(message,"%s\n", message);
      }
    }
  sprintf(mess,"%s", message);
  //printf("message : %s\n", mess);
  sprintf(message,"Personnes présentes dans le groupe %s :\n%s", nom_fichier, mess);
  close(file);
  return EXIT_SUCCESS;

}

int info(char * message){
  char groupe[100], mess[200];
  bzero(groupe, 100);
  bzero(mess, 200);
  system("ls ./Fichier | grep ^[^a][^n][^n][^u][^a][^i][^r][^e] > resultat.txt");

  //system("ls ./Fichier > resultat.txt");
  int file2=open("resultat.txt", O_RDONLY);

  if(file2==-1){
    perror("Erreur ouverture du fichier ");
    return EXIT_FAILURE;
  }
  read(file2, groupe, BUFFERSIZE);
  close(file2);
  system("rm resultat.txt");

  printf("\n---Lecture de l'annuaire et des groupes---\n");

  char buffer[BUFFERSIZE];
  char *pointeur = NULL;
  int file=open("./Fichier/annuaire.txt", O_RDONLY);

  if(file==-1){
    perror("Erreur ouverture du fichier ");
    return EXIT_FAILURE;
  }

  read(file, buffer, BUFFERSIZE);
  pointeur=strstr(buffer, "-");
  pointeur+=2;
  while (*pointeur != '\n' && *pointeur != '-'){
        pointeur++;
  }
  while(*pointeur != '-'){
      while (*pointeur != '\n' && *pointeur != '-'){
        pointeur++;
      }
      pointeur++;
      if (*pointeur != ' '){
        while(*pointeur != ' ' && *pointeur != '-'){
          sprintf(message,"%s%c", message, *pointeur);
          pointeur++;
        }
        sprintf(message,"%s\n", message);
      }
  }
  sprintf(mess,"%s", message);
  printf("message : %s\n", mess);
  sprintf(message,"\nPersonnes présentes : \n%sGroupes présents :\n%s", mess, groupe);
  close(file);
  return EXIT_SUCCESS;
}

int lecture_nom(char * nom, char * ip, char * port){

  printf("\n---Recherche de l'extremite de %s---\n", nom);
  //printf("nom : %s\n", nom);
  int file;
  char * pointeur=NULL;
  char buffer[BUFFERSIZE];

  file=open("./Fichier/annuaire.txt", O_RDONLY);

  if(file==-1){
    perror("Erreur ouverture du fichier ");
    return EXIT_FAILURE;
  }

  while((read(file, buffer, BUFFERSIZE)>0)&&(pointeur==NULL)){
    pointeur=strstr(buffer, nom);
  }
/*
  if (pointeur == NULL){
    printf("Ce pseudo est introuvable\n");
    return EXIT_SUCCESS;
  }
*/
  if (pointeur != NULL && verif(pointeur, nom)==EXIT_SUCCESS){
    while (*pointeur != ' '){
      pointeur++;
    }
    pointeur++;
    while (*pointeur != ' '){
      sprintf(ip, "%s%c", ip, *pointeur);
      pointeur++;
    }
    pointeur++;
    while (*pointeur != '\n'){
      sprintf(port, "%s%c", port, *pointeur);
      pointeur++;
    }
  }
  else {
    printf("Ce pseudo n'est pas renseigné\n");
    return EXIT_FAILURE;
  }

  close(file);
  return EXIT_SUCCESS;
}

int Rajouter_extremite(char * nom_file, char * nom, char * ip, char * port, int end){

  printf("\n---Rajout d'une extremite dans %s---\n", nom_file);

  int file;
  char extremite[40], chemin[40];
  char * pointeur=NULL;
  char buffer[BUFFERSIZE];

  sprintf (extremite, "%s %s %s\n-", nom, ip, port);
  sprintf(chemin, "./Fichier/%s.txt", nom_file);
  file=open(chemin, O_RDWR);
  //printf("chemin %s\n", chemin);
  if(file==-1){
    perror("Erreur ouverture du fichier ");
    return EXIT_FAILURE;
  }

  if (end==0){
    char entete[40];
    strcpy(entete, "Annuaire :\n-\n");
    write(file,entete, strlen(entete));
  }

  while((read(file, buffer, BUFFERSIZE)>0)&&(pointeur==NULL)){
    pointeur=strstr(buffer, nom);
  }
  //printf("Pointeur = %c\n", *pointeur);
  if (pointeur != NULL && verif(pointeur, nom)==EXIT_SUCCESS){
    printf("Ce pseudo est déjà renseigné\n");
    close(file);
    return EXIT_FAILURE;
  }
  else {
    if (end==-1) lseek(file,end,SEEK_END);
    write(file,extremite, strlen(extremite));
    close(file);
  }
  return EXIT_SUCCESS;
}

int verif(char * lu, char * nom){

  int compteur=0;

  lu--;

  if (*lu != '\n'){
   return EXIT_FAILURE;
  }
  else{
    lu++;
    while(*lu==*nom && *lu != ' ' && *nom != ' '){
      lu++;
      nom++;
      compteur++;
    }
    if(*lu != ' ' && *nom != ' '){
      return EXIT_FAILURE;
    }
    else{
      lu=lu-compteur;
      return EXIT_SUCCESS;
    }
  }
}

int creer_fichier(char * nom, char * pseudo, char * ext_dist){

  printf("\n---Création d'un groupe---\n");
  //printf("pseudo : %s ext_dist :%s\n", pseudo, ext_dist);
  char chemin[40], entete[40], extremite[40];
  int file;
  sprintf(chemin, "./Fichier/%s.txt", nom);

  file=open(chemin, O_RDWR | O_CREAT, 0666);

  if(file==-1){
    perror("Erreur ouverture du fichier ");
    return EXIT_FAILURE;
  }

  sprintf(entete, "%s :\n-\n", nom);
  write(file,entete, strlen(entete));

  sprintf(extremite, "%s %s\n-", pseudo, ext_dist);

  write(file,extremite, strlen(extremite));
  close(file);

  return EXIT_SUCCESS;

}

int suppression(char * nom){ //Remplace la ligne ciblé par des espaces(à améliorer)

  printf("\n---Suppression ---\n");

  int file, compteur=0, compteur2=0;
  char * pointeur=NULL, * pointeur_text=NULL;
  char buffer[BUFFERSIZE], chemin[60], nom_file[50];
  struct dirent *lecture;
  DIR *rep;
  rep = opendir("./Fichier" );

  while ((lecture = readdir(rep))) {
    compteur=0;
    compteur2=0;
    if (strstr((lecture->d_name), ".txt") != NULL){
      bzero(chemin, 60);
      bzero(nom_file, 50);
      strcpy(nom_file,lecture->d_name);
      sprintf(chemin, "./Fichier/%s", nom_file);
      printf("\nSuppression dans : %s de %s\n", chemin, nom);
      file=open(chemin, O_RDWR);

      if(file==-1){
          perror("Erreur ouverture du fichier ");
          return EXIT_FAILURE;
      }
      read(file, buffer, BUFFERSIZE);
      pointeur_text=strstr(buffer, ":");
      pointeur=strstr(buffer, nom);

       while (buffer[compteur2] != ':'){
         buffer[compteur2];
         compteur2++;
       }

       if (pointeur != NULL && verif(pointeur,nom)==EXIT_SUCCESS){
         while (pointeur_text != pointeur){
           compteur++;
           pointeur_text++;

         }
        lseek (file,compteur+compteur2,SEEK_SET);
        compteur=0;
        while (*pointeur != '\n'){
          compteur++;
          pointeur++;
        }
         char blanc[compteur];
         memset(blanc,' ',compteur);
         write(file,blanc,compteur);
         printf("\nFin suppression dans : %s\n", chemin);
     }
     else{
        printf("Nom non trouvé - impossible de supprimer\n");
      }
    close(file);
    }
  }
  closedir(rep);
  return EXIT_SUCCESS;
}
