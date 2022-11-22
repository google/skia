### Compilation failed:

error: 8: modifier 'workgroup' is not permitted on a struct field
   workgroup float bar;
   ^^^^^^^^^^^^^^^^^^^^
error: 11: 'workgroup' is not permitted here
workgroup texture2D tex;
^^^^^^^^^
error: 13: 'workgroup' is not permitted here
workgroup void a() {}
^^^^^^^^^
error: 15: 'workgroup' is not permitted here
void b(workgroup int b) {}
       ^^^^^^^^^
error: 18: expected expression, but found 'workgroup'
    workgroup bool x;
    ^^^^^^^^^
5 errors
