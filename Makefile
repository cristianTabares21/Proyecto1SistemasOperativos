CC=gcc

%.o: %.c
	$(CC) -c -o $@ $<

unixShell: unixShell.o
	gcc -o unixShell unixShell.o
	
clean:
	rm -f *.o unixShell
	rm -f *.txt unixShell
