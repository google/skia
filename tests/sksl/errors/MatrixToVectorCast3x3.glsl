### Compilation failed:

error: 4: 'half3x3' is not a valid parameter to 'half' constructor; use '[0][0]' instead
half    testScalar    = half (testMatrix3x3);
                        ^^^^^^^^^^^^^^^^^^^^
error: 5: 'half3x3' is not a valid parameter to 'half2' constructor
half2   testVec2      = half2(testMatrix3x3);
                        ^^^^^^^^^^^^^^^^^^^^
error: 6: 'half3x3' is not a valid parameter to 'half3' constructor
half3   testVec3      = half3(testMatrix3x3);
                        ^^^^^^^^^^^^^^^^^^^^
error: 7: 'half3x3' is not a valid parameter to 'half4' constructor
half4   testVec4      = half4(testMatrix3x3);
                        ^^^^^^^^^^^^^^^^^^^^
4 errors
