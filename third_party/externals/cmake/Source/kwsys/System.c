/*============================================================================
  KWSys - Kitware System Library
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "kwsysPrivate.h"
#include KWSYS_HEADER(System.h)

/* Work-around CMake dependency scanning limitation.  This must
   duplicate the above list of headers.  */
#if 0
# include "System.h.in"
#endif

#include <stddef.h> /* ptrdiff_t */
#include <stdlib.h> /* malloc, free */
#include <string.h> /* strlen */
#include <ctype.h>  /* isalpha */

#include <stdio.h>

#if defined(KWSYS_C_HAS_PTRDIFF_T) && KWSYS_C_HAS_PTRDIFF_T
typedef ptrdiff_t kwsysSystem_ptrdiff_t;
#else
typedef int kwsysSystem_ptrdiff_t;
#endif

/*

Notes:

Make variable replacements open a can of worms.  Sometimes they should
be quoted and sometimes not.  Sometimes their replacement values are
already quoted.

VS variables cause problems.  In order to pass the referenced value
with spaces the reference must be quoted.  If the variable value ends
in a backslash then it will escape the ending quote!  In order to make
the ending backslash appear we need this:

  "$(InputDir)\"

However if there is not a trailing backslash then this will put a
quote in the value so we need:

  "$(InputDir)"

Make variable references are platform specific so we should probably
just NOT quote them and let the listfile author deal with it.

*/

/*
TODO: For windows echo:

To display a pipe (|) or redirection character (< or >) when using the
echo command, use a caret character immediately before the pipe or
redirection character (for example, ^>, ^<, or ^| ). If you need to
use the caret character itself (^), use two in a row (^^).
*/

/*--------------------------------------------------------------------------*/
static int kwsysSystem_Shell__CharIsWhitespace(char c)
{
  return ((c == ' ') || (c == '\t'));
}

/*--------------------------------------------------------------------------*/
static int kwsysSystem_Shell__CharNeedsQuotesOnUnix(char c)
{
  return ((c == '\'') || (c == '`') || (c == ';') || (c == '#') ||
          (c == '&') || (c == '$') || (c == '(') || (c == ')') ||
          (c == '~') || (c == '<') || (c == '>') || (c == '|') ||
          (c == '*') || (c == '^') || (c == '\\'));
}

/*--------------------------------------------------------------------------*/
static int kwsysSystem_Shell__CharNeedsQuotesOnWindows(char c)
{
  return ((c == '\'') || (c == '#') || (c == '&') ||
          (c == '<') || (c == '>') || (c == '|') || (c == '^'));
}

/*--------------------------------------------------------------------------*/
static int kwsysSystem_Shell__CharNeedsQuotes(char c, int isUnix, int flags)
{
  /* On Windows the built-in command shell echo never needs quotes.  */
  if(!isUnix && (flags & kwsysSystem_Shell_Flag_EchoWindows))
    {
    return 0;
    }

  /* On all platforms quotes are needed to preserve whitespace.  */
  if(kwsysSystem_Shell__CharIsWhitespace(c))
    {
    return 1;
    }

  if(isUnix)
    {
    /* On UNIX several special characters need quotes to preserve them.  */
    if(kwsysSystem_Shell__CharNeedsQuotesOnUnix(c))
      {
      return 1;
      }
    }
  else
    {
    /* On Windows several special characters need quotes to preserve them.  */
    if(kwsysSystem_Shell__CharNeedsQuotesOnWindows(c))
      {
      return 1;
      }
    }
  return 0;
}

/*--------------------------------------------------------------------------*/
static int kwsysSystem_Shell__CharIsMakeVariableName(char c)
{
  return c && (c == '_' || isalpha(((int)c)));
}

/*--------------------------------------------------------------------------*/
static const char* kwsysSystem_Shell__SkipMakeVariables(const char* c)
{
  while(*c == '$' && *(c+1) == '(')
    {
    const char* skip = c+2;
    while(kwsysSystem_Shell__CharIsMakeVariableName(*skip))
      {
      ++skip;
      }
    if(*skip == ')')
      {
      c = skip+1;
      }
    else
      {
      break;
      }
    }
  return c;
}

