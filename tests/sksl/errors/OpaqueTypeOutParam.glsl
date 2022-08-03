### Compilation failed:

error: 2: 'out' is not permitted here
void func2(out sampler2D s)   { /*error*/ }
           ^^^
error: 3: 'out' is not permitted here
void func3(inout sampler2D s) { /*error*/ }
           ^^^^^
2 errors
