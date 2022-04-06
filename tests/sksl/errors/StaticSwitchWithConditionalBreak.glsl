### Compilation failed:

error: 3: static switch contains non-static conditional exit
void test_break()    { @switch (1) { case 1: if (testInputs.x > 2) break; } }
                       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
1 error
