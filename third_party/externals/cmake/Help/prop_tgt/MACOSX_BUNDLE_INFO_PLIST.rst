MACOSX_BUNDLE_INFO_PLIST
------------------------

Specify a custom Info.plist template for a Mac OS X App Bundle.

An executable target with MACOSX_BUNDLE enabled will be built as an
application bundle on Mac OS X.  By default its Info.plist file is
created by configuring a template called MacOSXBundleInfo.plist.in
located in the CMAKE_MODULE_PATH.  This property specifies an
alternative template file name which may be a full path.

The following target properties may be set to specify content to be
configured into the file:

::

  MACOSX_BUNDLE_INFO_STRING
  MACOSX_BUNDLE_ICON_FILE
  MACOSX_BUNDLE_GUI_IDENTIFIER
  MACOSX_BUNDLE_LONG_VERSION_STRING
  MACOSX_BUNDLE_BUNDLE_NAME
  MACOSX_BUNDLE_SHORT_VERSION_STRING
  MACOSX_BUNDLE_BUNDLE_VERSION
  MACOSX_BUNDLE_COPYRIGHT

CMake variables of the same name may be set to affect all targets in a
directory that do not have each specific property set.  If a custom
Info.plist is specified by this property it may of course hard-code
all the settings instead of using the target properties.
