### Compilation failed:

error: 15: layout qualifier 'sampler' is not permitted here
layout(webgpu, rgba32f, set=0, sampler=0) readonly texture2D rtexture3;    // invalid (has sampler)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 16: layout qualifier 'sampler' is not permitted here
layout(webgpu, rgba32f, set=0, sampler=0) writeonly texture2D wtexture3;   // invalid (has sampler)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 17: storage textures must declare a pixel format
layout(webgpu, set=0, texture=0) readonly texture2D rtexture4;             // invalid (no pixformat)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 18: storage textures must declare a pixel format
layout(webgpu, set=0, texture=0) writeonly texture2D wtexture4;            // invalid (no pixformat)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 19: 'binding' modifier cannot coexist with 'texture'/'sampler'
layout(webgpu, set=0, binding=0, texture=0, sampler=0) sampler2D sampler3; // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 20: layout qualifier 'texture' is not permitted here
layout(webgpu, set=0, texture=0, sampler=0) uniform ubo2 { float c; };     // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 20: layout qualifier 'sampler' is not permitted here
layout(webgpu, set=0, texture=0, sampler=0) uniform ubo2 { float c; };     // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
7 errors
