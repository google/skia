#include "cmVisualStudioGeneratorOptions.h"
#include "cmSystemTools.h"
#include <cmsys/System.h>
#include "cmVisualStudio10TargetGenerator.h"

static
std::string cmVisualStudio10GeneratorOptionsEscapeForXML(std::string ret)
{
  cmSystemTools::ReplaceString(ret, ";", "%3B");
  cmSystemTools::ReplaceString(ret, "&", "&amp;");
  cmSystemTools::ReplaceString(ret, "<", "&lt;");
  cmSystemTools::ReplaceString(ret, ">", "&gt;");
  return ret;
}

static
std::string cmVisualStudioGeneratorOptionsEscapeForXML(std::string ret)
{
  cmSystemTools::ReplaceString(ret, "&", "&amp;");
  cmSystemTools::ReplaceString(ret, "\"", "&quot;");
  cmSystemTools::ReplaceString(ret, "<", "&lt;");
  cmSystemTools::ReplaceString(ret, ">", "&gt;");
  cmSystemTools::ReplaceString(ret, "\n", "&#x0D;&#x0A;");
  return ret;
}

//----------------------------------------------------------------------------
cmVisualStudioGeneratorOptions
::cmVisualStudioGeneratorOptions(cmLocalVisualStudioGenerator* lg,
                                 Tool tool,
                                 cmVisualStudio10TargetGenerator* g):
  cmIDEOptions(),
  LocalGenerator(lg), Version(lg->GetVersion()), CurrentTool(tool),
  TargetGenerator(g)
{
  // Preprocessor definitions are not allowed for linker tools.
  this->AllowDefine = (tool != Linker);

  // Slash options are allowed for VS.
  this->AllowSlash = true;

  this->FortranRuntimeDebug = false;
  this->FortranRuntimeDLL = false;
  this->FortranRuntimeMT = false;
}

//----------------------------------------------------------------------------
cmVisualStudioGeneratorOptions
::cmVisualStudioGeneratorOptions(cmLocalVisualStudioGenerator* lg,
                                 Tool tool,
                                 cmVS7FlagTable const* table,
                                 cmVS7FlagTable const* extraTable,
                                 cmVisualStudio10TargetGenerator* g):
  cmIDEOptions(),
  LocalGenerator(lg), Version(lg->GetVersion()), CurrentTool(tool),
  TargetGenerator(g)
{
  // Store the given flag tables.
  this->AddTable(table);
  this->AddTable(extraTable);

  // Preprocessor definitions are not allowed for linker tools.
  this->AllowDefine = (tool != Linker);

  // Slash options are allowed for VS.
  this->AllowSlash = true;

  this->FortranRuntimeDebug = false;
  this->FortranRuntimeDLL = false;
  this->FortranRuntimeMT = false;
}

//----------------------------------------------------------------------------
void cmVisualStudioGeneratorOptions::AddTable(cmVS7FlagTable const* table)
{
  if(table)
    {
    for(int i=0; i < FlagTableCount; ++i)
      {
      if (!this->FlagTable[i])
        {
        this->FlagTable[i] = table;
        break;
        }
      }
    }
}

//----------------------------------------------------------------------------
void cmVisualStudioGeneratorOptions::FixExceptionHandlingDefault()
{
  // Exception handling is on by default because the platform file has
  // "/EHsc" in the flags.  Normally, that will override this
  // initialization to off, but the user has the option of removing
  // the flag to disable exception handling.  When the user does
  // remove the flag we need to override the IDE default of on.
  switch (this->Version)
    {
    case cmGlobalVisualStudioGenerator::VS7:
    case cmGlobalVisualStudioGenerator::VS71:
      this->FlagMap["ExceptionHandling"] = "FALSE";
      break;
    case cmGlobalVisualStudioGenerator::VS10:
    case cmGlobalVisualStudioGenerator::VS11:
    case cmGlobalVisualStudioGenerator::VS12:
    case cmGlobalVisualStudioGenerator::VS14:
      // by default VS puts <ExceptionHandling></ExceptionHandling> empty
      // for a project, to make our projects look the same put a new line
      // and space over for the closing </ExceptionHandling> as the default
      // value
      this->FlagMap["ExceptionHandling"] = "\n      ";
      break;
    default:
      this->FlagMap["ExceptionHandling"] = "0";
    break;
    }
}

