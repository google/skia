### Compilation failed:

error: 2: 'local_size_x' was specified more than once
layout(local_size_x = 64) in;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 4: 'local_size_y' was specified more than once
layout(local_size_y = 64) in;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 6: 'local_size_z' was specified more than once
layout(local_size_z = 1) in;
^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 7: 'local_size_x' was specified more than once
layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 7: 'local_size_y' was specified more than once
layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 7: 'local_size_z' was specified more than once
layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
6 errors
