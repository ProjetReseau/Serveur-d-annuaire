#define SERVEUR

#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include <unistd.h>
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


void * connexion(void* envoy){
  fifo* envoi=(fifo*)envoy;
  int sock=envoi->sock;
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
      //close(sock);
      break;
    }
    result_read=errno;
		//printf("buffer reçu : %s\n", buffer);


    if ((result_read != EWOULDBLOCK)&&(result_read != EAGAIN)){
	str_to_tr(datas,&trame_read);
	//bzero(datas,sizeof(datas));
      timeToSleep=1;
      //printf("Type reçu: %d\n", trame_read.type_message);
			//printf("trame reçu: %i %i %s\n", trame_read.taille, trame_read.type_message, trame_read.message);
      if (trame_read.type_message==hello){
	       	strcpy(envoi->pseudo,trame_read.message);
	        printf("%s vient de se connecter \n",envoi->pseudo);
          char  ip[16], port[6];
		sscanf(envoi->ext_dist, "%s %s", ip, port);
          Rajouter_extremite("annuaire",envoi->pseudo, ip, port, -1);
      }
      else if(trame_read.type_message==quit){
        	printf("Fermeture de connexion (en toute tranquillité)\n");
		write(sock,datas,TAILLE_MAX_MESSAGE+32);
		break;
        	//exit(EXIT_FAILURE);
      }
      else if (trame_read.type_message==annuaireAsk){
          char *port, *ip;
          port=calloc(sizeof(int), 6);
          ip=calloc(sizeof(int), 16);
          lecture_nom(trame_read.message,ip, port);
          //printf("L'extremite de %s est : %s %s\n", trame_read.message, ip, port);
          trame_write.type_message=texte;
          sprintf(trame_write.message,"%s %s %s",trame_read.message, ip, port);
	trame_write.taille=strlen(trame_write.message);
	tr_to_str(datas, trame_write);
          write(sock, datas, TAILLE_MAX_MESSAGE+32);
          free(port);
          free(ip);
      }
      else if (trame_read.type_message==annuaireInfo){
					//bzero(datas,TAILLE_MAX_MESSAGE);
          if(trame_read.taille==0)	info(buffer);
					else info_fichier(buffer, trame_read.message);
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
				Rajouter_extremite(trame_read.message, envoi->pseudo, ip, port, -1);
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
  
  suppression(envoi->pseudo);
  close(sock);
  supprimer_fifo(envoi);
  
  
}

void * waitConnectFROM(){

 int sock;
 struct sockaddr_in extremite_locale, extremite_distante;
 socklen_t length = sizeof(struct sockaddr_in);
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

  Rajouter_extremite("annuaire", "Serveurd'annuaire", "0.0.0.0","14650", 0);

  char datas[TAILLE_MAX_MESSAGE];

 //printf("Tapez : 'Serveur Annaire'");
 //saisir_texte(pseudo, TAILLE_PSEUDO);
 strcpy(pseudo, "Serveurd'annuaire");


 waitConnectFROM();


 return EXIT_SUCCESS;
}
