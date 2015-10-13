AUTORCC
-------

Should the target be processed with autorcc (for Qt projects).

``AUTORCC`` is a boolean specifying whether CMake will handle
the Qt ``rcc`` code generator automatically, i.e. without having to use
the :module:`QT4_ADD_RESOURCES() <FindQt4>` or ``QT5_ADD_RESOURCES()``
macro.  Currently Qt4 and Qt5 are supported.

When this property is ``ON``, CMake will handle ``.qrc`` files added
as target sources at build time and invoke ``rcc`` accordingly.
This property is initialized by the value of the :variable:`CMAKE_AUTORCC`
variable if it is set when a target is created.

Additional command line options for rcc can be set via the
:prop_sf:`AUTORCC_OPTIONS` source file property on the ``.qrc`` file.

The global property :prop_gbl:`AUTOGEN_TARGETS_FOLDER` can be used to group
the autorcc targets together in an IDE, e.g. in MSVS.

See the :manual:`cmake-qt(7)` manual for more information on using CMake
with Qt.