//----------------------------------------------------------------------------
void cmVisualStudioGeneratorOptions::SetVerboseMakefile(bool verbose)
{
  // If verbose makefiles have been requested and the /nologo option
  // was not given explicitly in the flags we want to add an attribute
  // to the generated project to disable logo suppression.  Otherwise
  // the GUI default is to enable suppression.
  //
  // On Visual Studio 10 (and later!), the value of this attribute should be
  // an empty string, instead of "FALSE", in order to avoid a warning:
  //   "cl ... warning D9035: option 'nologo-' has been deprecated"
  //
  if(verbose &&
     this->FlagMap.find("SuppressStartupBanner") == this->FlagMap.end())
    {
    this->FlagMap["SuppressStartupBanner"] =
      this->Version < cmGlobalVisualStudioGenerator::VS10 ? "FALSE" : "";
    }
}

bool cmVisualStudioGeneratorOptions::IsDebug() const
{
  return this->FlagMap.find("DebugInformationFormat") != this->FlagMap.end();
}

//----------------------------------------------------------------------------
bool cmVisualStudioGeneratorOptions::IsWinRt() const
{
  return this->FlagMap.find("CompileAsWinRT") != this->FlagMap.end();
}

//----------------------------------------------------------------------------
bool cmVisualStudioGeneratorOptions::UsingUnicode() const
{
  // Look for the a _UNICODE definition.
  for(std::vector<std::string>::const_iterator di = this->Defines.begin();
      di != this->Defines.end(); ++di)
    {
    if(*di == "_UNICODE")
      {
      return true;
      }
    }
  return false;
}
//----------------------------------------------------------------------------
bool cmVisualStudioGeneratorOptions::UsingSBCS() const
{
  // Look for the a _SBCS definition.
  for(std::vector<std::string>::const_iterator di = this->Defines.begin();
      di != this->Defines.end(); ++di)
    {
    if(*di == "_SBCS")
      {
      return true;
      }
    }
  return false;
}

//----------------------------------------------------------------------------
void cmVisualStudioGeneratorOptions::Parse(const char* flags)
{
  // Parse the input string as a windows command line since the string
  // is intended for writing directly into the build files.
  std::vector<std::string> args;
  cmSystemTools::ParseWindowsCommandLine(flags, args);

  // Process flags that need to be represented specially in the IDE
  // project file.
  for(std::vector<std::string>::iterator ai = args.begin();
      ai != args.end(); ++ai)
    {
    this->HandleFlag(ai->c_str());
    }
}

//----------------------------------------------------------------------------
void cmVisualStudioGeneratorOptions::ParseFinish()
{
  if(this->CurrentTool == FortranCompiler)
    {
    // "RuntimeLibrary" attribute values:
    //  "rtMultiThreaded", "0", /threads /libs:static
    //  "rtMultiThreadedDLL", "2", /threads /libs:dll
    //  "rtMultiThreadedDebug", "1", /threads /dbglibs /libs:static
    //  "rtMultiThreadedDebugDLL", "3", /threads /dbglibs /libs:dll
    // These seem unimplemented by the IDE:
    //  "rtSingleThreaded", "4", /libs:static
    //  "rtSingleThreadedDLL", "10", /libs:dll
    //  "rtSingleThreadedDebug", "5", /dbglibs /libs:static
    //  "rtSingleThreadedDebugDLL", "11", /dbglibs /libs:dll
    std::string rl = "rtMultiThreaded";
    rl += this->FortranRuntimeDebug? "Debug" : "";
    rl += this->FortranRuntimeDLL? "DLL" : "";
    this->FlagMap["RuntimeLibrary"] = rl;
    }
}

//----------------------------------------------------------------------------
void cmVisualStudioGeneratorOptions::StoreUnknownFlag(const char* flag)
{
  // Look for Intel Fortran flags that do not map well in the flag table.
  if(this->CurrentTool == FortranCompiler)
    {
    if(strcmp(flag, "/dbglibs") == 0)
      {
      this->FortranRuntimeDebug = true;
      return;
      }
    if(strcmp(flag, "/threads") == 0)
      {
      this->FortranRuntimeMT = true;
      return;
      }
    if(strcmp(flag, "/libs:dll") == 0)
      {
      this->FortranRuntimeDLL = true;
      return;
      }
    if(strcmp(flag, "/libs:static") == 0)
      {
      this->FortranRuntimeDLL = false;
      return;
      }
    }

  // This option is not known.  Store it in the output flags.
  this->FlagString += " ";
  this->FlagString +=
    cmSystemTools::EscapeWindowsShellArgument(
      flag,
      cmsysSystem_Shell_Flag_AllowMakeVariables |
      cmsysSystem_Shell_Flag_VSIDE);
}

//----------------------------------------------------------------------------
void cmVisualStudioGeneratorOptions::SetConfiguration(const char* config)
{
  this->Configuration = config;
}

