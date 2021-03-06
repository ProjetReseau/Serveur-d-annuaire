#define SERVEUR
#define _XOPEN_SOURCE

#include<stdio.h>
#include <signal.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<string.h>
#include<strings.h>
#include<netdb.h>
#include"protocole.h"
#include "file.h"
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include"fonction_annuaire.h"

#ifndef EWOULDBLOCK
	#define EWOULDBLOCK EAGAIN
#endif


static char pseudo[TAILLE_PSEUDO];
static pthread_t th;

void gestionnaire (int numero)
{
   printf("\n---Suppression des fichiers---\n");
   system("rm -rf ./Fichier/*");
   system("touch ./Fichier/annuaire.txt");
}

void * connexion(void* envoy){
  

  
  fifo* envoi=(fifo*)envoy;
  int sock=envoi->sock;
  int i;
  trame trame_read;
  trame trame_write;
  trame trame1;
  char datas[TAILLE_MAX_MESSAGE+32];
  ssize_t result_read;
  useconds_t timeToSleep=100;
  int nchar=0;
  char buffer[TAILLE_MAX_MESSAGE+8];

//  char pseudo[TAILLE_PSEUDO];

/*  printf("Choisir un pseudo: ");
  saisir_texte(pseudo,TAILLE_PSEUDO);
*/
  //Dis "bonjour !" en envoyant son pseudo
  bzero(trame1.message,TAILLE_MAX_MESSAGE);
  strcpy(trame1.message,pseudo);
  trame1.type_message=hello;
  trame1.taille=strlen(trame1.message);
  fcntl(sock,F_SETFL,fcntl(sock,F_GETFL)|O_NONBLOCK);
  tr_to_str(datas,trame1);
  write(sock,datas, TAILLE_MAX_MESSAGE+32);



  while(1){
    bzero(trame_read.message,TAILLE_MAX_MESSAGE);
    timeToSleep=1000;

    //Phase lecture
    errno=0;
    if((nchar=read(sock,datas,TAILLE_MAX_MESSAGE+32))==0){
      printf("Connexion interrompue\n");
      break;
    }
    result_read=errno;


    if ((result_read != EWOULDBLOCK)&&(result_read != EAGAIN)){
      
      str_to_tr(datas,&trame_read);
      //bzero(datas,sizeof(datas));
      timeToSleep=1;
      //printf("Type reçu: %d\n", trame_read.type_message);
      //printf("trame reçu: %i %i %s\n", trame_read.taille, trame_read.type_message, trame_read.message);
     
      if (trame_read.type_message==hello){
	char  ip[16], port[6];
	//strcpy(envoi->pseudo,trame_read.message);
	sscanf(trame_read.message, "%s %s", envoi->pseudo, port);
	printf("%s vient de se connecter \n",envoi->pseudo);
	bzero(datas,TAILLE_MAX_MESSAGE+32);
	//sscanf(envoi->ext_dist, "%s %s", ip, port);
	sscanf(envoi->ext_dist, "%s", ip);
        if (Rajouter_extremite("annuaire",envoi->pseudo, ip, port, -1)==EXIT_FAILURE){
	  trame_write.type_message=texte;
	  strcpy(trame_write.message, "Pseudo déjà renseigné. Veuillez en choisir un autre");
	  tr_to_str(datas,trame_write);
	  write(sock, datas, TAILLE_MAX_MESSAGE+32);
	  i=0;
	  break;
	}
      }
      else if(trame_read.type_message==quit){
        printf("Fermeture de connexion (en toute tranquillité)\n");
	write(sock,datas,TAILLE_MAX_MESSAGE+32);
	i=1;
	break;
      }
      else if (trame_read.type_message==annuaireAsk){
          char *port, *ip;
          port=calloc(sizeof(int), 6);
          ip=calloc(sizeof(int), 16);
          if (lecture_nom(trame_read.message,ip, port)==EXIT_SUCCESS){
          //printf("L'extremite de %s est : %s %s\n", trame_read.message, ip, port);
          sprintf(trame_write.message,"%s %s %s",trame_read.message, ip, port);
	  }
	  else{
	    strcpy(trame_write.message, "Pseudo non renseigné");
	  }
	  trame_write.type_message=texte;
	  trame_write.taille=strlen(trame_write.message);
	  tr_to_str(datas, trame_write);
          write(sock, datas, TAILLE_MAX_MESSAGE+32);
          free(port);
          free(ip);
      }
      else if (trame_read.type_message==annuaireInfo){
	bzero(datas,TAILLE_MAX_MESSAGE+32);
	bzero(buffer,TAILLE_MAX_MESSAGE+8);
        if(trame_read.taille==0){
	  info(buffer);
	}
	else{
	  if (info_fichier(buffer, trame_read.message)==EXIT_FAILURE){
	    trame_write.type_message=texte;
	    strcpy(trame_write.message, "Le groupe demandé n'existe pas");
	    trame_write.taille=strlen(trame_write.message);
	    tr_to_str(datas,trame_write);
	    write(sock, datas, TAILLE_MAX_MESSAGE+32);
	  }
	}
        trame_write.type_message=texte;
        sprintf(trame_write.message,"%s",buffer);
	trame_write.taille=strlen(trame_write.message);
        tr_to_str(datas,trame_write);
        write(sock, datas, TAILLE_MAX_MESSAGE+32);

      }
      else if (trame_read.type_message==annuaireNew){
	//printf("trame_read.message : %s\n", trame_read.message);
	creer_fichier(trame_read.message, envoi->pseudo, envoi->ext_dist);
      }
      else if (trame_read.type_message==groupJoin){
	char ip[16], port[6];
	sscanf(envoi->ext_dist, "%s %s", ip, port);
	if (Rajouter_extremite(trame_read.message, envoi->pseudo, ip, port, -1)==EXIT_FAILURE)
	    strcpy(trame_write.message, "Pseudo déjà renseigné ou bien le groupe n'existe pas");
	else
	  strcpy(trame_write.message, "Ok");
	trame_write.type_message=texte;
	trame_write.taille=strlen(trame_write.message);
	tr_to_str(datas,trame_write);
	write(sock, datas, TAILLE_MAX_MESSAGE+32);
	
      }
      else {
	//printf("Reception d'un message texte\n");
	/*s*/printf(/*datas,*/"[%s] %s\n",envoi->pseudo,trame_read.message);	//pour une utilisation future
	//enfiler_fifo(recu, datas);
      }
     }


     bzero(buffer,TAILLE_MAX_MESSAGE);
    //Phase écriture
    /*if(!(estVide_fifo(envoi))){

      bzero(trame_write.message,TAILLE_MAX_MESSAGE);
      timeToSleep=1;
      defiler_fifo(envoi,trame_write.message);
      //saisir_texte(trame_write.message,TAILLE_MAX_MESSAGE);

      if(0==strcmp("QUIT",trame_write.message)){
        	trame_write.type_message=quit;
                else
        	trame_write.type_message=texte;
      }
      else {
          trame_write.type_message=texte; }
        	trame_write.taille=strlen(trame_write.message);
        	printf("Taille message: %d\n", trame_write.taille);
        	printf("Type message envoyé: %d\n", trame_write.type_message);
        	tr_to_str(buffer, trame_write);
        	write(sock,buffer,TAILLE_MAX_MESSAGE+8);
    }*/

    usleep(timeToSleep);

  }
  if (i==1){
    suppression(envoi->pseudo);
  }
  close(sock);
  supprimer_fifo(envoi);

	return;

}

