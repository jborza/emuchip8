CC=gcc
emuchip8:
	$(CC) -o emuchip8 -g cpu.c font.c main.c -lSDL2 
clean: 
	rm ./emuchip8