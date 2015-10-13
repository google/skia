include_external_msproject
--------------------------

Include an external Microsoft project file in a workspace.

::

  include_external_msproject(projectname location
                             [TYPE projectTypeGUID]
                             [GUID projectGUID]
                             [PLATFORM platformName]
                             dep1 dep2 ...)

Includes an external Microsoft project in the generated workspace
file.  Currently does nothing on UNIX.  This will create a target
named [projectname].  This can be used in the add_dependencies command
to make things depend on the external project.

TYPE, GUID and PLATFORM are optional parameters that allow one to
specify the type of project, id (GUID) of the project and the name of
the target platform.  This is useful for projects requiring values
other than the default (e.g.  WIX projects).  These options are not
supported by the Visual Studio 6 generator.
