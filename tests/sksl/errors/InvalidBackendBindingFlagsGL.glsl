### Compilation failed:

error: 8: layout qualifier 'texture' is not permitted here
layout(gl, texture=0) uniform texture2D texture2;             // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 9: layout qualifier 'texture' is not permitted here
layout(gl, texture=0, sampler=0) uniform sampler2D sampler2;  // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 9: layout qualifier 'sampler' is not permitted here
layout(gl, texture=0, sampler=0) uniform sampler2D sampler2;  // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 10: layout qualifier 'set' is not permitted here
layout(gl, set=0, binding=0) uniform ubo2 { float c; };       // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
4 errors
