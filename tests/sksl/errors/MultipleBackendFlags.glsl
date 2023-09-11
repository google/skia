### Compilation failed:

error: 1: only one backend qualifier can be used
layout(metal, vulkan, webgpu, direct3d, binding = 0) uniform ubo { float f; }; // multiple backends
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 2: layout qualifier 'texture' is not permitted here
layout(texture=0, sampler=0) sampler2D s;                                      // missing backend
^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 2: layout qualifier 'sampler' is not permitted here
layout(texture=0, sampler=0) sampler2D s;                                      // missing backend
^^^^^^^^^^^^^^^^^^^^^^^^^^^^
3 errors
