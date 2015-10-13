set_target_properties
---------------------

Targets can have properties that affect how they are built.

::

  set_target_properties(target1 target2 ...
                        PROPERTIES prop1 value1
                        prop2 value2 ...)

Set properties on a target.  The syntax for the command is to list all
the files you want to change, and then provide the values you want to
set next.  You can use any prop value pair you want and extract it
later with the GET_TARGET_PROPERTY command.

Properties that affect the name of a target's output file are as
follows.  The PREFIX and SUFFIX properties override the default target
name prefix (such as "lib") and suffix (such as ".so").  IMPORT_PREFIX
and IMPORT_SUFFIX are the equivalent properties for the import library
corresponding to a DLL (for SHARED library targets).  OUTPUT_NAME sets
the real name of a target when it is built and can be used to help
create two targets of the same name even though CMake requires unique
logical target names.  There is also a <CONFIG>_OUTPUT_NAME that can
set the output name on a per-configuration basis.  <CONFIG>_POSTFIX
sets a postfix for the real name of the target when it is built under
the configuration named by <CONFIG> (in upper-case, such as
"DEBUG_POSTFIX").  The value of this property is initialized when the
target is created to the value of the variable CMAKE_<CONFIG>_POSTFIX
(except for executable targets because earlier CMake versions which
did not use this variable for executables).

The LINK_FLAGS property can be used to add extra flags to the link
step of a target.  LINK_FLAGS_<CONFIG> will add to the configuration
<CONFIG>, for example, DEBUG, RELEASE, MINSIZEREL, RELWITHDEBINFO.
DEFINE_SYMBOL sets the name of the preprocessor symbol defined when
compiling sources in a shared library.  If not set here then it is set
to target_EXPORTS by default (with some substitutions if the target is
not a valid C identifier).  This is useful for headers to know whether
they are being included from inside their library or outside to
properly setup dllexport/dllimport decorations.  The COMPILE_FLAGS
property sets additional compiler flags used to build sources within
the target.  It may also be used to pass additional preprocessor
definitions.

The LINKER_LANGUAGE property is used to change the tool used to link
an executable or shared library.  The default is set the language to
match the files in the library.  CXX and C are common values for this
property.

For shared libraries VERSION and SOVERSION can be used to specify the
build version and API version respectively.  When building or
installing appropriate symlinks are created if the platform supports
symlinks and the linker supports so-names.  If only one of both is
specified the missing is assumed to have the same version number.  For
executables VERSION can be used to specify the build version.  When
building or installing appropriate symlinks are created if the
platform supports symlinks.  For shared libraries and executables on
Windows the VERSION attribute is parsed to extract a "major.minor"
version number.  These numbers are used as the image version of the
binary.

There are a few properties used to specify RPATH rules.  INSTALL_RPATH
is a semicolon-separated list specifying the rpath to use in installed
targets (for platforms that support it).  INSTALL_RPATH_USE_LINK_PATH
is a boolean that if set to true will append directories in the linker
search path and outside the project to the INSTALL_RPATH.
SKIP_BUILD_RPATH is a boolean specifying whether to skip automatic
generation of an rpath allowing the target to run from the build tree.
BUILD_WITH_INSTALL_RPATH is a boolean specifying whether to link the
target in the build tree with the INSTALL_RPATH.  This takes
precedence over SKIP_BUILD_RPATH and avoids the need for relinking
before installation.  INSTALL_NAME_DIR is a string specifying the
directory portion of the "install_name" field of shared libraries on
Mac OSX to use in the installed targets.  When the target is created
the values of the variables CMAKE_INSTALL_RPATH,
CMAKE_INSTALL_RPATH_USE_LINK_PATH, CMAKE_SKIP_BUILD_RPATH,
CMAKE_BUILD_WITH_INSTALL_RPATH, and CMAKE_INSTALL_NAME_DIR are used to
initialize these properties.

PROJECT_LABEL can be used to change the name of the target in an IDE
like visual studio.  VS_KEYWORD can be set to change the visual studio
keyword, for example Qt integration works better if this is set to
Qt4VSv1.0.

VS_SCC_PROJECTNAME, VS_SCC_LOCALPATH, VS_SCC_PROVIDER and
VS_SCC_AUXPATH can be set to add support for source control bindings
in a Visual Studio project file.

VS_GLOBAL_<variable> can be set to add a Visual Studio
project-specific global variable.  Qt integration works better if
VS_GLOBAL_QtVersion is set to the Qt version FindQt4.cmake found.  For
example, "4.7.3"

The PRE_INSTALL_SCRIPT and POST_INSTALL_SCRIPT properties are the old
way to specify CMake scripts to run before and after installing a
target.  They are used only when the old INSTALL_TARGETS command is
used to install the target.  Use the INSTALL command instead.

The EXCLUDE_FROM_DEFAULT_BUILD property is used by the visual studio
generators.  If it is set to 1 the target will not be part of the
default build when you select "Build Solution".  This can also be set
on a per-configuration basis using
EXCLUDE_FROM_DEFAULT_BUILD_<CONFIG>.
