### Compilation failed:

error: 4: variables of type 'void' are not allowed
void a;
^^^^^^
error: 5: variables of type 'void' are not allowed
void b = func();
^^^^^^^^^^^^^^^
error: 11: unknown identifier 'a'
        case 1: void c = a;
                         ^
error: 11: variables of type 'void' are not allowed
        case 1: void c = a;
                ^^^^^^^^^^
4 errors
