### Compilation failed:

error: 1: symbol 'r' was already defined
void n() { for(int r,r;;) int s; }
                     ^
error: 1: variable 's' must be created in a scope
void n() { for(int r,r;;) int s; }
                          ^^^^^
2 errors
