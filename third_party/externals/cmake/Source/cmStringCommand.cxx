/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmStringCommand.h"
#include "cmCryptoHash.h"

#include <cmsys/RegularExpression.hxx>
#include <cmsys/SystemTools.hxx>

#include <stdlib.h> // required for atoi
#include <ctype.h>
#include <time.h>

#include <cmTimestamp.h>
#include <cmUuid.h>

//----------------------------------------------------------------------------
bool cmStringCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if(args.size() < 1)
    {
    this->SetError("must be called with at least one argument.");
    return false;
    }

  const std::string &subCommand = args[0];
  if(subCommand == "REGEX")
    {
    return this->HandleRegexCommand(args);
    }
  else if(subCommand == "REPLACE")
    {
    return this->HandleReplaceCommand(args);
    }
  else if ( subCommand == "MD5" ||
            subCommand == "SHA1" ||
            subCommand == "SHA224" ||
            subCommand == "SHA256" ||
            subCommand == "SHA384" ||
            subCommand == "SHA512" )
    {
    return this->HandleHashCommand(args);
    }
  else if(subCommand == "TOLOWER")
    {
    return this->HandleToUpperLowerCommand(args, false);
    }
  else if(subCommand == "TOUPPER")
    {
    return this->HandleToUpperLowerCommand(args, true);
    }
  else if(subCommand == "COMPARE")
    {
    return this->HandleCompareCommand(args);
    }
  else if(subCommand == "ASCII")
    {
    return this->HandleAsciiCommand(args);
    }
  else if(subCommand == "CONFIGURE")
    {
    return this->HandleConfigureCommand(args);
    }
  else if(subCommand == "LENGTH")
    {
    return this->HandleLengthCommand(args);
    }
  else if(subCommand == "CONCAT")
    {
    return this->HandleConcatCommand(args);
    }
  else if(subCommand == "SUBSTRING")
    {
    return this->HandleSubstringCommand(args);
    }
  else if(subCommand == "STRIP")
    {
    return this->HandleStripCommand(args);
    }
  else if(subCommand == "RANDOM")
    {
    return this->HandleRandomCommand(args);
    }
  else if(subCommand == "FIND")
    {
    return this->HandleFindCommand(args);
    }
  else if(subCommand == "TIMESTAMP")
    {
    return this->HandleTimestampCommand(args);
    }
  else if(subCommand == "MAKE_C_IDENTIFIER")
    {
    return this->HandleMakeCIdentifierCommand(args);
    }
  else if(subCommand == "GENEX_STRIP")
    {
    return this->HandleGenexStripCommand(args);
    }
  else if(subCommand == "UUID")
    {
    return this->HandleUuidCommand(args);
    }

  std::string e = "does not recognize sub-command "+subCommand;
  this->SetError(e);
  return false;
}

//----------------------------------------------------------------------------
bool cmStringCommand::HandleHashCommand(std::vector<std::string> const& args)
{
#if defined(CMAKE_BUILD_WITH_CMAKE)
  if(args.size() != 3)
    {
    std::ostringstream e;
    e << args[0] << " requires an output variable and an input string";
    this->SetError(e.str());
    return false;
    }

  cmsys::auto_ptr<cmCryptoHash> hash(cmCryptoHash::New(args[0].c_str()));
  if(hash.get())
    {
    std::string out = hash->HashString(args[2]);
    this->Makefile->AddDefinition(args[1], out.c_str());
    return true;
    }
  return false;
#else
  std::ostringstream e;
  e << args[0] << " not available during bootstrap";
  this->SetError(e.str().c_str());
  return false;
#endif
}

//----------------------------------------------------------------------------
bool cmStringCommand::HandleToUpperLowerCommand(
  std::vector<std::string> const& args, bool toUpper)
{
  if ( args.size() < 3 )
    {
    this->SetError("no output variable specified");
    return false;
    }

  std::string outvar = args[2];
  std::string output;

  if (toUpper)
    {
    output = cmSystemTools::UpperCase(args[1]);
    }
  else
    {
    output = cmSystemTools::LowerCase(args[1]);
    }

  // Store the output in the provided variable.
  this->Makefile->AddDefinition(outvar, output.c_str());
  return true;
}

