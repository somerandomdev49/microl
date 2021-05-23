CC = gcc # gcc-9 for my setup.

microl: *.c *.h
	$(CC) *.c -o microl -g
