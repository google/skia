### Compilation failed:

error: 1: unsized arrays are not permitted here
int arrUnsized[];
^^^^^^^^^^^^^^^^
error: 2: array size must be an integer
int arrFloat[1.];
             ^^
error: 3: array size must be an integer
int arrBool[true];
            ^^^^
error: 6: unsized arrays are not permitted here
    int inStructVariable[];
                         ^
error: 8: unsized arrays are not permitted here
S arrOfStruct[];
^^^^^^^^^^^^^^^
error: 10: missing index in '[]'
int unsized_in_expression_a() { return int[](0)[0]; }
                                          ^^
error: 11: missing index in '[]'
S   unsized_in_expression_b() { return S(int[](0)); }
                                            ^^
error: 14: unsized arrays are not permitted here
    int[] inStructType;
    ^^^^^^^^^^^^^^^^^^
error: 24: functions may not return type 'int[]'
int[] unsized_in_return_type_a() {}
^^^
error: 25: functions may not return type 'S[]'
S[]   unsized_in_return_type_b() {}
^
10 errors
