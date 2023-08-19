### Compilation failed:

error: 9: operator ',' can not operate on arrays (or structs containing arrays)
    myStruct, 123;
    ^^^^^^^^
error: 10: operator ',' can not operate on arrays (or structs containing arrays)
    123, myStruct;
         ^^^^^^^^
error: 11: operator ',' can not operate on arrays (or structs containing arrays)
    myArray, 123;
    ^^^^^^^
error: 12: operator ',' can not operate on arrays (or structs containing arrays)
    123, myArray;
         ^^^^^^^
error: 13: operator ',' can not operate on arrays (or structs containing arrays)
    myArray, myStruct;
    ^^^^^^^
error: 14: operator ',' can not operate on arrays (or structs containing arrays)
    myStruct, myArray;
    ^^^^^^^^
6 errors
