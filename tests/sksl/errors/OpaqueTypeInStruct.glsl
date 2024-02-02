### Compilation failed:

error: 1: opaque type 'sampler' is not permitted in a struct
struct Bad { sampler x; };
             ^^^^^^^^^
error: 2: variables of type 'Bad' may not be uniform
uniform Bad b;
^^^^^^^^^^^^^
error: 1: caused by:
struct Bad { sampler x; };
             ^^^^^^^^^
3 errors
