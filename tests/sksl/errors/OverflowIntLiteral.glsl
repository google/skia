### Compilation failed:

error: 2: value is out of range for type 'int': -2147483649
const int intMinMinusOne = -2147483649;                 // error
                           ^^^^^^^^^^^
error: 4: value is out of range for type 'int': 2147483648
const int intMaxPlusOne  = 2147483648;                  // error
                           ^^^^^^^^^^
error: 5: value is out of range for type 'int': 2147483648
int   cast_int   = int(2147483648.);                    // error
                       ^^^^^^^^^^^
3 errors
