### Compilation failed:

error: 1: type 'void' may not be used in an array
void a[2];
^^^^^^^^^
error: 2: type 'void' may not be used in an array
void[2] b;
^^^^^^^
error: 4: type 'void' may not be used in an array
void[2] funcF() {}
^^^^^^^
error: 4: function 'funcF' can exit without returning a value
void[2] funcF() {}
                ^^
error: 5: type 'void' may not be used in an array
void funcG() { void g[2]; }
               ^^^^^^^^^
error: 6: type 'void' may not be used in an array
void funcH() { void[2] h; }
               ^^^^^^^
error: 7: type 'void' may not be used in an array
void funcI() { void[2]; }
               ^^^^^^^
7 errors
