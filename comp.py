
def fib(x):
	return 1 if x < 2 else fib(x - 1) + fib(x - 2)

import timeit
print(timeit.timeit(lambda: print(fib(20)), number=1))