//----------------------------------------------------------------------------
void
cmVisualStudioGeneratorOptions
::OutputPreprocessorDefinitions(std::ostream& fout,
                                const char* prefix,
                                const char* suffix,
                                const std::string& lang)
{
  if(this->Defines.empty())
    {
    return;
    }
  if(this->Version >= cmGlobalVisualStudioGenerator::VS10)
    {
    // if there are configuration specific flags, then
    // use the configuration specific tag for PreprocessorDefinitions
    if(this->Configuration.size())
      {
      fout << prefix;
      this->TargetGenerator->WritePlatformConfigTag(
        "PreprocessorDefinitions",
        this->Configuration.c_str(),
        0,
        0, 0, &fout);
      }
    else
      {
      fout << prefix << "<PreprocessorDefinitions>";
      }
    }
  else
    {
    fout << prefix <<  "PreprocessorDefinitions=\"";
    }
  const char* sep = "";
  for(std::vector<std::string>::const_iterator di = this->Defines.begin();
      di != this->Defines.end(); ++di)
    {
    // Escape the definition for the compiler.
    std::string define;
    if(this->Version < cmGlobalVisualStudioGenerator::VS10)
      {
      define =
        this->LocalGenerator->EscapeForShell(di->c_str(), true);
      }
    else
      {
      define = *di;
      }
    // Escape this flag for the IDE.
    if(this->Version >= cmGlobalVisualStudioGenerator::VS10)
      {
      define = cmVisualStudio10GeneratorOptionsEscapeForXML(define);

      if(lang == "RC")
        {
        cmSystemTools::ReplaceString(define, "\"", "\\\"");
        }
      }
    else
      {
      define = cmVisualStudioGeneratorOptionsEscapeForXML(define);
      }
    // Store the flag in the project file.
    fout << sep << define;
    sep = ";";
    }
  if(this->Version >= cmGlobalVisualStudioGenerator::VS10)
    {
    fout <<  ";%(PreprocessorDefinitions)</PreprocessorDefinitions>" << suffix;
    }
  else
    {
    fout << "\"" << suffix;
    }
}

//----------------------------------------------------------------------------
void
cmVisualStudioGeneratorOptions
::OutputFlagMap(std::ostream& fout, const char* indent)
{
  if(this->Version >= cmGlobalVisualStudioGenerator::VS10)
    {
    for(std::map<std::string, FlagValue>::iterator m = this->FlagMap.begin();
        m != this->FlagMap.end(); ++m)
      {
      fout << indent;
      if(this->Configuration.size())
        {
        this->TargetGenerator->WritePlatformConfigTag(
          m->first.c_str(),
          this->Configuration.c_str(),
          0,
          0, 0, &fout);
        }
      else
        {
        fout << "<" << m->first << ">";
        }
      const char* sep = "";
      for(std::vector<std::string>::iterator i = m->second.begin();
            i != m->second.end(); ++i)
        {
        fout << sep << cmVisualStudio10GeneratorOptionsEscapeForXML(*i);
        sep = ";";
        }
      fout  << "</" << m->first << ">\n";
      }
    }
  else
    {
    for(std::map<std::string, FlagValue>::iterator m = this->FlagMap.begin();
        m != this->FlagMap.end(); ++m)
      {
      fout << indent << m->first << "=\"";
      const char* sep = "";
      for(std::vector<std::string>::iterator i = m->second.begin();
            i != m->second.end(); ++i)
        {
        fout << sep << cmVisualStudioGeneratorOptionsEscapeForXML(*i);
        sep = ";";
        }
      fout << "\"\n";
      }
    }
}

//----------------------------------------------------------------------------
void
cmVisualStudioGeneratorOptions
::OutputAdditionalOptions(std::ostream& fout,
                          const char* prefix,
                          const char* suffix)
{
  if(!this->FlagString.empty())
    {
    if(this->Version >= cmGlobalVisualStudioGenerator::VS10)
      {
      fout << prefix;
      if(this->Configuration.size())
        {
        this->TargetGenerator->WritePlatformConfigTag(
          "AdditionalOptions",
          this->Configuration.c_str(),
          0,
          0, 0, &fout);
        }
      else
        {
        fout << "<AdditionalOptions>";
        }
      fout << cmVisualStudio10GeneratorOptionsEscapeForXML(this->FlagString)
           << " %(AdditionalOptions)</AdditionalOptions>\n";
      }
    else
      {
      fout << prefix << "AdditionalOptions=\"";
      fout << cmVisualStudioGeneratorOptionsEscapeForXML(this->FlagString);
      fout << "\"" << suffix;
      }
    }
}