//----------------------------------------------------------------------------
bool cmStringCommand::HandleAsciiCommand(std::vector<std::string> const& args)
{
  if ( args.size() < 3 )
    {
    this->SetError("No output variable specified");
    return false;
    }
  std::string::size_type cc;
  std::string outvar = args[args.size()-1];
  std::string output = "";
  for ( cc = 1; cc < args.size()-1; cc ++ )
    {
    int ch = atoi(args[cc].c_str());
    if ( ch > 0 && ch < 256 )
      {
      output += static_cast<char>(ch);
      }
    else
      {
      std::string error = "Character with code ";
      error += args[cc];
      error += " does not exist.";
      this->SetError(error);
      return false;
      }
    }
  // Store the output in the provided variable.
  this->Makefile->AddDefinition(outvar, output.c_str());
  return true;
}

//----------------------------------------------------------------------------
bool cmStringCommand::HandleConfigureCommand(
  std::vector<std::string> const& args)
{
  if ( args.size() < 2 )
    {
    this->SetError("No input string specified.");
    return false;
    }
  else if ( args.size() < 3 )
    {
    this->SetError("No output variable specified.");
    return false;
    }

  // Parse options.
  bool escapeQuotes = false;
  bool atOnly = false;
  for(unsigned int i = 3; i < args.size(); ++i)
    {
    if(args[i] == "@ONLY")
      {
      atOnly = true;
      }
    else if(args[i] == "ESCAPE_QUOTES")
      {
      escapeQuotes = true;
      }
    else
      {
      std::ostringstream err;
      err << "Unrecognized argument \"" << args[i] << "\"";
      this->SetError(err.str());
      return false;
      }
    }

  // Configure the string.
  std::string output;
  this->Makefile->ConfigureString(args[1], output, atOnly, escapeQuotes);

  // Store the output in the provided variable.
  this->Makefile->AddDefinition(args[2], output.c_str());

  return true;
}

//----------------------------------------------------------------------------
bool cmStringCommand::HandleRegexCommand(std::vector<std::string> const& args)
{
  if(args.size() < 2)
    {
    this->SetError("sub-command REGEX requires a mode to be specified.");
    return false;
    }
  std::string mode = args[1];
  if(mode == "MATCH")
    {
    if(args.size() < 5)
      {
      this->SetError("sub-command REGEX, mode MATCH needs "
                     "at least 5 arguments total to command.");
      return false;
      }
    return this->RegexMatch(args);
    }
  else if(mode == "MATCHALL")
    {
    if(args.size() < 5)
      {
      this->SetError("sub-command REGEX, mode MATCHALL needs "
                     "at least 5 arguments total to command.");
      return false;
      }
    return this->RegexMatchAll(args);
    }
  else if(mode == "REPLACE")
    {
    if(args.size() < 6)
      {
      this->SetError("sub-command REGEX, mode REPLACE needs "
                     "at least 6 arguments total to command.");
      return false;
      }
    return this->RegexReplace(args);
    }

  std::string e = "sub-command REGEX does not recognize mode "+mode;
  this->SetError(e);
  return false;
}

//----------------------------------------------------------------------------
bool cmStringCommand::RegexMatch(std::vector<std::string> const& args)
{
  //"STRING(REGEX MATCH <regular_expression> <output variable>
  // <input> [<input>...])\n";
  std::string regex = args[2];
  std::string outvar = args[3];

  this->Makefile->ClearMatches();
  // Compile the regular expression.
  cmsys::RegularExpression re;
  if(!re.compile(regex.c_str()))
    {
    std::string e =
      "sub-command REGEX, mode MATCH failed to compile regex \""+regex+"\".";
    this->SetError(e);
    return false;
    }

  // Concatenate all the last arguments together.
  std::string input = cmJoin(cmRange(args).advance(4), std::string());

  // Scan through the input for all matches.
  std::string output;
  if(re.find(input.c_str()))
    {
    this->Makefile->StoreMatches(re);
    std::string::size_type l = re.start();
    std::string::size_type r = re.end();
    if(r-l == 0)
      {
      std::string e =
        "sub-command REGEX, mode MATCH regex \""+regex+
        "\" matched an empty string.";
      this->SetError(e);
      return false;
      }
    output = input.substr(l, r-l);
    }

  // Store the output in the provided variable.
  this->Makefile->AddDefinition(outvar, output.c_str());
  return true;
}