/*
Allowing make variable replacements opens a can of worms.  Sometimes
they should be quoted and sometimes not.  Sometimes their replacement
values are already quoted or contain escapes.

Some Visual Studio variables cause problems.  In order to pass the
referenced value with spaces the reference must be quoted.  If the
variable value ends in a backslash then it will escape the ending
quote!  In order to make the ending backslash appear we need this:

  "$(InputDir)\"

However if there is not a trailing backslash then this will put a
quote in the value so we need:

  "$(InputDir)"

This macro decides whether we quote an argument just because it
contains a make variable reference.  This should be replaced with a
flag later when we understand applications of this better.
*/
#define KWSYS_SYSTEM_SHELL_QUOTE_MAKE_VARIABLES 0

/*--------------------------------------------------------------------------*/
static int kwsysSystem_Shell__ArgumentNeedsQuotes(const char* in, int isUnix,
                                                  int flags)
{
  /* The empty string needs quotes.  */
  if(!*in)
    {
    return 1;
    }

  /* Scan the string for characters that require quoting.  */
  {
  const char* c;
  for(c=in; *c; ++c)
    {
    /* Look for $(MAKEVAR) syntax if requested.  */
    if(flags & kwsysSystem_Shell_Flag_AllowMakeVariables)
      {
#if KWSYS_SYSTEM_SHELL_QUOTE_MAKE_VARIABLES
      const char* skip = kwsysSystem_Shell__SkipMakeVariables(c);
      if(skip != c)
        {
        /* We need to quote make variable references to preserve the
           string with contents substituted in its place.  */
        return 1;
        }
#else
      /* Skip over the make variable references if any are present.  */
      c = kwsysSystem_Shell__SkipMakeVariables(c);

      /* Stop if we have reached the end of the string.  */
      if(!*c)
        {
        break;
        }
#endif
      }

    /* Check whether this character needs quotes.  */
    if(kwsysSystem_Shell__CharNeedsQuotes(*c, isUnix, flags))
      {
      return 1;
      }
    }
  }

  /* On Windows some single character arguments need quotes.  */
  if(!isUnix && *in && !*(in+1))
    {
    char c = *in;
    if((c == '?') || (c == '&') || (c == '^') || (c == '|') || (c == '#'))
      {
      return 1;
      }
    }

  return 0;
}

