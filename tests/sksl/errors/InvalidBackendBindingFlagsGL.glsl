### Compilation failed:

error: 8: layout qualifier 'texture' is not permitted here
layout(gl, texture=0) uniform texture2D t10;              // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 9: layout qualifier 'texture' is not permitted here
layout(gl, texture=0, sampler=0) uniform sampler2D s11;   // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 9: layout qualifier 'sampler' is not permitted here
layout(gl, texture=0, sampler=0) uniform sampler2D s11;   // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 10: layout qualifier 'set' is not permitted here
layout(gl, set=0, binding=0) uniform ubo9 { float f13; }; // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
4 errors