//----------------------------------------------------------------------------
bool cmStringCommand::RegexMatchAll(std::vector<std::string> const& args)
{
  //"STRING(REGEX MATCHALL <regular_expression> <output variable> <input>
  // [<input>...])\n";
  std::string regex = args[2];
  std::string outvar = args[3];

  this->Makefile->ClearMatches();
  // Compile the regular expression.
  cmsys::RegularExpression re;
  if(!re.compile(regex.c_str()))
    {
    std::string e =
      "sub-command REGEX, mode MATCHALL failed to compile regex \""+
      regex+"\".";
    this->SetError(e);
    return false;
    }

  // Concatenate all the last arguments together.
  std::string input = cmJoin(cmRange(args).advance(4), std::string());

  // Scan through the input for all matches.
  std::string output;
  const char* p = input.c_str();
  while(re.find(p))
    {
    this->Makefile->StoreMatches(re);
    std::string::size_type l = re.start();
    std::string::size_type r = re.end();
    if(r-l == 0)
      {
      std::string e = "sub-command REGEX, mode MATCHALL regex \""+
        regex+"\" matched an empty string.";
      this->SetError(e);
      return false;
      }
    if(!output.empty())
      {
      output += ";";
      }
    output += std::string(p+l, r-l);
    p += r;
    }

  // Store the output in the provided variable.
  this->Makefile->AddDefinition(outvar, output.c_str());
  return true;
}

//----------------------------------------------------------------------------
bool cmStringCommand::RegexReplace(std::vector<std::string> const& args)
{
  //"STRING(REGEX REPLACE <regular_expression> <replace_expression>
  // <output variable> <input> [<input>...])\n"
  std::string regex = args[2];
  std::string replace = args[3];
  std::string outvar = args[4];

  // Pull apart the replace expression to find the escaped [0-9] values.
  std::vector<RegexReplacement> replacement;
  std::string::size_type l = 0;
  while(l < replace.length())
    {
    std::string::size_type r = replace.find("\\", l);
    if(r == std::string::npos)
      {
      r = replace.length();
      replacement.push_back(replace.substr(l, r-l));
      }
    else
      {
      if(r-l > 0)
        {
        replacement.push_back(replace.substr(l, r-l));
        }
      if(r == (replace.length()-1))
        {
        this->SetError("sub-command REGEX, mode REPLACE: "
                       "replace-expression ends in a backslash.");
        return false;
        }
      if((replace[r+1] >= '0') && (replace[r+1] <= '9'))
        {
        replacement.push_back(replace[r+1]-'0');
        }
      else if(replace[r+1] == 'n')
        {
        replacement.push_back("\n");
        }
      else if(replace[r+1] == '\\')
        {
        replacement.push_back("\\");
        }
      else
        {
        std::string e = "sub-command REGEX, mode REPLACE: Unknown escape \"";
        e += replace.substr(r, 2);
        e += "\" in replace-expression.";
        this->SetError(e);
        return false;
        }
      r += 2;
      }
    l = r;
    }

  this->Makefile->ClearMatches();
  // Compile the regular expression.
  cmsys::RegularExpression re;
  if(!re.compile(regex.c_str()))
    {
    std::string e =
      "sub-command REGEX, mode REPLACE failed to compile regex \""+
      regex+"\".";
    this->SetError(e);
    return false;
    }

  // Concatenate all the last arguments together.
  std::string input = cmJoin(cmRange(args).advance(5), std::string());

  // Scan through the input for all matches.
  std::string output;
  std::string::size_type base = 0;
  while(re.find(input.c_str()+base))
    {
    this->Makefile->StoreMatches(re);
    std::string::size_type l2 = re.start();
    std::string::size_type r = re.end();

    // Concatenate the part of the input that was not matched.
    output += input.substr(base, l2);

    // Make sure the match had some text.
    if(r-l2 == 0)
      {
      std::string e = "sub-command REGEX, mode REPLACE regex \""+
        regex+"\" matched an empty string.";
      this->SetError(e);
      return false;
      }

    // Concatenate the replacement for the match.
    for(unsigned int i=0; i < replacement.size(); ++i)
      {
      if(replacement[i].number < 0)
        {
        // This is just a plain-text part of the replacement.
        output += replacement[i].value;
        }
      else
        {
        // Replace with part of the match.
        int n = replacement[i].number;
        std::string::size_type start = re.start(n);
        std::string::size_type end = re.end(n);
        std::string::size_type len = input.length()-base;
        if((start != std::string::npos) && (end != std::string::npos) &&
           (start <= len) && (end <= len))
          {
          output += input.substr(base+start, end-start);
          }
        else
          {
          std::string e =
            "sub-command REGEX, mode REPLACE: replace expression \""+
            replace+"\" contains an out-of-range escape for regex \""+
            regex+"\".";
          this->SetError(e);
          return false;
          }
        }
      }

    // Move past the match.
    base += r;
    }

  // Concatenate the text after the last match.
  output += input.substr(base, input.length()-base);

  // Store the output in the provided variable.
  this->Makefile->AddDefinition(outvar, output.c_str());
  return true;
}

