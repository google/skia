RESOURCE
--------

Specify resource files in a FRAMEWORK shared library target.

Shared library targets marked with the FRAMEWORK property generate
frameworks on OS X and normal shared libraries on other platforms.
This property may be set to a list of files to be placed in the
Resources directory inside the framework folder.  On non-Apple
platforms these files may be installed using the RESOURCE option to
the install(TARGETS) command.
