MACOSX_BUNDLE
-------------

Build an executable as an application bundle on Mac OS X.

When this property is set to true the executable when built on Mac OS
X will be created as an application bundle.  This makes it a GUI
executable that can be launched from the Finder.  See the
MACOSX_BUNDLE_INFO_PLIST target property for information about
creation of the Info.plist file for the application bundle.  This
property is initialized by the value of the variable
CMAKE_MACOSX_BUNDLE if it is set when a target is created.
