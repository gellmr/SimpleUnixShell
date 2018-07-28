#makefile for mash
#the filename must be makefile or Makefile

####################################################
#
#		"Mikes Awesome Shell"
#
#		 	Usage:
#
#			"make ./bin/mash"
#
####################################################

mash: ./obj/main.o ./obj/token.o ./obj/command.o ./obj/shellEngine.o
	gcc ./obj/main.o ./obj/token.o ./obj/command.o ./obj/shellEngine.o -o ./bin/mash


./obj/main.o: ./source/main.c ./include/token.h ./include/command.h ./include/shellEngine.h
	gcc -c ./source/main.c -o ./obj/main.o


./obj/shellEngine.o: ./source/shellEngine.c ./include/shellEngine.h ./include/command.h
	gcc -c ./source/shellEngine.c -o ./obj/shellEngine.o


./obj/token.o: ./source/token.c ./include/token.h
	gcc -c ./source/token.c -o ./obj/token.o


./obj/command.o: ./source/command.c ./include/command.h
	gcc -c ./source/command.c -o ./obj/command.o

clean:
	rm ./obj/*.o --force
	rm *.o --force
