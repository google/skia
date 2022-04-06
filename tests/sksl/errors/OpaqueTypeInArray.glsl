### Compilation failed:

error: 1: opaque type 'sampler' may not be used in an array
in sampler a[1];
^^^^^^^^^^^^^^^
error: 2: opaque type 'sampler' may not be used in an array
in sampler[1] b;
   ^^^^^^^^^^
error: 3: opaque type 'sampler' may not be used in an array
void fnC() { sampler c[1]; }
             ^^^^^^^^^^^^
error: 4: opaque type 'sampler' may not be used in an array
void fnD() { sampler[1] d; }
             ^^^^^^^^^^
4 errors
