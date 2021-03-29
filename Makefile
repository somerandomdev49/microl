CC = clang

microl: *.c *.h
	$(CC) *.c -o microl