/*--------------------------------------------------------------------------*/
static int kwsysSystem_Shell__GetArgumentSize(const char* in,
                                              int isUnix, int flags)
{
  /* Start with the length of the original argument, plus one for
     either a terminating null or a separating space.  */
  int size = (int)strlen(in) + 1;

  /* String iterator.  */
  const char* c;

  /* Keep track of how many backslashes have been encountered in a row.  */
  int windows_backslashes = 0;

  /* Scan the string for characters that require escaping or quoting.  */
  for(c=in; *c; ++c)
    {
    /* Look for $(MAKEVAR) syntax if requested.  */
    if(flags & kwsysSystem_Shell_Flag_AllowMakeVariables)
      {
      /* Skip over the make variable references if any are present.  */
      c = kwsysSystem_Shell__SkipMakeVariables(c);

      /* Stop if we have reached the end of the string.  */
      if(!*c)
        {
        break;
        }
      }

    /* Check whether this character needs escaping for the shell.  */
    if(isUnix)
      {
      /* On Unix a few special characters need escaping even inside a
         quoted argument.  */
      if(*c == '\\' || *c == '"' || *c == '`' || *c == '$')
        {
        /* This character needs a backslash to escape it.  */
        ++size;
        }
      }
    else if(flags & kwsysSystem_Shell_Flag_EchoWindows)
      {
      /* On Windows the built-in command shell echo never needs escaping.  */
      }
    else
      {
      /* On Windows only backslashes and double-quotes need escaping.  */
      if(*c == '\\')
        {
        /* Found a backslash.  It may need to be escaped later.  */
        ++windows_backslashes;
        }
      else if(*c == '"')
        {
        /* Found a double-quote.  We need to escape it and all
           immediately preceding backslashes.  */
        size += windows_backslashes + 1;
        windows_backslashes = 0;
        }
      else
        {
        /* Found another character.  This eliminates the possibility
           that any immediately preceding backslashes will be
           escaped.  */
        windows_backslashes = 0;
        }
      }

    /* Check whether this character needs escaping for a make tool.  */
    if(*c == '$')
      {
      if(flags & kwsysSystem_Shell_Flag_Make)
        {
        /* In Makefiles a dollar is written $$ so we need one extra
           character.  */
        ++size;
        }
      else if(flags & kwsysSystem_Shell_Flag_VSIDE)
        {
        /* In a VS IDE a dollar is written "$" so we need two extra
           characters.  */
        size += 2;
        }
      }
    else if(*c == '#')
      {
      if((flags & kwsysSystem_Shell_Flag_Make) &&
         (flags & kwsysSystem_Shell_Flag_WatcomWMake))
        {
        /* In Watcom WMake makefiles a pound is written $# so we need
           one extra character.  */
        ++size;
        }
      }
    else if(*c == '%')
      {
      if((flags & kwsysSystem_Shell_Flag_VSIDE) ||
         ((flags & kwsysSystem_Shell_Flag_Make) &&
          ((flags & kwsysSystem_Shell_Flag_MinGWMake) ||
           (flags & kwsysSystem_Shell_Flag_NMake))))
        {
        /* In the VS IDE, NMake, or MinGW make a percent is written %%
           so we need one extra characters.  */
        size += 1;
        }
      }
    else if(*c == ';')
      {
      if(flags & kwsysSystem_Shell_Flag_VSIDE)
        {
        /* In a VS IDE a semicolon is written ";" so we need two extra
           characters.  */
        size += 2;
        }
      }
    }

  /* Check whether the argument needs surrounding quotes.  */
  if(kwsysSystem_Shell__ArgumentNeedsQuotes(in, isUnix, flags))
    {
    /* Surrounding quotes are needed.  Allocate space for them.  */
    if((flags & kwsysSystem_Shell_Flag_WatcomQuote) && (isUnix))
      {
        size += 2;
      }
    size += 2;

    /* We must escape all ending backslashes when quoting on windows.  */
    size += windows_backslashes;
    }

  return size;
}

