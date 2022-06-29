### Compilation failed:

error: 1: in / out variables may not be declared threadgroup
layout(set=0, binding=0) in threadgroup float x[];
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 2: in / out variables may not be declared threadgroup
layout(set=0, binding=1) out threadgroup float y[];
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 4: 'threadgroup' is not permitted here
threadgroup texture2D tex;
^^^^^^^^^^^
error: 6: 'threadgroup' is not permitted here
threadgroup void a() {}
^^^^^^^^^^^
error: 8: 'threadgroup' is not permitted here
void b(threadgroup int b) {}
       ^^^^^^^^^^^
error: 11: expected expression, but found 'threadgroup'
    threadgroup bool x;
    ^^^^^^^^^^^
6 errors
