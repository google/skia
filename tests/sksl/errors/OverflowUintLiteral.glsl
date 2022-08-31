### Compilation failed:

error: 4: value is out of range for type 'uint': -1
uint uintMinMinusOne = -1;                             // error
                       ^^
error: 6: integer is too large: 4294967296
uint uintMaxPlusOne  = 4294967296;                     // error
                       ^^^^^^^^^^
error: 8: value is out of range for type 'ushort': -1
ushort4 us4_neg = ushort4(2, 1, 0, -1);                // error -1
                                   ^^
error: 9: value is out of range for type 'ushort': 65536
ushort4 us4_pos = ushort4(65536, 65535, 65534, 65533); // error 65536
                          ^^^^^
error: 11: value is out of range for type 'uint': 4294967296
uint   cast_int   = uint(4294967296.);                 // error
                         ^^^^^^^^^^^
error: 12: value is out of range for type 'ushort': 65536
ushort cast_short = ushort(65536.);                    // error
                           ^^^^^^
6 errors
