RULE_LAUNCH_LINK
----------------

Specify a launcher for link rules.

Makefile generators prefix link and archive commands with the given
launcher command line.  This is intended to allow launchers to
intercept build problems with high granularity.  Non-Makefile
generators currently ignore this property.
