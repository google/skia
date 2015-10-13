separate_arguments
------------------

Parse space-separated arguments into a semicolon-separated list.

::

  separate_arguments(<var> <UNIX|WINDOWS>_COMMAND "<args>")

Parses a unix- or windows-style command-line string "<args>" and
stores a semicolon-separated list of the arguments in <var>.  The
entire command line must be given in one "<args>" argument.

The UNIX_COMMAND mode separates arguments by unquoted whitespace.  It
recognizes both single-quote and double-quote pairs.  A backslash
escapes the next literal character (\" is "); there are no special
escapes (\n is just n).

The WINDOWS_COMMAND mode parses a windows command-line using the same
syntax the runtime library uses to construct argv at startup.  It
separates arguments by whitespace that is not double-quoted.
Backslashes are literal unless they precede double-quotes.  See the
MSDN article "Parsing C Command-Line Arguments" for details.

::

  separate_arguments(VARIABLE)

Convert the value of VARIABLE to a semi-colon separated list.  All
spaces are replaced with ';'.  This helps with generating command
lines.
