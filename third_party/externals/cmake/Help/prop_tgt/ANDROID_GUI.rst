ANDROID_GUI
-----------

Build an executable as an application package on Android.

When this property is set to true the executable when built for Android
will be created as an application package.  This property is initialized
by the value of the :variable:`CMAKE_ANDROID_GUI` variable if it is set
when a target is created.

Add the ``AndroidManifest.xml`` source file explicitly to the
target :command:`add_executable` command invocation to specify the
root directory of the application package source.
