MACOSX_FRAMEWORK_INFO_PLIST
---------------------------

Specify a custom Info.plist template for a Mac OS X Framework.

A library target with FRAMEWORK enabled will be built as a framework
on Mac OS X.  By default its Info.plist file is created by configuring
a template called MacOSXFrameworkInfo.plist.in located in the
CMAKE_MODULE_PATH.  This property specifies an alternative template
file name which may be a full path.

The following target properties may be set to specify content to be
configured into the file:

::

  MACOSX_FRAMEWORK_ICON_FILE
  MACOSX_FRAMEWORK_IDENTIFIER
  MACOSX_FRAMEWORK_SHORT_VERSION_STRING
  MACOSX_FRAMEWORK_BUNDLE_VERSION

CMake variables of the same name may be set to affect all targets in a
directory that do not have each specific property set.  If a custom
Info.plist is specified by this property it may of course hard-code
all the settings instead of using the target properties.
