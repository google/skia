### Compilation failed:

error: 1: opaque type 'sampler' is not permitted in a struct
struct Bad { sampler x; };
             ^^^^^^^^^
error: 1: variables of type 'sampler' may not be uniform
struct Bad { sampler x; };
             ^^^^^^^^^
error: 2: caused by:
uniform Bad b;
^^^^^^^^^^^^^
3 errors
