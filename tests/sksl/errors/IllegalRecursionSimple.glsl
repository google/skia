### Compilation failed:

error: 4: potential recursion (function call cycle) not allowed:
	int fibonacci(int n)
	int fibonacci(int n)
int fibonacci(int n) { return n <= 1 ? n : fibonacci(n - 1) + fibonacci(n - 2); }
                     ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
1 error
