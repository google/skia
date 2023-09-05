### Compilation failed:

error: 10: layout qualifier 'rgba32f' is not permitted here
layout(vulkan, rgba32f, input_attachment_index=0,
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^...
error: 12: layout qualifier 'texture' is not permitted here
layout(vulkan, rgba32f, texture=0) readonly texture2D rtexture2;   // invalid (no set/binding)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 13: layout qualifier 'texture' is not permitted here
layout(vulkan, rgba32f, texture=0) writeonly texture2D wtexture2;  // invalid (no set/binding)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 14: storage textures must declare a pixel format
layout(vulkan, set=0, binding=0) readonly texture2D rtexture3;     // invalid (no pixformat)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 15: storage textures must declare a pixel format
layout(vulkan, set=0, binding=0) writeonly texture2D wtexture3;    // invalid (no pixformat)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 16: layout qualifier 'texture' is not permitted here
layout(vulkan, texture=0, sampler=0) sampler2D sampler2;           // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 16: layout qualifier 'sampler' is not permitted here
layout(vulkan, texture=0, sampler=0) sampler2D sampler2;           // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
7 errors
