AUTOUIC
-------

Should the target be processed with autouic (for Qt projects).

``AUTOUIC`` is a boolean specifying whether CMake will handle
the Qt ``uic`` code generator automatically, i.e. without having to use
the :module:`QT4_WRAP_UI() <FindQt4>` or ``QT5_WRAP_UI()`` macro. Currently
Qt4 and Qt5 are supported.

When this property is ``ON``, CMake will scan the source files at build time
and invoke ``uic`` accordingly.  If an ``#include`` statement like
``#include "ui_foo.h"`` is found in ``foo.cpp``, a ``foo.ui`` file is
expected next to ``foo.cpp``, and ``uic`` is run on the ``foo.ui`` file.
This property is initialized by the value of the :variable:`CMAKE_AUTOUIC`
variable if it is set when a target is created.

Additional command line options for ``uic`` can be set via the
:prop_sf:`AUTOUIC_OPTIONS` source file property on the ``foo.ui`` file.
The global property :prop_gbl:`AUTOGEN_TARGETS_FOLDER` can be used to group the
autouic targets together in an IDE, e.g. in MSVS.

See the :manual:`cmake-qt(7)` manual for more information on using CMake
with Qt.
