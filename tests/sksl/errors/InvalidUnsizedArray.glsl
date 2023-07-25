### Compilation failed:

error: 1: unsized arrays are not permitted here
in int[] a;
^^^^^^^^^^
error: 1: pipeline inputs not permitted in compute shaders
in int[] a;
^^^^^^^^^^
error: 1: 'in' variables may not have unsized array type
in int[] a;
^^^^^^^^^^
error: 2: multi-dimensional arrays are not supported
int[][] b;
^^^^^^^
error: 5: unsized array must be the last member of a storage block
    int[] c;
    ^^^^^^^^
error: 9: unsized array must be the last member of a storage block
    int[] d;
    ^^^^^^^^
error: 14: unsized array must be the last member of a storage block
    int[] f;
    ^^^^^^^^
error: 19: multi-dimensional arrays are not supported
    int[][] h;
    ^^^^^^^
error: 23: unsized arrays are not permitted here
    int[] i;
    ^^^^^^^
error: 23: unsized arrays are not permitted here
    int[] i;
    ^^^^^^^
10 errors
