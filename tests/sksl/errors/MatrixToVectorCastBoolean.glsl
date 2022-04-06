### Compilation failed:

error: 4: 'half2x2' is not a valid parameter to 'bool' constructor
bool    testScalar    = bool (testMatrix2x2);
                        ^^^^^^^^^^^^^^^^^^^^
error: 5: 'half2x2' is not a valid parameter to 'bool2' constructor
bool2   testVec2      = bool2(testMatrix2x2);
                        ^^^^^^^^^^^^^^^^^^^^
error: 6: 'half2x2' is not a valid parameter to 'bool3' constructor
bool3   testVec3      = bool3(testMatrix2x2);
                        ^^^^^^^^^^^^^^^^^^^^
3 errors
