### Compilation failed:

error: 15: layout qualifier 'sampler' is not permitted here
layout(webgpu, set=0, sampler=0) readonly texture2D rtexture3;              // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 16: layout qualifier 'sampler' is not permitted here
layout(webgpu, set=0, sampler=0) writeonly texture2D wtexture3;             // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 17: 'binding' modifier cannot coexist with 'texture'/'sampler'
layout(webgpu, set=0, binding=0, texture=0, sampler=0) sampler2D sampler3;  // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 18: layout qualifier 'texture' is not permitted here
layout(webgpu, set=0, texture=0, sampler=0) uniform ubo2 { float c; };      // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 18: layout qualifier 'sampler' is not permitted here
layout(webgpu, set=0, texture=0, sampler=0) uniform ubo2 { float c; };      // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
5 errors
