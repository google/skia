get_source_file_property
------------------------

Get a property for a source file.

::

  get_source_file_property(VAR file property)

Get a property from a source file.  The value of the property is
stored in the variable VAR.  If the property is not found, VAR will be
set to "NOTFOUND".  Use set_source_files_properties to set property
values.  Source file properties usually control how the file is built.
One property that is always there is LOCATION

See also the more general get_property() command.
