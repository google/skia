### Compilation failed:

error: 8: layout qualifier 'texture' is not permitted here
layout(spirv, texture=0) uniform texture2D t5;             // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 9: layout qualifier 'texture' is not permitted here
layout(spirv, texture=0, sampler=0) uniform sampler2D s6;  // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 9: layout qualifier 'sampler' is not permitted here
layout(spirv, texture=0, sampler=0) uniform sampler2D s6;  // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
3 errors
