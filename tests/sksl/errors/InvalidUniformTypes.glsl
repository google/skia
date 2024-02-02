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
error: 21: variables of type 'InvalidStruct1' may not be uniform
uniform InvalidStruct1 st1;
^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 14: caused by:
    bool b; // invalid
    ^^^^^^
error: 22: variables of type 'InvalidStruct2' may not be uniform
uniform InvalidStruct2 st2;
^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 14: caused by:
    bool b; // invalid
    ^^^^^^
error: 24: variables of type 'invalidBlock' may not be uniform
uniform invalidBlock {
        ^^^^^^^^^^^^
error: 26: caused by:
    bool b; // invalid
    ^^^^^^^
10 errors
