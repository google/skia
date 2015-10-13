string
------

String operations.

::

  string(REGEX MATCH <regular_expression>
         <output variable> <input> [<input>...])
  string(REGEX MATCHALL <regular_expression>
         <output variable> <input> [<input>...])
  string(REGEX REPLACE <regular_expression>
         <replace_expression> <output variable>
         <input> [<input>...])
  string(REPLACE <match_string>
         <replace_string> <output variable>
         <input> [<input>...])
  string(CONCAT <output variable> [<input>...])
  string(<MD5|SHA1|SHA224|SHA256|SHA384|SHA512>
         <output variable> <input>)
  string(COMPARE EQUAL <string1> <string2> <output variable>)
  string(COMPARE NOTEQUAL <string1> <string2> <output variable>)
  string(COMPARE LESS <string1> <string2> <output variable>)
  string(COMPARE GREATER <string1> <string2> <output variable>)
  string(ASCII <number> [<number> ...] <output variable>)
  string(CONFIGURE <string1> <output variable>
         [@ONLY] [ESCAPE_QUOTES])
  string(TOUPPER <string1> <output variable>)
  string(TOLOWER <string1> <output variable>)
  string(LENGTH <string> <output variable>)
  string(SUBSTRING <string> <begin> <length> <output variable>)
  string(STRIP <string> <output variable>)
  string(RANDOM [LENGTH <length>] [ALPHABET <alphabet>]
         [RANDOM_SEED <seed>] <output variable>)
  string(FIND <string> <substring> <output variable> [REVERSE])
  string(TIMESTAMP <output variable> [<format string>] [UTC])
  string(MAKE_C_IDENTIFIER <input string> <output variable>)
  string(GENEX_STRIP <input string> <output variable>)
  string(UUID <output variable> NAMESPACE <namespace> NAME <name>
         TYPE <MD5|SHA1> [UPPER])

REGEX MATCH will match the regular expression once and store the match
in the output variable.

REGEX MATCHALL will match the regular expression as many times as
possible and store the matches in the output variable as a list.

REGEX REPLACE will match the regular expression as many times as
possible and substitute the replacement expression for the match in
the output.  The replace expression may refer to paren-delimited
subexpressions of the match using \1, \2, ..., \9.  Note that two
backslashes (\\1) are required in CMake code to get a backslash
through argument parsing.

REPLACE will replace all occurrences of match_string in the input with
replace_string and store the result in the output.

CONCAT will concatenate all the input arguments together and store
the result in the named output variable.

MD5, SHA1, SHA224, SHA256, SHA384, and SHA512 will compute a
cryptographic hash of the input string.

COMPARE EQUAL/NOTEQUAL/LESS/GREATER will compare the strings and store
true or false in the output variable.

ASCII will convert all numbers into corresponding ASCII characters.

CONFIGURE will transform a string like CONFIGURE_FILE transforms a
file.

TOUPPER/TOLOWER will convert string to upper/lower characters.

LENGTH will return a given string's length.

SUBSTRING will return a substring of a given string. If length is -1
the remainder of the string starting at begin will be returned.
If string is shorter than length then end of string is used instead.

.. note::
  CMake 3.1 and below reported an error if length pointed past
  the end of string.

STRIP will return a substring of a given string with leading and
trailing spaces removed.

RANDOM will return a random string of given length consisting of
characters from the given alphabet.  Default length is 5 characters
and default alphabet is all numbers and upper and lower case letters.
If an integer RANDOM_SEED is given, its value will be used to seed the
random number generator.

FIND will return the position where the given substring was found in
the supplied string.  If the REVERSE flag was used, the command will
search for the position of the last occurrence of the specified
substring.

The following characters have special meaning in regular expressions:

::

   ^         Matches at beginning of input
   $         Matches at end of input
   .         Matches any single character
   [ ]       Matches any character(s) inside the brackets
   [^ ]      Matches any character(s) not inside the brackets
    -        Inside brackets, specifies an inclusive range between
             characters on either side e.g. [a-f] is [abcdef]
             To match a literal - using brackets, make it the first
             or the last character e.g. [+*/-] matches basic
             mathematical operators.
   *         Matches preceding pattern zero or more times
   +         Matches preceding pattern one or more times
   ?         Matches preceding pattern zero or once only
   |         Matches a pattern on either side of the |
   ()        Saves a matched subexpression, which can be referenced
             in the REGEX REPLACE operation. Additionally it is saved
             by all regular expression-related commands, including
             e.g. if( MATCHES ), in the variables CMAKE_MATCH_(0..9).

``*``, ``+`` and ``?`` have higher precedence than concatenation.  | has lower
precedence than concatenation.  This means that the regular expression
"^ab+d$" matches "abbd" but not "ababd", and the regular expression
"^(ab|cd)$" matches "ab" but not "abd".

TIMESTAMP will write a string representation of the current date
and/or time to the output variable.

Should the command be unable to obtain a timestamp the output variable
will be set to the empty string "".

The optional UTC flag requests the current date/time representation to
be in Coordinated Universal Time (UTC) rather than local time.

The optional <format string> may contain the following format
specifiers:

::

   %d        The day of the current month (01-31).
   %H        The hour on a 24-hour clock (00-23).
   %I        The hour on a 12-hour clock (01-12).
   %j        The day of the current year (001-366).
   %m        The month of the current year (01-12).
   %M        The minute of the current hour (00-59).
   %S        The second of the current minute.
             60 represents a leap second. (00-60)
   %U        The week number of the current year (00-53).
   %w        The day of the current week. 0 is Sunday. (0-6)
   %y        The last two digits of the current year (00-99)
   %Y        The current year.

Unknown format specifiers will be ignored and copied to the output
as-is.

If no explicit <format string> is given it will default to:

::

   %Y-%m-%dT%H:%M:%S    for local time.
   %Y-%m-%dT%H:%M:%SZ   for UTC.

MAKE_C_IDENTIFIER will write a string which can be used as an
identifier in C.

``GENEX_STRIP`` will strip any
:manual:`generator expressions <cmake-generator-expressions(7)>` from the
``input string`` and store the result in the ``output variable``.

UUID creates a univerally unique identifier (aka GUID) as per RFC4122
based on the hash of the combined values of <namespace>
(which itself has to be a valid UUID) and <name>.
The hash algorithm can be either ``MD5`` (Version 3 UUID) or
``SHA1`` (Version 5 UUID).
A UUID has the format ``xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx``
where each `x` represents a lower case hexadecimal character.
Where required an uppercase representation can be requested
with the optional ``UPPER`` flag.
