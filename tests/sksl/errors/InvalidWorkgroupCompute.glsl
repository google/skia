### Compilation failed:

error: 8: modifier 'workgroup' is not permitted on an interface block field
   workgroup float bar;
   ^^^^^^^^^^^^^^^^^^^^
error: 11: 'workgroup' is not permitted here
layout (r32f) workgroup readonly texture2D rtex;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 13: 'workgroup' is not permitted here
layout (r32f) workgroup writeonly texture2D wtex;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 15: 'workgroup' is not permitted here
workgroup void a() {}
^^^^^^^^^
error: 17: 'workgroup' is not permitted here
void b(workgroup int b) {}
       ^^^^^^^^^
error: 20: expected expression, but found 'workgroup'
    workgroup bool x;
    ^^^^^^^^^
6 errors
