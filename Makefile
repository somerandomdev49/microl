CC = gcc-9

microl: *.c *.h
	$(CC) *.c -o microl -g
