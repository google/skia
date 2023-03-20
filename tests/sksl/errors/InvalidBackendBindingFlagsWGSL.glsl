### Compilation failed:

error: 13: layout qualifier 'sampler' is not permitted here
layout(wgsl, set=0, sampler=0) uniform texture2D texture3;                        // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 14: 'binding' modifier cannot coexist with 'texture'/'sampler'
layout(wgsl, set=0, binding=0, texture=0, sampler=0) uniform sampler2D sampler3;  // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 15: layout qualifier 'texture' is not permitted here
layout(wgsl, set=0, texture=0, sampler=0) uniform ubo2 { float c; };              // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 15: layout qualifier 'sampler' is not permitted here
layout(wgsl, set=0, texture=0, sampler=0) uniform ubo2 { float c; };              // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
4 errors
