#include "mkpkit.h"
#include "mkpsol.h"

int main(int argc, char *argv[]) {
	Mkp *mkp;
	Solution *s;

	if(argc != 2) {
		printf("Usage: programme fichier\n");
		exit(0);
	}
	mkp = load_mkp(argv[1]);
	save_mkp(mkp, "verif.txt");

	s = alloc_sol(mkp);
	Add(mkp, s, 1);
	Add(mkp, s, 10);
	printf("obj= %d\n", s->objValue);
	Drop(mkp, s, 10);
	printf("obj= %d\n", s->objValue);

	free_sol(s);
	del_mkp(mkp);

	return 0;
}
