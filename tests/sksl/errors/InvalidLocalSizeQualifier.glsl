### Compilation failed:

error: 1: local size layout qualifiers must be defined using an 'in' declaration
layout(local_size_x = 16, local_size_y = 16, local_size_z = 1);
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 2: local size layout qualifiers must be defined using an 'in' declaration
layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) out;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 3: local size layout qualifiers must be defined using an 'in' declaration
layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) inout;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 4: local size layout qualifiers must be defined using an 'in' declaration
layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) const;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 5: local size layout qualifiers must be defined using an 'in' declaration
layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) uniform;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 6: local size layout qualifiers must be defined using an 'in' declaration
layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) buffer;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 7: local size layout qualifiers must be defined using an 'in' declaration
layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) workgroup;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 8: local size layout qualifiers must be defined using an 'in' declaration
layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) highp;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 9: local size layout qualifiers must be defined using an 'in' declaration
layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) mediump;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 10: local size layout qualifiers must be defined using an 'in' declaration
layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) lowp;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 11: local size layout qualifiers must be defined using an 'in' declaration
layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) flat;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 12: local size layout qualifiers must be defined using an 'in' declaration
layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) noperspective;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 13: local size layout qualifiers must be defined using an 'in' declaration
layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) readonly;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 14: local size layout qualifiers must be defined using an 'in' declaration
layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) writeonly;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 16: local size layout qualifiers must be defined using an 'in' declaration
layout(local_size_x = 16) out;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 17: local size layout qualifiers must be defined using an 'in' declaration
layout(local_size_y = 16) out;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 18: local size layout qualifiers must be defined using an 'in' declaration
layout(local_size_z = 16) out;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 20: local size qualifiers cannot be zero
layout(local_size_x = 0) in;
^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 21: local size qualifiers cannot be zero
layout(local_size_y = 0) in;
^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 22: local size qualifiers cannot be zero
layout(local_size_z = 0) in;
^^^^^^^^^^^^^^^^^^^^^^^^^^^
20 errors
