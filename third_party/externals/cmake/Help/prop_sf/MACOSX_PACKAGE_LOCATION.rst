MACOSX_PACKAGE_LOCATION
-----------------------

Place a source file inside a Mac OS X bundle, CFBundle, or framework.

Executable targets with the MACOSX_BUNDLE property set are built as
Mac OS X application bundles on Apple platforms.  Shared library
targets with the FRAMEWORK property set are built as Mac OS X
frameworks on Apple platforms.  Module library targets with the BUNDLE
property set are built as Mac OS X CFBundle bundles on Apple
platforms.  Source files listed in the target with this property set
will be copied to a directory inside the bundle or framework content
folder specified by the property value.  For bundles the content
folder is "<name>.app/Contents".  For frameworks the content folder is
"<name>.framework/Versions/<version>".  For cfbundles the content
folder is "<name>.bundle/Contents" (unless the extension is changed).
See the PUBLIC_HEADER, PRIVATE_HEADER, and RESOURCE target properties
for specifying files meant for Headers, PrivateHeaders, or Resources
directories.
