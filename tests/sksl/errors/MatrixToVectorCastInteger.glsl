### Compilation failed:

error: 4: 'half2x2' is not a valid parameter to 'int' constructor
int    testScalar    = int (testMatrix2x2);
                       ^^^^^^^^^^^^^^^^^^^
error: 5: 'half2x2' is not a valid parameter to 'int2' constructor
int2   testVec2      = int2(testMatrix2x2);
                       ^^^^^^^^^^^^^^^^^^^
error: 6: 'half2x2' is not a valid parameter to 'int3' constructor
int3   testVec3      = int3(testMatrix2x2);
                       ^^^^^^^^^^^^^^^^^^^
3 errors
