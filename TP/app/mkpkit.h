#ifndef MKPKIT_H_
#define MKPKIT_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define MAXDOUBLE 1000000
#define CHECK(p) if (p==NULL) puts("Out of memory")

typedef struct {
     int n;
     int m;
     int **a;
} Mkp;

   void alloc_mkp(Mkp *mkp);			/* allocation d'un Mkp */

   Mkp *load_mkp(char *file);			/* chargement des données depuis le fichier texte file */

   void save_mkp(Mkp *mkp, char *file);	/* sauvegarde du Mkp dans le fichier texte file */

   void del_mkp(Mkp *mkp);				/* libération mémoire d'un Mkp */

#endif
