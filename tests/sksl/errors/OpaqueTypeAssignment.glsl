### Compilation failed:

error: 3: opaque type 'sampler' cannot use initializer expressions
sampler c = a;
            ^
error: 4: variables of type 'sampler' must be global
void declare() { sampler d; }
                 ^^^^^^^^^
error: 5: variables of type 'sampler' must be global
void initialize() { sampler e = a; }
                    ^^^^^^^^^^^^^
error: 5: opaque type 'sampler' cannot use initializer expressions
void initialize() { sampler e = a; }
                                ^
error: 6: assignments to opaque type 'sampler' are not permitted
void assign() { a = b; }
                ^^^^^
5 errors
