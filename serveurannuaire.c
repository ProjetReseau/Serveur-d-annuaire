#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<unistd.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>

#define BUFFERSIZE 512

int lecture_nom(char * nom, char * ip, char * port){
  printf("\n--- Lecture ---\n\n");
  int file;
  char * pointeur=NULL;
  char buffer[BUFFERSIZE];

  file=open("./annuaire.txt", O_RDONLY);
 
  while((read(file, buffer, BUFFERSIZE)>0)&&(pointeur==NULL)){ 
    pointeur=strstr(buffer, nom);
  }  
 
  if (pointeur != NULL){
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
    return EXIT_FAILURE;
  }

  close(file);
  return EXIT_SUCCESS;
}

int Rajouter_extremite(char * nom, char * ip, char * port){
  printf("\n--- Rajout d'une extremite ---\n\n");

  int file;
  char * extremite=calloc(sizeof(int), 512), * pointeur=NULL;
  char buffer[BUFFERSIZE];

  sprintf (extremite, "%s %s %s", nom, ip, port);

  file=open("./annuaire.txt", O_RDWR);

  while((read(file, buffer, BUFFERSIZE)>0)&&(pointeur==NULL)){
    pointeur=strstr(buffer, nom);
  }
  //printf("Pointeur = %c\n", *pointeur);
  if (pointeur != NULL){
    printf("Ce pseudo est déjà renseigné\n");
    return EXIT_FAILURE;
  }
  else {
    lseek(file,0,SEEK_END);
    write(file,extremite, strlen(extremite));
    close(file);
  }
  return EXIT_SUCCESS;
}

int suppression(char * nom){
  printf("\n--- Suppression ---\n\n");

  int file;
  char * pointeur=NULL;//, * pointeur_text=NULL;
  char buffer[BUFFERSIZE];

  file=open("./annuaire.txt", O_RDWR);
 
   while((read(file, buffer, BUFFERSIZE)>0)&&(pointeur==NULL)){
     pointeur=strstr(buffer, nom);
   }
  
  /* if (pointeur != NULL){
     pointeur_text=pointeur;
     while (*pointeur_text != '\n'){
       pointeur_text++;
     }
     pointeur_text++;
     printf("pointeur = %c\n", *pointeur);
     printf("pointeur_text = %c\n", *pointeur_text);

     while ((nchar=read(file, buffer2, BUFFERSIZE))){
       printf("nchar=%i\n", nchar);
       write(file, buffer, nchar);
       printf("pointeur = %s\n", pointeur);
     }
*/ 
  while((*pointeur!='\n')&&(pointeur != NULL) && (pointeur != '\0')){
    printf("pointeur = %c\n", *pointeur);    
    *pointeur='A';
    printf("pointeur après = %c\n", *pointeur);
    pointeur++;
    //pointeur_text++;
  } 
  
  close(file); 
  return EXIT_SUCCESS;
}

int main(int argc, char ** argv){

  if (argc==2){

    char * port="", * ip="", * nom=argv[1];
    port=calloc(sizeof(int), 6);
    ip=calloc(sizeof(int), 15);

    lecture_nom(nom,ip, port);
    printf("L'extremite de %s est : %s %s\n", nom, ip, port);
    }

  else if (argc==4) {

   Rajouter_extremite(argv[1], argv[2], argv[3]);
       
  }

  else if (argc==3){
   suppression(argv[1]);
  }
  return EXIT_SUCCESS;
}
