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
#include KWSYS_HEADER(CommandLineArguments.hxx)

#include KWSYS_HEADER(Configure.hxx)
#include KWSYS_HEADER(String.hxx)

#include KWSYS_HEADER(stl/vector)
#include KWSYS_HEADER(stl/map)
#include KWSYS_HEADER(stl/set)
#include KWSYS_HEADER(ios/sstream)
#include KWSYS_HEADER(ios/iostream)

// Work-around CMake dependency scanning limitation.  This must
// duplicate the above list of headers.
#if 0
# include "CommandLineArguments.hxx.in"
# include "Configure.hxx.in"
# include "kwsys_stl.hxx.in"
# include "kwsys_ios_sstream.h.in"
# include "kwsys_ios_iostream.h.in"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _MSC_VER
# pragma warning (disable: 4786)
#endif

#if defined(__sgi) && !defined(__GNUC__)
# pragma set woff 1375 /* base class destructor not virtual */
#endif

#if 0
#  define CommandLineArguments_DEBUG(x) \
  kwsys_ios::cout << __LINE__ << " CLA: " << x << kwsys_ios::endl
#else
#  define CommandLineArguments_DEBUG(x)
#endif

namespace KWSYS_NAMESPACE
{

//----------------------------------------------------------------------------
//============================================================================
struct CommandLineArgumentsCallbackStructure
{
  const char* Argument;
  int ArgumentType;
  CommandLineArguments::CallbackType Callback;
  void* CallData;
  void* Variable;
  int VariableType;
  const char* Help;
};
 
class CommandLineArgumentsVectorOfStrings : 
  public kwsys_stl::vector<kwsys::String> {};
class CommandLineArgumentsSetOfStrings :
  public kwsys_stl::set<kwsys::String> {};
class CommandLineArgumentsMapOfStrucs : 
  public kwsys_stl::map<kwsys::String,
    CommandLineArgumentsCallbackStructure> {};

class CommandLineArgumentsInternal
{
public:
  CommandLineArgumentsInternal()
    {
    this->UnknownArgumentCallback = 0;
    this->ClientData = 0;
    this->LastArgument = 0;
    }

  typedef CommandLineArgumentsVectorOfStrings VectorOfStrings;
  typedef CommandLineArgumentsMapOfStrucs CallbacksMap;
  typedef kwsys::String String;
  typedef CommandLineArgumentsSetOfStrings SetOfStrings;

  VectorOfStrings Argv;
  String Argv0;
  CallbacksMap Callbacks;

  CommandLineArguments::ErrorCallbackType UnknownArgumentCallback;
  void*             ClientData;

  VectorOfStrings::size_type LastArgument;