/*--------------------------------------------------------------------------*/
static char* kwsysSystem_Shell__GetArgument(const char* in, char* out,
                                            int isUnix, int flags)
{
  /* String iterator.  */
  const char* c;

  /* Keep track of how many backslashes have been encountered in a row.  */
  int windows_backslashes = 0;

  /* Whether the argument must be quoted.  */
  int needQuotes = kwsysSystem_Shell__ArgumentNeedsQuotes(in, isUnix, flags);
  if(needQuotes)
    {
    /* Add the opening quote for this argument.  */
    if(flags & kwsysSystem_Shell_Flag_WatcomQuote)
      {
      if(isUnix)
        {
        *out++ = '"';
        }
      *out++ = '\'';
      }
    else
      {
      *out++ = '"';
      }
    }

  /* Scan the string for characters that require escaping or quoting.  */
  for(c=in; *c; ++c)
    {
    /* Look for $(MAKEVAR) syntax if requested.  */
    if(flags & kwsysSystem_Shell_Flag_AllowMakeVariables)
      {
      const char* skip = kwsysSystem_Shell__SkipMakeVariables(c);
      if(skip != c)
        {
        /* Copy to the end of the make variable references.  */
        while(c != skip)
          {
          *out++ = *c++;
          }

        /* The make variable reference eliminates any escaping needed
           for preceding backslashes.  */
        windows_backslashes = 0;

        /* Stop if we have reached the end of the string.  */
        if(!*c)
          {
          break;
          }
        }
      }

    /* Check whether this character needs escaping for the shell.  */
    if(isUnix)
      {
      /* On Unix a few special characters need escaping even inside a
         quoted argument.  */
      if(*c == '\\' || *c == '"' || *c == '`' || *c == '$')
        {
        /* This character needs a backslash to escape it.  */
        *out++ = '\\';
        }
      }
    else if(flags & kwsysSystem_Shell_Flag_EchoWindows)
      {
      /* On Windows the built-in command shell echo never needs escaping.  */
      }
    else
      {
      /* On Windows only backslashes and double-quotes need escaping.  */
      if(*c == '\\')
        {
        /* Found a backslash.  It may need to be escaped later.  */
        ++windows_backslashes;
        }
      else if(*c == '"')
        {
        /* Found a double-quote.  Escape all immediately preceding
           backslashes.  */
        while(windows_backslashes > 0)
          {
          --windows_backslashes;
          *out++ = '\\';
          }

        /* Add the backslash to escape the double-quote.  */
        *out++ = '\\';
        }
      else
        {
        /* We encountered a normal character.  This eliminates any
           escaping needed for preceding backslashes.  */
        windows_backslashes = 0;
        }
      }

    /* Check whether this character needs escaping for a make tool.  */
    if(*c == '$')
      {
      if(flags & kwsysSystem_Shell_Flag_Make)
        {
        /* In Makefiles a dollar is written $$.  The make tool will
           replace it with just $ before passing it to the shell.  */
        *out++ = '$';
        *out++ = '$';
        }
      else if(flags & kwsysSystem_Shell_Flag_VSIDE)
        {
        /* In a VS IDE a dollar is written "$".  If this is written in
           an un-quoted argument it starts a quoted segment, inserts
           the $ and ends the segment.  If it is written in a quoted
           argument it ends quoting, inserts the $ and restarts
           quoting.  Either way the $ is isolated from surrounding
           text to avoid looking like a variable reference.  */
        *out++ = '"';
        *out++ = '$';
        *out++ = '"';
        }
      else
        {
        /* Otherwise a dollar is written just $. */
        *out++ = '$';
        }
      }
    else if(*c == '#')
      {
      if((flags & kwsysSystem_Shell_Flag_Make) &&
         (flags & kwsysSystem_Shell_Flag_WatcomWMake))
        {
        /* In Watcom WMake makefiles a pound is written $#.  The make
           tool will replace it with just # before passing it to the
           shell.  */
        *out++ = '$';
        *out++ = '#';
        }
      else
        {
        /* Otherwise a pound is written just #. */
        *out++ = '#';
        }
      }
    else if(*c == '%')
      {
      if((flags & kwsysSystem_Shell_Flag_VSIDE) ||
         ((flags & kwsysSystem_Shell_Flag_Make) &&
          ((flags & kwsysSystem_Shell_Flag_MinGWMake) ||
           (flags & kwsysSystem_Shell_Flag_NMake))))
        {
        /* In the VS IDE, NMake, or MinGW make a percent is written %%.  */
        *out++ = '%';
        *out++ = '%';
        }
      else
        {
        /* Otherwise a percent is written just %. */
        *out++ = '%';
        }
      }
    else if(*c == ';')
      {
      if(flags & kwsysSystem_Shell_Flag_VSIDE)
        {
        /* In a VS IDE a semicolon is written ";".  If this is written
           in an un-quoted argument it starts a quoted segment,
           inserts the ; and ends the segment.  If it is written in a
           quoted argument it ends quoting, inserts the ; and restarts
           quoting.  Either way the ; is isolated.  */
        *out++ = '"';
        *out++ = ';';
        *out++ = '"';
        }
      else
        {
        /* Otherwise a semicolon is written just ;. */
        *out++ = ';';
        }
      }
    else
      {
      /* Store this character.  */
      *out++ = *c;
      }
    }

  if(needQuotes)
    {
    /* Add enough backslashes to escape any trailing ones.  */
    while(windows_backslashes > 0)
      {
      --windows_backslashes;
      *out++ = '\\';
      }

    /* Add the closing quote for this argument.  */
    if(flags & kwsysSystem_Shell_Flag_WatcomQuote)
      {
      *out++ = '\'';
      if(isUnix)
        {
        *out++ = '"';
        }
      }
    else
      {
      *out++ = '"';
      }
    }

  /* Store a terminating null without incrementing.  */
  *out = 0;

  return out;
}

