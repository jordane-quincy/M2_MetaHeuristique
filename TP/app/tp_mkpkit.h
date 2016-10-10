#ifndef TPMKPKIT_H_
#define TPMKPKIT_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define MAXDOUBLE 1000000
#define CHECK(p) if (p==NULL) puts("Out of memory")

/*
 * n: Nombre de valeurs.
 * cc: Nombre de contraintes de capacités.
 * cd: Nombre de contraintes de demandes.
 * a: Tableau deux dimensions.
**/
typedef struct {
     int n;
     int cc;
     int cd;
     int **a;
} tp_Mkp;

   void tp_alloc_mkp(tp_Mkp *mkp);			/* allocation d'un Mkp */

   tp_Mkp *tp_load_mkp(char *file);			/* chargement des données depuis le fichier texte file */

   void tp_save_mkp(tp_Mkp *mkp, char *file);	/* sauvegarde du Mkp dans le fichier texte file */

   void tp_del_mkp(tp_Mkp *mkp);				/* libération mémoire d'un Mkp */

#endif
