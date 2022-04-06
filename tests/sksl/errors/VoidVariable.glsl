### Compilation failed:

error: 4: variables of type 'void' are not allowed
void a;
^^^^^^
error: 5: variables of type 'void' are not allowed
void b = func();
^^^^^^^^^^^^^^^
error: 5: global variable initializer must be a constant expression
void b = func();
         ^^^^^^
3 errors
