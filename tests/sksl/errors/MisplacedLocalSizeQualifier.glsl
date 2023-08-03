### Compilation failed:

error: 1: local size layout qualifiers are only allowed in a compute program
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 2: local size layout qualifiers are only allowed in a compute program
layout(local_size_x = 1) in;
^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 3: local size layout qualifiers are only allowed in a compute program
layout(local_size_y = 1) in;
^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 4: local size layout qualifiers are only allowed in a compute program
layout(local_size_z = 1) in;
^^^^^^^^^^^^^^^^^^^^^^^^^^^
4 errors