//----------------------------------------------------------------------------
bool cmStringCommand::HandleFindCommand(std::vector<std::string> const&
                                           args)
{
  // check if all required parameters were passed
  if(args.size() < 4 || args.size() > 5)
    {
    this->SetError("sub-command FIND requires 3 or 4 parameters.");
    return false;
    }

  // check if the reverse flag was set or not
  bool reverseMode = false;
  if(args.size() == 5 && args[4] == "REVERSE")
    {
    reverseMode = true;
    }

  // if we have 5 arguments the last one must be REVERSE
  if(args.size() == 5 && args[4] != "REVERSE")
    {
    this->SetError("sub-command FIND: unknown last parameter");
    return false;
    }

  // local parameter names.
  const std::string& sstring = args[1];
  const std::string& schar = args[2];
  const std::string& outvar = args[3];

  // ensure that the user cannot accidentally specify REVERSE as a variable
  if(outvar == "REVERSE")
    {
    this->SetError("sub-command FIND does not allow to select REVERSE as "
                   "the output variable.  "
                   "Maybe you missed the actual output variable?");
    return false;
    }

  // try to find the character and return its position
  size_t pos;
  if(!reverseMode)
    {
    pos = sstring.find(schar);
    }
  else
    {
    pos = sstring.rfind(schar);
    }
  if(std::string::npos != pos)
    {
    std::ostringstream s;
    s << pos;
    this->Makefile->AddDefinition(outvar, s.str().c_str());
    return true;
    }

  // the character was not found, but this is not really an error
  this->Makefile->AddDefinition(outvar, "-1");
  return true;
}

