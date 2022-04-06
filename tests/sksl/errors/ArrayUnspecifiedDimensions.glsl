### Compilation failed:

error: 1: expected array dimension
int arrUnsized[];
              ^^
error: 2: array size must be an integer
int arrFloat[1.];
             ^^
error: 3: array size must be an integer
int arrBool[true];
            ^^^^
error: 5: missing index in '[]'
int unsized_in_expression() { return int[](0)[0]; }
                                        ^^
4 errors
