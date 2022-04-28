### Compilation failed:

error: 5: potential recursion (function call cycle) not allowed:
	bool is_odd(int n)
	bool is_even(int n)
	bool is_odd(int n)
bool is_odd (int n) { return n == 0 ? false : is_even(n - 1); }
                    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
1 error
