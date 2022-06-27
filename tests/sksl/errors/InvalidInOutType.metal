### Compilation failed:

error: 1: unsupported compute shader in / out type
layout(set=0, binding=0) in int x;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 2: unsupported compute shader in / out type
layout(set=0, binding=1) out float4 y;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 3: compute shader in / out arrays must be unsized
layout(set=0, binding=2) in int[5] z;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
3 errors
