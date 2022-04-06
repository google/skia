### Compilation failed:

error: 3: static switch contains non-static conditional exit
void test_continue() { for (;;) { @switch (1) { case 1: if (testInputs.x > 3) continue; } } }
                                  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
1 error
