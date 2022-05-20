### Compilation failed:

error: 6: variables of type 'bool' may not be uniform
uniform bool  b;
^^^^^^^^^^^^^^^
error: 7: variables of type 'bool2' may not be uniform
uniform bool2 b2;
^^^^^^^^^^^^^^^^
error: 8: variables of type 'bool3' may not be uniform
uniform bool3 b3;
^^^^^^^^^^^^^^^^
error: 9: variables of type 'bool4' may not be uniform
uniform bool4 b4;
^^^^^^^^^^^^^^^^
error: 14: variables of type 'bool' may not be uniform
    bool b; // invalid
    ^^^^^^
error: 21: caused by:
uniform InvalidStruct1 st1;
^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 14: variables of type 'bool' may not be uniform
    bool b; // invalid
    ^^^^^^
error: 22: caused by:
uniform InvalidStruct2 st2;
^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 26: variables of type 'bool' may not be uniform
    bool b; // invalid
    ^^^^^^^
error: 24: caused by:
uniform invalidBlock {
        ^^^^^^^^^^^^
10 errors