//----------------------------------------------------------------------------
bool cmStringCommand::HandleCompareCommand(std::vector<std::string> const&
                                           args)
{
  if(args.size() < 2)
    {
    this->SetError("sub-command COMPARE requires a mode to be specified.");
    return false;
    }
  std::string mode = args[1];
  if((mode == "EQUAL") || (mode == "NOTEQUAL") ||
     (mode == "LESS") || (mode == "GREATER"))
    {
    if(args.size() < 5)
      {
      std::string e = "sub-command COMPARE, mode ";
      e += mode;
      e += " needs at least 5 arguments total to command.";
      this->SetError(e);
      return false;
      }

    const std::string& left = args[2];
    const std::string& right = args[3];
    const std::string& outvar = args[4];
    bool result;
    if(mode == "LESS")
      {
      result = (left < right);
      }
    else if(mode == "GREATER")
      {
      result = (left > right);
      }
    else if(mode == "EQUAL")
      {
      result = (left == right);
      }
    else // if(mode == "NOTEQUAL")
      {
      result = !(left == right);
      }
    if(result)
      {
      this->Makefile->AddDefinition(outvar, "1");
      }
    else
      {
      this->Makefile->AddDefinition(outvar, "0");
      }
    return true;
    }
  std::string e = "sub-command COMPARE does not recognize mode "+mode;
  this->SetError(e);
  return false;
}

//----------------------------------------------------------------------------
bool cmStringCommand::HandleReplaceCommand(std::vector<std::string> const&
                                           args)
{
  if(args.size() < 5)
    {
    this->SetError("sub-command REPLACE requires at least four arguments.");
    return false;
    }

  const std::string& matchExpression = args[1];
  const std::string& replaceExpression = args[2];
  const std::string& variableName = args[3];

  std::string input = cmJoin(cmRange(args).advance(4), std::string());

  cmsys::SystemTools::ReplaceString(input, matchExpression.c_str(),
                                    replaceExpression.c_str());

  this->Makefile->AddDefinition(variableName, input.c_str());
  return true;
}

//----------------------------------------------------------------------------
bool cmStringCommand::HandleSubstringCommand(std::vector<std::string> const&
                                             args)
{
  if(args.size() != 5)
    {
    this->SetError("sub-command SUBSTRING requires four arguments.");
    return false;
    }

  const std::string& stringValue = args[1];
  int begin = atoi(args[2].c_str());
  int end = atoi(args[3].c_str());
  const std::string& variableName = args[4];

  size_t stringLength = stringValue.size();
  int intStringLength = static_cast<int>(stringLength);
  if ( begin < 0 || begin > intStringLength )
    {
    std::ostringstream ostr;
    ostr << "begin index: " << begin << " is out of range 0 - "
         << stringLength;
    this->SetError(ostr.str());
    return false;
    }
  if ( end < -1 )
    {
    std::ostringstream ostr;
    ostr << "end index: " << end << " should be -1 or greater";
    this->SetError(ostr.str());
    return false;
    }

  this->Makefile->AddDefinition(variableName,
                                stringValue.substr(begin, end).c_str());
  return true;
}

//----------------------------------------------------------------------------
bool cmStringCommand
::HandleLengthCommand(std::vector<std::string> const& args)
{
  if(args.size() != 3)
    {
    this->SetError("sub-command LENGTH requires two arguments.");
    return false;
    }

  const std::string& stringValue = args[1];
  const std::string& variableName = args[2];

  size_t length = stringValue.size();
  char buffer[1024];
  sprintf(buffer, "%d", static_cast<int>(length));

  this->Makefile->AddDefinition(variableName, buffer);
  return true;
}

//----------------------------------------------------------------------------
bool cmStringCommand
::HandleConcatCommand(std::vector<std::string> const& args)
{
  if(args.size() < 2)
    {
    this->SetError("sub-command CONCAT requires at least one argument.");
    return false;
    }

  std::string const& variableName = args[1];
  std::string value = cmJoin(cmRange(args).advance(2), std::string());

  this->Makefile->AddDefinition(variableName, value.c_str());
  return true;
}

//----------------------------------------------------------------------------
bool cmStringCommand
::HandleMakeCIdentifierCommand(std::vector<std::string> const& args)
{
  if(args.size() != 3)
    {
    this->SetError("sub-command MAKE_C_IDENTIFIER requires two arguments.");
    return false;
    }

  const std::string& input = args[1];
  const std::string& variableName = args[2];

  this->Makefile->AddDefinition(variableName,
                      cmSystemTools::MakeCidentifier(input).c_str());
  return true;
}

