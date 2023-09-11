### Compilation failed:

error: 4: modifiers on parameter 1 differ between declaration and definition
void overloaded_function(layout(r32f)    writeonly texture2D tex) {}
                         ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 5: modifiers on parameter 1 differ between declaration and definition
void overloaded_function(layout(rgba8)   writeonly texture2D tex) {}
                         ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
2 errors
