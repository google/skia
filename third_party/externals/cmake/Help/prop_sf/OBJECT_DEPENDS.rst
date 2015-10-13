OBJECT_DEPENDS
--------------

Additional files on which a compiled object file depends.

Specifies a semicolon-separated list of full-paths to files on which
any object files compiled from this source file depend.  An object
file will be recompiled if any of the named files is newer than it.

This property need not be used to specify the dependency of a source
file on a generated header file that it includes.  Although the
property was originally introduced for this purpose, it is no longer
necessary.  If the generated header file is created by a custom
command in the same target as the source file, the automatic
dependency scanning process will recognize the dependency.  If the
generated header file is created by another target, an inter-target
dependency should be created with the add_dependencies command (if one
does not already exist due to linking relationships).
