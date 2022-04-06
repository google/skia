### Compilation failed:

error: 1: invalid arguments to 'float[2]' constructor (expected 2 elements, but found 0)
float noElements[2]            = float[2]();
                                 ^^^^^^^^^^
error: 2: invalid arguments to 'float[2]' constructor (expected 2 elements, but found 1)
float notEnoughElements[2]     = float[2](1);
                                 ^^^^^^^^^^^
error: 4: invalid arguments to 'float[2]' constructor (expected 2 elements, but found 3)
float tooManyElements[2]       = float[2](1, 2, 3);
                                 ^^^^^^^^^^^^^^^^^
3 errors