/*--------------------------------------------------------------------------*/
char* kwsysSystem_Shell_GetArgumentForWindows(const char* in,
                                              char* out,
                                              int flags)
{
  return kwsysSystem_Shell__GetArgument(in, out, 0, flags);
}

/*--------------------------------------------------------------------------*/
char* kwsysSystem_Shell_GetArgumentForUnix(const char* in,
                                           char* out,
                                           int flags)
{
  return kwsysSystem_Shell__GetArgument(in, out, 1, flags);
}

/*--------------------------------------------------------------------------*/
int kwsysSystem_Shell_GetArgumentSizeForWindows(const char* in, int flags)
{
  return kwsysSystem_Shell__GetArgumentSize(in, 0, flags);
}

/*--------------------------------------------------------------------------*/
int kwsysSystem_Shell_GetArgumentSizeForUnix(const char* in, int flags)
{
  return kwsysSystem_Shell__GetArgumentSize(in, 1, flags);
}

/*--------------------------------------------------------------------------*/
static int kwsysSystem__AppendByte(char* local,
                                   char** begin, char** end,
                                   int* size, char c)
{
  /* Allocate space for the character.  */
  if((*end - *begin) >= *size)
    {
    kwsysSystem_ptrdiff_t length = *end - *begin;
    char* newBuffer = (char*)malloc((size_t)(*size*2));
    if(!newBuffer)
      {
      return 0;
      }
    memcpy(newBuffer, *begin, (size_t)(length)*sizeof(char));
    if(*begin != local)
      {
      free(*begin);
      }
    *begin = newBuffer;
    *end = *begin + length;
    *size *= 2;
    }

  /* Store the character.  */
  *(*end)++ = c;
  return 1;
}

/*--------------------------------------------------------------------------*/
static int kwsysSystem__AppendArgument(char** local,
                                       char*** begin, char*** end,
                                       int* size,
                                       char* arg_local,
                                       char** arg_begin, char** arg_end,
                                       int* arg_size)
{
  /* Append a null-terminator to the argument string.  */
  if(!kwsysSystem__AppendByte(arg_local, arg_begin, arg_end, arg_size, '\0'))
    {
    return 0;
    }

  /* Allocate space for the argument pointer.  */
  if((*end - *begin) >= *size)
    {
    kwsysSystem_ptrdiff_t length = *end - *begin;
    char** newPointers = (char**)malloc((size_t)(*size)*2*sizeof(char*));
    if(!newPointers)
      {
      return 0;
      }
    memcpy(newPointers, *begin, (size_t)(length)*sizeof(char*));
    if(*begin != local)
      {
      free(*begin);
      }
    *begin = newPointers;
    *end = *begin + length;
    *size *= 2;
    }

  /* Allocate space for the argument string.  */
  **end = (char*)malloc((size_t)(*arg_end - *arg_begin));
  if(!**end)
    {
    return 0;
    }

  /* Store the argument in the command array.  */
  memcpy(**end, *arg_begin,(size_t)(*arg_end - *arg_begin));
  ++(*end);

  /* Reset the argument to be empty.  */
  *arg_end = *arg_begin;

  return 1;
}

