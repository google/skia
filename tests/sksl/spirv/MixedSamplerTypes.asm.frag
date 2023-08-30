### Compilation failed:

error: programs cannot contain a mixture of sampler types
error: 3: combined sampler found here:
layout(vulkan,   set=1, binding=6)            sampler2D vkSampler;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 2: separate sampler found here:
layout(direct3d, set=1, texture=4, sampler=5) sampler2D d3dSampler;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
3 errors
