RULE_LAUNCH_COMPILE
-------------------

Specify a launcher for compile rules.

Makefile generators prefix compiler commands with the given launcher
command line.  This is intended to allow launchers to intercept build
problems with high granularity.  Non-Makefile generators currently
ignore this property.
