all: miniprojet_1

miniprojet_1:tp_mkpkit.o tp_mkpsol.o tp.o tp_main.o 
	gcc -o miniprojet_1 tp_mkpkit.o tp_mkpsol.o tp.o tp_main.o 
	

tp_mkpkit.o: tp_mkpkit.c	
	gcc -o tp_mkpkit.o -c tp_mkpkit.c -W -Wall -ansi -pedantic
	
tp_mkpsol.o: tp_mkpsol.c tp_mkpkit.c
	gcc -o tp_mkpsol.o -c tp_mkpsol.c -W -Wall -ansi -pedantic
	
tp.o: tp.c tp_mkpsol.c
	gcc -o tp.o -c tp.c -W -Wall -ansi -pedantic

tp_main.o: tp_main.c tp_mkpkit.c tp_mkpsol.c tp.c
	gcc -o tp_main.o -c tp_main.c -W -Wall -ansi -pedantic

clean:
	rm -rf *.o

mrproper: clean
	rm -rf miniprojet_1
