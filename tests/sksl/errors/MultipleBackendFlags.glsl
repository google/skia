### Compilation failed:

error: 1: only one backend qualifier can be used
layout(metal, spirv, wgsl, binding = 0) uniform ubo { float f; };  // multiple backends
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 2: layout qualifier 'texture' is not permitted here
layout(texture=0, sampler=0) uniform sampler2D s;  // invalid (requires backend)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 2: layout qualifier 'sampler' is not permitted here
layout(texture=0, sampler=0) uniform sampler2D s;  // invalid (requires backend)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
3 errors
