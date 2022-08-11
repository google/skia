### Compilation failed:

error: 1: pipeline inputs not permitted in compute shaders
layout(set=0, binding=0) in int x;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 2: pipeline outputs not permitted in compute shaders
layout(set=0, binding=1) out float4 y;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 3: pipeline inputs not permitted in compute shaders
layout(set=0, binding=2) in int[5] z;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 4: pipeline inputs not permitted in compute shaders
layout(set=0, binding=3) inout float w;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 5: unsized arrays are not permitted here
layout(set=0, binding=4) out int[] a;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 5: pipeline outputs not permitted in compute shaders
layout(set=0, binding=4) out int[] a;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 5: 'out' variables may not have unsized array type
layout(set=0, binding=4) out int[] a;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 6: unsized arrays are not permitted here
layout(set=0, binding=5) in int[] b;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 6: pipeline inputs not permitted in compute shaders
layout(set=0, binding=5) in int[] b;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 6: 'in' variables may not have unsized array type
layout(set=0, binding=5) in int[] b;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 7: pipeline inputs not permitted in compute shaders
layout(set=0, binding=6) in blockOne {
                            ^^^^^^^^
error: 10: pipeline outputs not permitted in compute shaders
layout(set=0, binding=5) out blockTwo {
                             ^^^^^^^^
12 errors
