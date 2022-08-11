### Compilation failed:

error: 8: 'threadgroup' is not permitted here
   threadgroup float bar;
   ^^^^^^^^^^^
error: 11: 'threadgroup' is not permitted here
threadgroup texture2D tex;
^^^^^^^^^^^
error: 13: 'threadgroup' is not permitted here
threadgroup void a() {}
^^^^^^^^^^^
error: 15: 'threadgroup' is not permitted here
void b(threadgroup int b) {}
       ^^^^^^^^^^^
error: 18: expected expression, but found 'threadgroup'
    threadgroup bool x;
    ^^^^^^^^^^^
5 errors