//----------------------------------------------------------------------------
bool cmStringCommand
::HandleGenexStripCommand(std::vector<std::string> const& args)
{
  if(args.size() != 3)
    {
    this->SetError("sub-command GENEX_STRIP requires two arguments.");
    return false;
    }

  const std::string& input = args[1];

  std::string result = cmGeneratorExpression::Preprocess(input,
                        cmGeneratorExpression::StripAllGeneratorExpressions);

  const std::string& variableName = args[2];

  this->Makefile->AddDefinition(variableName, result.c_str());
  return true;
}

//----------------------------------------------------------------------------
bool cmStringCommand::HandleStripCommand(
  std::vector<std::string> const& args)
{
 if(args.size() != 3)
    {
    this->SetError("sub-command STRIP requires two arguments.");
    return false;
    }

  const std::string& stringValue = args[1];
  const std::string& variableName = args[2];
  size_t inStringLength = stringValue.size();
  size_t startPos = inStringLength + 1;
  size_t endPos = 0;
  const char* ptr = stringValue.c_str();
  size_t cc;
  for ( cc = 0; cc < inStringLength; ++ cc )
    {
    if ( !isspace(*ptr) )
      {
      if ( startPos > inStringLength )
        {
        startPos = cc;
        }
      endPos = cc;
      }
    ++ ptr;
    }

  size_t outLength = 0;

  // if the input string didn't contain any non-space characters, return
  // an empty string
  if (startPos > inStringLength)
    {
    outLength = 0;
    startPos = 0;
    }
  else
    {
    outLength=endPos - startPos + 1;
    }

  this->Makefile->AddDefinition(variableName,
    stringValue.substr(startPos, outLength).c_str());
  return true;
}

//----------------------------------------------------------------------------
bool cmStringCommand
::HandleRandomCommand(std::vector<std::string> const& args)
{
  if(args.size() < 2 || args.size() == 3 || args.size() == 5)
    {
    this->SetError("sub-command RANDOM requires at least one argument.");
    return false;
    }

  static bool seeded = false;
  bool force_seed = false;
  unsigned int seed = 0;
  int length = 5;
  const char cmStringCommandDefaultAlphabet[] = "qwertyuiopasdfghjklzxcvbnm"
    "QWERTYUIOPASDFGHJKLZXCVBNM"
    "0123456789";
  std::string alphabet;

  if ( args.size() > 3 )
    {
    size_t i = 1;
    size_t stopAt = args.size() - 2;

    for ( ; i < stopAt; ++i )
      {
      if ( args[i] == "LENGTH" )
        {
        ++i;
        length = atoi(args[i].c_str());
        }
      else if ( args[i] == "ALPHABET" )
        {
        ++i;
        alphabet = args[i];
        }
      else if ( args[i] == "RANDOM_SEED" )
        {
        ++i;
        seed = static_cast<unsigned int>(atoi(args[i].c_str()));
        force_seed = true;
        }
      }
    }
  if (alphabet.empty())
    {
    alphabet = cmStringCommandDefaultAlphabet;
    }

  double sizeofAlphabet = static_cast<double>(alphabet.size());
  if ( sizeofAlphabet < 1 )
    {
    this->SetError("sub-command RANDOM invoked with bad alphabet.");
    return false;
    }
  if ( length < 1 )
    {
    this->SetError("sub-command RANDOM invoked with bad length.");
    return false;
    }
  const std::string& variableName = args[args.size()-1];

  std::vector<char> result;

  if (!seeded || force_seed)
    {
    seeded = true;
    srand(force_seed? seed : cmSystemTools::RandomSeed());
    }

  const char* alphaPtr = alphabet.c_str();
  int cc;
  for ( cc = 0; cc < length; cc ++ )
    {
    int idx=(int) (sizeofAlphabet* rand()/(RAND_MAX+1.0));
    result.push_back(*(alphaPtr + idx));
    }
  result.push_back(0);

  this->Makefile->AddDefinition(variableName, &*result.begin());
  return true;
}

