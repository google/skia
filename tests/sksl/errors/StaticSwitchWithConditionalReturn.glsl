### Compilation failed:

error: 3: static switch contains non-static conditional exit
void test_return()   { @switch (1) { case 1: if (testInputs.x > 1) return; } }
                       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
1 error