/*--------------------------------------------------------------------------*/
#define KWSYSPE_LOCAL_BYTE_COUNT 1024
#define KWSYSPE_LOCAL_ARGS_COUNT 32
static char** kwsysSystem__ParseUnixCommand(const char* command, int flags)
{
  /* Create a buffer for argument pointers during parsing.  */
  char* local_pointers[KWSYSPE_LOCAL_ARGS_COUNT];
  int pointers_size = KWSYSPE_LOCAL_ARGS_COUNT;
  char** pointer_begin = local_pointers;
  char** pointer_end = pointer_begin;

  /* Create a buffer for argument strings during parsing.  */
  char local_buffer[KWSYSPE_LOCAL_BYTE_COUNT];
  int buffer_size = KWSYSPE_LOCAL_BYTE_COUNT;
  char* buffer_begin = local_buffer;
  char* buffer_end = buffer_begin;

  /* Parse the command string.  Try to behave like a UNIX shell.  */
  char** newCommand = 0;
  const char* c = command;
  int in_argument = 0;
  int in_escape = 0;
  int in_single = 0;
  int in_double = 0;
  int failed = 0;
  for(;*c; ++c)
    {
    if(in_escape)
      {
      /* This character is escaped so do no special handling.  */
      if(!in_argument)
        {
        in_argument = 1;
        }
      if(!kwsysSystem__AppendByte(local_buffer, &buffer_begin,
                                  &buffer_end, &buffer_size, *c))
        {
        failed = 1;
        break;
        }
      in_escape = 0;
      }
    else if(*c == '\\')
      {
      /* The next character should be escaped.  */
      in_escape = 1;
      }
    else if(*c == '\'' && !in_double)
      {
      /* Enter or exit single-quote state.  */
      if(in_single)
        {
        in_single = 0;
        }
      else
        {
        in_single = 1;
        if(!in_argument)
          {
          in_argument = 1;
          }
        }
      }
    else if(*c == '"' && !in_single)
      {
      /* Enter or exit double-quote state.  */
      if(in_double)
        {
        in_double = 0;
        }
      else
        {
        in_double = 1;
        if(!in_argument)
          {
          in_argument = 1;
          }
        }
      }
    else if(isspace((unsigned char) *c))
      {
      if(in_argument)
        {
        if(in_single || in_double)
          {
          /* This space belongs to a quoted argument.  */
          if(!kwsysSystem__AppendByte(local_buffer, &buffer_begin,
                                      &buffer_end, &buffer_size, *c))
            {
            failed = 1;
            break;
            }
          }
        else
          {
          /* This argument has been terminated by whitespace.  */
          if(!kwsysSystem__AppendArgument(local_pointers, &pointer_begin,
                                          &pointer_end, &pointers_size,
                                          local_buffer, &buffer_begin,
                                          &buffer_end, &buffer_size))
            {
            failed = 1;
            break;
            }
          in_argument = 0;
          }
        }
      }
    else
      {
      /* This character belong to an argument.  */
      if(!in_argument)
        {
        in_argument = 1;
        }
      if(!kwsysSystem__AppendByte(local_buffer, &buffer_begin,
                                  &buffer_end, &buffer_size, *c))
        {
        failed = 1;
        break;
        }
      }
    }

  /* Finish the last argument.  */
  if(in_argument)
    {
    if(!kwsysSystem__AppendArgument(local_pointers, &pointer_begin,
                                    &pointer_end, &pointers_size,
                                    local_buffer, &buffer_begin,
                                    &buffer_end, &buffer_size))
      {
      failed = 1;
      }
    }

  /* If we still have memory allocate space for the new command
     buffer.  */
  if(!failed)
    {
    kwsysSystem_ptrdiff_t n = pointer_end - pointer_begin;
    newCommand = (char**)malloc((size_t)(n+1)*sizeof(char*));
    }

  if(newCommand)
    {
    /* Copy the arguments into the new command buffer.  */
    kwsysSystem_ptrdiff_t n = pointer_end - pointer_begin;
    memcpy(newCommand, pointer_begin, sizeof(char*)*(size_t)(n));
    newCommand[n] = 0;
    }
  else
    {
    /* Free arguments already allocated.  */
    while(pointer_end != pointer_begin)
      {
      free(*(--pointer_end));
      }
    }

  /* Free temporary buffers.  */
  if(pointer_begin != local_pointers)
    {
    free(pointer_begin);
    }
  if(buffer_begin != local_buffer)
    {
    free(buffer_begin);
    }

  /* The flags argument is currently unused.  */
  (void)flags;

  /* Return the final command buffer.  */
  return newCommand;
}

/*--------------------------------------------------------------------------*/
char** kwsysSystem_Parse_CommandForUnix(const char* command, int flags)
{
  /* Validate the flags.  */
  if(flags != 0)
    {
    return 0;
    }

  /* Forward to our internal implementation.  */
  return kwsysSystem__ParseUnixCommand(command, flags);
}
