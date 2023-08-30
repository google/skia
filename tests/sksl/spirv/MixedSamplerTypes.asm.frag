### Compilation failed:

error: programs cannot contain a mixture of sampler types
error: 2: Vulkan sampler found here:
layout(vulkan, set=1, binding=4)            sampler2D vkSampler;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 1: WebGPU sampler found here:
layout(webgpu, set=1, texture=2, sampler=3) sampler2D wgpuSampler;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
3 errors
