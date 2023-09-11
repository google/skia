### Compilation failed:

error: 14: layout qualifier 'sampler' is not permitted here
layout(metal, rgba32f, sampler=0) readonly texture2D rtexture3;            // invalid (has sampler)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 15: layout qualifier 'sampler' is not permitted here
layout(metal, rgba32f, sampler=0) writeonly texture2D wtexture3;           // invalid (has sampler)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 16: layout qualifier 'sampler' is not permitted here
layout(metal, rgba32f, texture=0, sampler=0) readonly texture2D rtexture4; // invalid (has sampler)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 17: layout qualifier 'sampler' is not permitted here
layout(metal, rgba32f, texture=0, sampler=0) writeonly texture2D wtexture4;// invalid (has sampler)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 18: storage textures must declare a pixel format
layout(metal, texture=0) readonly texture2D rtexture5;                     // invalid (no pixformat)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 19: storage textures must declare a pixel format
layout(metal, texture=0) writeonly texture2D wtexture5;                    // invalid (no pixformat)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 20: 'binding' modifier cannot coexist with 'texture'/'sampler'
layout(metal, binding=0, texture=0, sampler=0) sampler2D sampler3;         // invalid (has binding)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 21: layout qualifier 'texture' is not permitted here
layout(metal, texture=0, sampler=0) ubo2 { float c; };                     // invalid (has tex/samp)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 21: layout qualifier 'sampler' is not permitted here
layout(metal, texture=0, sampler=0) ubo2 { float c; };                     // invalid (has tex/samp)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 22: layout qualifier 'set' is not permitted here
layout(metal, set=0, binding=0) ubo3 { float d; };                         // invalid (has set)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
10 errors