  VectorOfStrings UnusedArguments;
};
//============================================================================
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
CommandLineArguments::CommandLineArguments()
{
  this->Internals = new CommandLineArguments::Internal;
  this->Help = "";
  this->LineLength = 80;
  this->StoreUnusedArgumentsFlag = false;
}

//----------------------------------------------------------------------------
CommandLineArguments::~CommandLineArguments()
{
  delete this->Internals;
}

//----------------------------------------------------------------------------
void CommandLineArguments::Initialize(int argc, const char* const argv[])
{
  int cc;

  this->Initialize();
  this->Internals->Argv0 = argv[0];
  for ( cc = 1; cc < argc; cc ++ )
    {
    this->ProcessArgument(argv[cc]);
    }
}

//----------------------------------------------------------------------------
void CommandLineArguments::Initialize(int argc, char* argv[])
{
  this->Initialize(argc, static_cast<const char* const*>(argv));
}

//----------------------------------------------------------------------------
void CommandLineArguments::Initialize()
{
  this->Internals->Argv.clear();
  this->Internals->LastArgument = 0;
}

//----------------------------------------------------------------------------
void CommandLineArguments::ProcessArgument(const char* arg)
{
  this->Internals->Argv.push_back(arg);
}

//----------------------------------------------------------------------------
bool CommandLineArguments::GetMatchedArguments(
  kwsys_stl::vector<kwsys_stl::string>* matches,
  const kwsys_stl::string& arg)
{
  matches->clear();
  CommandLineArguments::Internal::CallbacksMap::iterator it;

  // Does the argument match to any we know about?
  for ( it = this->Internals->Callbacks.begin();
    it != this->Internals->Callbacks.end();
    it ++ )
    {
    const CommandLineArguments::Internal::String& parg = it->first;
    CommandLineArgumentsCallbackStructure *cs = &it->second;
    if (cs->ArgumentType == CommandLineArguments::NO_ARGUMENT ||
      cs->ArgumentType == CommandLineArguments::SPACE_ARGUMENT) 
      {
      if ( arg == parg )
        {
        matches->push_back(parg);
        }
      }
    else if ( arg.find( parg ) == 0 )
      {
      matches->push_back(parg);
      }
    }
  return !matches->empty();
}

//----------------------------------------------------------------------------
int CommandLineArguments::Parse()
{
  kwsys_stl::vector<kwsys_stl::string>::size_type cc;
  kwsys_stl::vector<kwsys_stl::string> matches;
  if ( this->StoreUnusedArgumentsFlag )
    {
    this->Internals->UnusedArguments.clear();
    }
  for ( cc = 0; cc < this->Internals->Argv.size(); cc ++ )
    {
    const kwsys_stl::string& arg = this->Internals->Argv[cc]; 
    CommandLineArguments_DEBUG("Process argument: " << arg);
    this->Internals->LastArgument = cc;
    if ( this->GetMatchedArguments(&matches, arg) )
      {
      // Ok, we found one or more arguments that match what user specified.
      // Let's find the longest one.
      CommandLineArguments::Internal::VectorOfStrings::size_type kk;
      CommandLineArguments::Internal::VectorOfStrings::size_type maxidx = 0;
      CommandLineArguments::Internal::String::size_type maxlen = 0;
      for ( kk = 0; kk < matches.size(); kk ++ )
        {
        if ( matches[kk].size() > maxlen )
          {
          maxlen = matches[kk].size();
          maxidx = kk;
          }
        }
      // So, the longest one is probably the right one. Now see if it has any
      // additional value
      CommandLineArgumentsCallbackStructure *cs 
        = &this->Internals->Callbacks[matches[maxidx]];
      const kwsys_stl::string& sarg = matches[maxidx];
      if ( cs->Argument != sarg )
        {
        abort();
        }
      switch ( cs->ArgumentType )
        {
      case NO_ARGUMENT:
        // No value
        if ( !this->PopulateVariable(cs, 0) )
          {
          return 0;
          }
        break;
      case SPACE_ARGUMENT:
        if ( cc == this->Internals->Argv.size()-1 )
          {
          this->Internals->LastArgument --;
          return 0;
          }
        CommandLineArguments_DEBUG("This is a space argument: " << arg
          << " value: " << this->Internals->Argv[cc+1]);
        // Value is the next argument
        if ( !this->PopulateVariable(cs, this->Internals->Argv[cc+1].c_str()) )
          {
          return 0;
          }
        cc ++;
        break;
      case EQUAL_ARGUMENT:
        if ( arg.size() == sarg.size() || arg.at(sarg.size()) != '=' )
          {
          this->Internals->LastArgument --;
          return 0;
          }
        // Value is everythng followed the '=' sign
        if ( !this->PopulateVariable(cs, arg.c_str() + sarg.size() + 1) )
          {
          return 0;
          }
        break;
      case CONCAT_ARGUMENT:
        // Value is whatever follows the argument
        if ( !this->PopulateVariable(cs, arg.c_str() + sarg.size()) )
          {
          return 0;
          }
        break;
      case MULTI_ARGUMENT:
        // Suck in all the rest of the arguments
        CommandLineArguments_DEBUG("This is a multi argument: " << arg);
        for (cc++; cc < this->Internals->Argv.size(); ++ cc )
          {
          const kwsys_stl::string& marg = this->Internals->Argv[cc];
          CommandLineArguments_DEBUG(" check multi argument value: " << marg);
          if ( this->GetMatchedArguments(&matches, marg) )
            {
            CommandLineArguments_DEBUG("End of multi argument " << arg << " with value: " << marg);
            break;
            }
          CommandLineArguments_DEBUG(" populate multi argument value: " << marg);
          if ( !this->PopulateVariable(cs, marg.c_str()) )
            {
            return 0;
            }
          }
        if ( cc != this->Internals->Argv.size() )
          {
          CommandLineArguments_DEBUG("Again End of multi argument " << arg);
          cc--;
          continue;
          }
        break;
      default:
        kwsys_ios::cerr << "Got unknown argument type: \"" << cs->ArgumentType << "\"" << kwsys_ios::endl;
        this->Internals->LastArgument --;
        return 0;
        }
      }
    else
      {
      // Handle unknown arguments
      if ( this->Internals->UnknownArgumentCallback )
        {
        if ( !this->Internals->UnknownArgumentCallback(arg.c_str(), 
            this->Internals->ClientData) )
          {
          this->Internals->LastArgument --;
          return 0;
          }
        return 1;
        }
      else if ( this->StoreUnusedArgumentsFlag )
        {
        CommandLineArguments_DEBUG("Store unused argument " << arg);
        this->Internals->UnusedArguments.push_back(arg);
        }
      else
        {
        kwsys_ios::cerr << "Got unknown argument: \"" << arg << "\"" << kwsys_ios::endl;
        this->Internals->LastArgument --;
        return 0;
        }
      }
    }
  return 1;
}

//----------------------------------------------------------------------------
void CommandLineArguments::GetRemainingArguments(int* argc, char*** argv)
{
  CommandLineArguments::Internal::VectorOfStrings::size_type size 
    = this->Internals->Argv.size() - this->Internals->LastArgument + 1;
  CommandLineArguments::Internal::VectorOfStrings::size_type cc;

  // Copy Argv0 as the first argument
  char** args = new char*[ size ];
  args[0] = new char[ this->Internals->Argv0.size() + 1 ];
  strcpy(args[0], this->Internals->Argv0.c_str());
  int cnt = 1;

  // Copy everything after the LastArgument, since that was not parsed.
  for ( cc = this->Internals->LastArgument+1; 
    cc < this->Internals->Argv.size(); cc ++ )
    {
    args[cnt] = new char[ this->Internals->Argv[cc].size() + 1];
    strcpy(args[cnt], this->Internals->Argv[cc].c_str());
    cnt ++;
    }
  *argc = cnt;
  *argv = args;
}

//----------------------------------------------------------------------------
void CommandLineArguments::GetUnusedArguments(int* argc, char*** argv)
{
  CommandLineArguments::Internal::VectorOfStrings::size_type size 
    = this->Internals->UnusedArguments.size() + 1;
  CommandLineArguments::Internal::VectorOfStrings::size_type cc;

  // Copy Argv0 as the first argument
  char** args = new char*[ size ];
  args[0] = new char[ this->Internals->Argv0.size() + 1 ];
  strcpy(args[0], this->Internals->Argv0.c_str());
  int cnt = 1;

  // Copy everything after the LastArgument, since that was not parsed.
  for ( cc = 0;
    cc < this->Internals->UnusedArguments.size(); cc ++ )
    {
    kwsys::String &str = this->Internals->UnusedArguments[cc];
    args[cnt] = new char[ str.size() + 1];
    strcpy(args[cnt], str.c_str());
    cnt ++;
    }
  *argc = cnt;
  *argv = args;
}

//----------------------------------------------------------------------------
void CommandLineArguments::DeleteRemainingArguments(int argc, char*** argv)
{
  int cc;
  for ( cc = 0; cc < argc; ++ cc )
    {
    delete [] (*argv)[cc];
    }
  delete [] *argv;
}

//----------------------------------------------------------------------------
void CommandLineArguments::AddCallback(const char* argument, ArgumentTypeEnum type, 
  CallbackType callback, void* call_data, const char* help)
{
  CommandLineArgumentsCallbackStructure s;
  s.Argument     = argument;
  s.ArgumentType = type;
  s.Callback     = callback;
  s.CallData     = call_data;
  s.VariableType = CommandLineArguments::NO_VARIABLE_TYPE;
  s.Variable     = 0;
  s.Help         = help;

  this->Internals->Callbacks[argument] = s;
  this->GenerateHelp();
}

//----------------------------------------------------------------------------
void CommandLineArguments::AddArgument(const char* argument, ArgumentTypeEnum type,
  VariableTypeEnum vtype, void* variable, const char* help)
{
  CommandLineArgumentsCallbackStructure s;
  s.Argument     = argument;
  s.ArgumentType = type;
  s.Callback     = 0;
  s.CallData     = 0;
  s.VariableType = vtype;
  s.Variable     = variable;
  s.Help         = help;

  this->Internals->Callbacks[argument] = s;
  this->GenerateHelp();
}

//----------------------------------------------------------------------------
#define CommandLineArgumentsAddArgumentMacro(type, ctype) \
  void CommandLineArguments::AddArgument(const char* argument, ArgumentTypeEnum type, \
    ctype* variable, const char* help) \
  { \
    this->AddArgument(argument, type, CommandLineArguments::type##_TYPE, variable, help); \
  }

CommandLineArgumentsAddArgumentMacro(BOOL,       bool)
CommandLineArgumentsAddArgumentMacro(INT,        int)
CommandLineArgumentsAddArgumentMacro(DOUBLE,     double)
CommandLineArgumentsAddArgumentMacro(STRING,     char*)
CommandLineArgumentsAddArgumentMacro(STL_STRING, kwsys_stl::string)

CommandLineArgumentsAddArgumentMacro(VECTOR_BOOL,       kwsys_stl::vector<bool>)
CommandLineArgumentsAddArgumentMacro(VECTOR_INT,        kwsys_stl::vector<int>)
CommandLineArgumentsAddArgumentMacro(VECTOR_DOUBLE,     kwsys_stl::vector<double>)
CommandLineArgumentsAddArgumentMacro(VECTOR_STRING,     kwsys_stl::vector<char*>)
CommandLineArgumentsAddArgumentMacro(VECTOR_STL_STRING, kwsys_stl::vector<kwsys_stl::string>)

//----------------------------------------------------------------------------
#define CommandLineArgumentsAddBooleanArgumentMacro(type, ctype) \
  void CommandLineArguments::AddBooleanArgument(const char* argument, \
    ctype* variable, const char* help) \
  { \
    this->AddArgument(argument, CommandLineArguments::NO_ARGUMENT, \
      CommandLineArguments::type##_TYPE, variable, help); \
  }

CommandLineArgumentsAddBooleanArgumentMacro(BOOL,       bool)
CommandLineArgumentsAddBooleanArgumentMacro(INT,        int)
CommandLineArgumentsAddBooleanArgumentMacro(DOUBLE,     double)
CommandLineArgumentsAddBooleanArgumentMacro(STRING,     char*)
CommandLineArgumentsAddBooleanArgumentMacro(STL_STRING, kwsys_stl::string)

//----------------------------------------------------------------------------
void CommandLineArguments::SetClientData(void* client_data)
{
  this->Internals->ClientData = client_data;
}

//----------------------------------------------------------------------------
void CommandLineArguments::SetUnknownArgumentCallback(
  CommandLineArguments::ErrorCallbackType callback)
{
  this->Internals->UnknownArgumentCallback = callback;
}

//----------------------------------------------------------------------------
const char* CommandLineArguments::GetHelp(const char* arg)
{
  CommandLineArguments::Internal::CallbacksMap::iterator it 
    = this->Internals->Callbacks.find(arg);
  if ( it == this->Internals->Callbacks.end() )
    {
    return 0;
    }

  // Since several arguments may point to the same argument, find the one this
  // one point to if this one is pointing to another argument.
  CommandLineArgumentsCallbackStructure *cs = &(it->second);
  for(;;)
    {
    CommandLineArguments::Internal::CallbacksMap::iterator hit 
      = this->Internals->Callbacks.find(cs->Help);
    if ( hit == this->Internals->Callbacks.end() )
      {
      break;
      }
    cs = &(hit->second);
    }
  return cs->Help;
}

//----------------------------------------------------------------------------
void CommandLineArguments::SetLineLength(unsigned int ll)
{
  if ( ll < 9 || ll > 1000 )
    {
    return;
    }
  this->LineLength = ll;
  this->GenerateHelp();
}

//----------------------------------------------------------------------------
const char* CommandLineArguments::GetArgv0()
{
  return this->Internals->Argv0.c_str();
}

//----------------------------------------------------------------------------
unsigned int CommandLineArguments::GetLastArgument()
{
  return static_cast<unsigned int>(this->Internals->LastArgument + 1);
}

//----------------------------------------------------------------------------
void CommandLineArguments::GenerateHelp()
{
  kwsys_ios::ostringstream str;
  
  // Collapse all arguments into the map of vectors of all arguments that do
  // the same thing.
  CommandLineArguments::Internal::CallbacksMap::iterator it;
  typedef kwsys_stl::map<CommandLineArguments::Internal::String, 
     CommandLineArguments::Internal::SetOfStrings > MapArgs;
  MapArgs mp;
  MapArgs::iterator mpit, smpit;
  for ( it = this->Internals->Callbacks.begin();
    it != this->Internals->Callbacks.end();
    it ++ )
    {
    CommandLineArgumentsCallbackStructure *cs = &(it->second);
    mpit = mp.find(cs->Help);
    if ( mpit != mp.end() )
      {
      mpit->second.insert(it->first);
      mp[it->first].insert(it->first);
      }
    else
      {
      mp[it->first].insert(it->first);
      }
    }
  for ( it = this->Internals->Callbacks.begin();
    it != this->Internals->Callbacks.end();
    it ++ )
    {
    CommandLineArgumentsCallbackStructure *cs = &(it->second);
    mpit = mp.find(cs->Help);
    if ( mpit != mp.end() )
      {
      mpit->second.insert(it->first);
      smpit = mp.find(it->first);
      CommandLineArguments::Internal::SetOfStrings::iterator sit;
      for ( sit = smpit->second.begin(); sit != smpit->second.end(); sit++ )
        {
        mpit->second.insert(*sit);
        }
      mp.erase(smpit);
      }
    else
      {
      mp[it->first].insert(it->first);
      }
    }
 
  // Find the length of the longest string
  CommandLineArguments::Internal::String::size_type maxlen = 0;
  for ( mpit = mp.begin();
    mpit != mp.end();
    mpit ++ )
    {
    CommandLineArguments::Internal::SetOfStrings::iterator sit;
    for ( sit = mpit->second.begin(); sit != mpit->second.end(); sit++ )
      {
      CommandLineArguments::Internal::String::size_type clen = sit->size();
      switch ( this->Internals->Callbacks[*sit].ArgumentType )
        {
        case CommandLineArguments::NO_ARGUMENT:     clen += 0; break;
        case CommandLineArguments::CONCAT_ARGUMENT: clen += 3; break;
        case CommandLineArguments::SPACE_ARGUMENT:  clen += 4; break;
        case CommandLineArguments::EQUAL_ARGUMENT:  clen += 4; break;
        }
      if ( clen > maxlen )
        {
        maxlen = clen;
        }
      }
    }

  // Create format for that string
  char format[80];
  sprintf(format, "  %%-%us  ", static_cast<unsigned int>(maxlen));

  maxlen += 4; // For the space before and after the option

  // Print help for each option
  for ( mpit = mp.begin();
    mpit != mp.end();
    mpit ++ )
    {
    CommandLineArguments::Internal::SetOfStrings::iterator sit;
    for ( sit = mpit->second.begin(); sit != mpit->second.end(); sit++ )
      {
      str << kwsys_ios::endl;
      char argument[100];
      sprintf(argument, "%s", sit->c_str());
      switch ( this->Internals->Callbacks[*sit].ArgumentType )
        {
        case CommandLineArguments::NO_ARGUMENT: break;
        case CommandLineArguments::CONCAT_ARGUMENT: strcat(argument, "opt"); break;
        case CommandLineArguments::SPACE_ARGUMENT:  strcat(argument, " opt"); break;
        case CommandLineArguments::EQUAL_ARGUMENT:  strcat(argument, "=opt"); break;
        case CommandLineArguments::MULTI_ARGUMENT:  strcat(argument, " opt opt ..."); break;
        }
      char buffer[80];
      sprintf(buffer, format, argument);
      str << buffer;
      }
    const char* ptr = this->Internals->Callbacks[mpit->first].Help;
    size_t len = strlen(ptr);
    int cnt = 0;
    while ( len > 0)
      {
      // If argument with help is longer than line length, split it on previous
      // space (or tab) and continue on the next line
      CommandLineArguments::Internal::String::size_type cc;
      for ( cc = 0; ptr[cc]; cc ++ )
        {
        if ( *ptr == ' ' || *ptr == '\t' )
          {
          ptr ++;
          len --;
          }
        }
      if ( cnt > 0 )
        {
        for ( cc = 0; cc < maxlen; cc ++ )
          {
          str << " ";
          }
        }
      CommandLineArguments::Internal::String::size_type skip = len;
      if ( skip > this->LineLength - maxlen )
        {
        skip = this->LineLength - maxlen;
        for ( cc = skip-1; cc > 0; cc -- )
          {
          if ( ptr[cc] == ' ' || ptr[cc] == '\t' )
            {
            break;
            }
          }
        if ( cc != 0 )
          {
          skip = cc;
          }
        }
      str.write(ptr, static_cast<kwsys_ios::streamsize>(skip));
      str << kwsys_ios::endl;
      ptr += skip;
      len -= skip;
      cnt ++;
      }
    }
  /*
  // This can help debugging help string
  str << endl;
  unsigned int cc;
  for ( cc = 0; cc < this->LineLength; cc ++ )
    {
    str << cc % 10;
    }
  str << endl;
  */
  this->Help = str.str();
}

//----------------------------------------------------------------------------
void CommandLineArguments::PopulateVariable(
  bool* variable, const kwsys_stl::string& value)
{
  if ( value == "1" || value == "ON" || value == "on" || value == "On" ||
    value == "TRUE" || value == "true" || value == "True" ||
    value == "yes" || value == "Yes" || value == "YES" )
    {
    *variable = true;
    }
  else
    {
    *variable = false;
    }
}

//----------------------------------------------------------------------------
void CommandLineArguments::PopulateVariable(
  int* variable, const kwsys_stl::string& value)
{
  char* res = 0;
  *variable = static_cast<int>(strtol(value.c_str(), &res, 10));
  //if ( res && *res )
  //  {
  //  Can handle non-int
  //  }
}

//----------------------------------------------------------------------------
void CommandLineArguments::PopulateVariable(
  double* variable, const kwsys_stl::string& value)
{
  char* res = 0;
  *variable = strtod(value.c_str(), &res);
  //if ( res && *res )
  //  {
  //  Can handle non-double
  //  }
}

//----------------------------------------------------------------------------
void CommandLineArguments::PopulateVariable(
  char** variable, const kwsys_stl::string& value)
{
  if ( *variable )
    {
    delete [] *variable;
    *variable = 0;
    }
  *variable = new char[ value.size() + 1 ];
  strcpy(*variable, value.c_str());
}

//----------------------------------------------------------------------------
void CommandLineArguments::PopulateVariable(
  kwsys_stl::string* variable, const kwsys_stl::string& value)
{
  *variable = value;
}

//----------------------------------------------------------------------------
void CommandLineArguments::PopulateVariable(
  kwsys_stl::vector<bool>* variable, const kwsys_stl::string& value)
{
  bool val = false;
  if ( value == "1" || value == "ON" || value == "on" || value == "On" ||
    value == "TRUE" || value == "true" || value == "True" ||
    value == "yes" || value == "Yes" || value == "YES" )
    {
    val = true;
    }
  variable->push_back(val);
}

//----------------------------------------------------------------------------
void CommandLineArguments::PopulateVariable(
  kwsys_stl::vector<int>* variable, const kwsys_stl::string& value)
{
  char* res = 0;
  variable->push_back(static_cast<int>(strtol(value.c_str(), &res, 10)));
  //if ( res && *res )
  //  {
  //  Can handle non-int
  //  }
}

//----------------------------------------------------------------------------
void CommandLineArguments::PopulateVariable(
  kwsys_stl::vector<double>* variable, const kwsys_stl::string& value)
{
  char* res = 0;
  variable->push_back(strtod(value.c_str(), &res));
  //if ( res && *res )
  //  {
  //  Can handle non-int
  //  }
}

//----------------------------------------------------------------------------
void CommandLineArguments::PopulateVariable(
  kwsys_stl::vector<char*>* variable, const kwsys_stl::string& value)
{
  char* var = new char[ value.size() + 1 ];
  strcpy(var, value.c_str());
  variable->push_back(var);
}

//----------------------------------------------------------------------------
void CommandLineArguments::PopulateVariable(
  kwsys_stl::vector<kwsys_stl::string>* variable,
  const kwsys_stl::string& value)
{
  variable->push_back(value);
}

//----------------------------------------------------------------------------
bool CommandLineArguments::PopulateVariable(CommandLineArgumentsCallbackStructure* cs,
  const char* value)
{
  // Call the callback
  if ( cs->Callback )
    {
    if ( !cs->Callback(cs->Argument, value, cs->CallData) )
      {
      this->Internals->LastArgument --;
      return 0;
      }
    }
  CommandLineArguments_DEBUG("Set argument: " << cs->Argument << " to " << value);
  if ( cs->Variable )
    {
    kwsys_stl::string var = "1";
    if ( value )
      {
      var = value;
      }
    switch ( cs->VariableType )
      {
    case CommandLineArguments::INT_TYPE:
      this->PopulateVariable(static_cast<int*>(cs->Variable), var);
      break;
    case CommandLineArguments::DOUBLE_TYPE:
      this->PopulateVariable(static_cast<double*>(cs->Variable), var);
      break;
    case CommandLineArguments::STRING_TYPE:
      this->PopulateVariable(static_cast<char**>(cs->Variable), var);
      break;
    case CommandLineArguments::STL_STRING_TYPE:
      this->PopulateVariable(static_cast<kwsys_stl::string*>(cs->Variable), var);
      break;
    case CommandLineArguments::BOOL_TYPE:
      this->PopulateVariable(static_cast<bool*>(cs->Variable), var);
      break;
    case CommandLineArguments::VECTOR_BOOL_TYPE:
      this->PopulateVariable(static_cast<kwsys_stl::vector<bool>*>(cs->Variable), var);
      break;
    case CommandLineArguments::VECTOR_INT_TYPE:
      this->PopulateVariable(static_cast<kwsys_stl::vector<int>*>(cs->Variable), var);
      break;
    case CommandLineArguments::VECTOR_DOUBLE_TYPE:
      this->PopulateVariable(static_cast<kwsys_stl::vector<double>*>(cs->Variable), var);
      break;
    case CommandLineArguments::VECTOR_STRING_TYPE:
      this->PopulateVariable(static_cast<kwsys_stl::vector<char*>*>(cs->Variable), var);
      break;
    case CommandLineArguments::VECTOR_STL_STRING_TYPE:
      this->PopulateVariable(static_cast<kwsys_stl::vector<kwsys_stl::string>*>(cs->Variable), var);
      break;
    default:
      kwsys_ios::cerr << "Got unknown variable type: \"" << cs->VariableType << "\"" << kwsys_ios::endl;
      this->Internals->LastArgument --;
      return 0;
      }
    }
  return 1;
}


} // namespace KWSYS_NAMESPACE
