### Compilation failed:

error: 1: may not return a value from a void function
void a() { return true; }
                  ^^^^
error: 2: may not return a value from a void function
void b() { return b; }
                  ^
error: 3: may not return a value from a void function
void c() { return int; }
                  ^^^
3 errors
