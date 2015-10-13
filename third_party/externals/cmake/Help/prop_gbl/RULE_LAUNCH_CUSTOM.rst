RULE_LAUNCH_CUSTOM
------------------

Specify a launcher for custom rules.

Makefile generators prefix custom commands with the given launcher
command line.  This is intended to allow launchers to intercept build
problems with high granularity.  Non-Makefile generators currently
ignore this property.