//----------------------------------------------------------------------------
bool cmStringCommand
::HandleTimestampCommand(std::vector<std::string> const& args)
{
  if(args.size() < 2)
    {
    this->SetError("sub-command TIMESTAMP requires at least one argument.");
    return false;
    }
  else if(args.size() > 4)
    {
    this->SetError("sub-command TIMESTAMP takes at most three arguments.");
    return false;
    }

  unsigned int argsIndex = 1;

  const std::string &outputVariable = args[argsIndex++];

  std::string formatString;
  if(args.size() > argsIndex && args[argsIndex] != "UTC")
    {
    formatString = args[argsIndex++];
    }

  bool utcFlag = false;
  if(args.size() > argsIndex)
    {
    if(args[argsIndex] == "UTC")
      {
      utcFlag = true;
      }
    else
      {
      std::string e = " TIMESTAMP sub-command does not recognize option " +
          args[argsIndex] + ".";
      this->SetError(e);
      return false;
      }
    }

  cmTimestamp timestamp;
  std::string result = timestamp.CurrentTime(formatString, utcFlag);
  this->Makefile->AddDefinition(outputVariable, result.c_str());

  return true;
}

bool cmStringCommand
::HandleUuidCommand(std::vector<std::string> const& args)
{
#if defined(CMAKE_BUILD_WITH_CMAKE)
  unsigned int argsIndex = 1;

  if(args.size() < 2)
    {
    this->SetError("UUID sub-command requires an output variable.");
    return false;
    }

  const std::string &outputVariable = args[argsIndex++];

  std::string uuidNamespaceString;
  std::string uuidName;
  std::string uuidType;
  bool uuidUpperCase = false;

  while(args.size() > argsIndex)
    {
    if(args[argsIndex] == "NAMESPACE")
      {
      ++argsIndex;
      if(argsIndex >= args.size())
        {
        this->SetError("UUID sub-command, NAMESPACE requires a value.");
        return false;
        }
      uuidNamespaceString = args[argsIndex++];
      }
    else if(args[argsIndex] == "NAME")
      {
      ++argsIndex;
      if(argsIndex >= args.size())
        {
        this->SetError("UUID sub-command, NAME requires a value.");
        return false;
        }
      uuidName = args[argsIndex++];
      }
    else if(args[argsIndex] == "TYPE")
      {
      ++argsIndex;
      if(argsIndex >= args.size())
        {
        this->SetError("UUID sub-command, TYPE requires a value.");
        return false;
        }
      uuidType = args[argsIndex++];
      }
    else if(args[argsIndex] == "UPPER")
      {
      ++argsIndex;
      uuidUpperCase = true;
      }
    else
      {
      std::string e = "UUID sub-command does not recognize option " +
          args[argsIndex] + ".";
      this->SetError(e);
      return false;
      }
    }

  std::string uuid;
  cmUuid uuidGenerator;

  std::vector<unsigned char> uuidNamespace;
  if(!uuidGenerator.StringToBinary(uuidNamespaceString, uuidNamespace))
    {
    this->SetError("UUID sub-command, malformed NAMESPACE UUID.");
    return false;
    }

  if(uuidType == "MD5")
    {
    uuid = uuidGenerator.FromMd5(uuidNamespace, uuidName);
    }
  else if(uuidType == "SHA1")
    {
    uuid = uuidGenerator.FromSha1(uuidNamespace, uuidName);
    }
  else
    {
    std::string e = "UUID sub-command, unknown TYPE '" + uuidType + "'.";
    this->SetError(e);
    return false;
    }

  if(uuid.empty())
    {
    this->SetError("UUID sub-command, generation failed.");
    return false;
    }

  if(uuidUpperCase)
    {
    uuid = cmSystemTools::UpperCase(uuid);
    }

  this->Makefile->AddDefinition(outputVariable, uuid.c_str());
  return true;
#else
  std::ostringstream e;
  e << args[0] << " not available during bootstrap";
  this->SetError(e.str().c_str());
  return false;
#endif
}