void * waitConnectFROM(){

 int sock;
 struct sockaddr_in extremite_locale, extremite_distante;
 socklen_t length = sizeof(struct sockaddr_in);
 char port[6];
 fifo *envoy;

 //struct hostent *hote_distant;


 sock=socket(AF_INET, SOCK_STREAM, 0);
 if (sock==-1){
   perror ("Erreur appel sock ");
   exit(1);
 }

 extremite_locale.sin_family=AF_INET;
 extremite_locale.sin_addr.s_addr=htonl(INADDR_ANY);
 extremite_locale.sin_port=0;	//à fixer



 //RECUPERER IP/PORT DU SERVEUR

 if (bind(sock, (struct sockaddr *) &extremite_locale, sizeof(extremite_locale))==-1){
   perror("Erreur appel bind ");
   exit(1);
 }

 if (getsockname(sock,(struct sockaddr *) &extremite_locale, &length)<0){
   perror("Erreur appel getsockname ");
   exit(1);
 }

 printf("\nOuverture d'une socket (n°%i) sur le port %i on mode connecté\n", sock, ntohs(extremite_locale.sin_port));
 printf("extremite locale :\n sin_family = %d\n sin_addr.s_addr = %s\n sin_port = %d\n\n", extremite_locale.sin_family, inet_ntoa(extremite_locale.sin_addr), ntohs(extremite_locale.sin_port));

 printf("En attente de connexion.........\n");

 if (listen(sock, 2)<0){
     perror("Erreur appel listen ");
     exit(1);
 }

 sprintf(port, "%d",  ntohs(extremite_locale.sin_port));
 Rajouter_extremite("annuaire", pseudo, inet_ntoa(extremite_locale.sin_addr),port, 0);

 while(1){
   int ear=accept(sock, (struct sockaddr *) &extremite_distante, &length);
   if (ear == -1){
     perror("Erreur appel accept ");
     exit(1);
   }
   printf("\nConnection établie\n\n");

   printf("Connection sur la socket ayant le fd %d\nextremite distante\n sin_family : %d\n sin_addr.s_addr = %s\n sin_port = %d\n\n", ear, extremite_distante.sin_family, inet_ntoa(extremite_distante.sin_addr), ntohs(extremite_distante.sin_port));

   envoy=creer_fifo();
   envoy->sock=ear;

   sprintf(envoy->ext_dist, "%s %d ", inet_ntoa(extremite_distante.sin_addr),ntohs(extremite_distante.sin_port));

   pthread_create(&th,NULL,connexion,(void*) envoy);


   }

}


int main(int argc, char ** argv){
  
 signal(SIGINT, gestionnaire);

 strcpy(pseudo, "Serveurd'annuaire");

 waitConnectFROM();

 return EXIT_SUCCESS;
}
