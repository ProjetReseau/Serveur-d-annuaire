#ifndef __FONCTION_SERVEUR__
#define __FONCTION_SERVEUR__

int info(char * message);
int info_fichier(char * message, char * nom_fichier);
int lecture_nom(char * nom, char * ip, char * port);
int Rajouter_extremite(char * nom_file, char * nom, char * ip, char * port, int end);
int suppression(char * nom);
int verif(char * lu, char * mot);
int creer_fichier(char * nom, char * pseudo, char * ext_dist);

#endif
