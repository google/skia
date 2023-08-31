### Compilation failed:

error: 9: layout qualifier 'texture' is not permitted here
layout(vulkan, texture=0) readonly texture2D rtexture2;   // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 10: layout qualifier 'texture' is not permitted here
layout(vulkan, texture=0) writeonly texture2D wtexture2;  // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 11: layout qualifier 'texture' is not permitted here
layout(vulkan, texture=0, sampler=0) sampler2D sampler2;  // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 11: layout qualifier 'sampler' is not permitted here
layout(vulkan, texture=0, sampler=0) sampler2D sampler2;  // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
4 errors
