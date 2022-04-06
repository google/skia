### Compilation failed:

error: 4: 'half2x2' is not a valid parameter to 'half' constructor; use '[0][0]' instead
half    testScalar    = half (testMatrix2x2);
                        ^^^^^^^^^^^^^^^^^^^^
error: 5: 'half2x2' is not a valid parameter to 'half2' constructor
half2   testVec2      = half2(testMatrix2x2);
                        ^^^^^^^^^^^^^^^^^^^^
error: 6: 'half2x2' is not a valid parameter to 'half3' constructor
half3   testVec3      = half3(testMatrix2x2);
                        ^^^^^^^^^^^^^^^^^^^^
3 errors
