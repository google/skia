/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmLoadCommandCommand.h"
#include "cmCPluginAPI.h"
#include "cmCPluginAPI.cxx"
#include "cmDynamicLoader.h"

#include <cmsys/DynamicLoader.hxx>

#include <stdlib.h>

#ifdef __QNX__
# include <malloc.h> /* for malloc/free on QNX */
#endif

#include <signal.h>
extern "C" void TrapsForSignalsCFunction(int sig);


// a class for loadabple commands
class cmLoadedCommand : public cmCommand
{
public:
  cmLoadedCommand() {
    memset(&this->info,0,sizeof(this->info));
    this->info.CAPI = &cmStaticCAPI;
  }

  ///! clean up any memory allocated by the plugin
  ~cmLoadedCommand();

  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
      cmLoadedCommand *newC = new cmLoadedCommand;
      // we must copy when we clone
      memcpy(&newC->info,&this->info,sizeof(info));
      return newC;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus &);

  /**
   * This is called at the end after all the information
   * specified by the command is accumulated. Most commands do
   * not implement this method.  At this point, reading and
   * writing to the cache can be done.
   */
  virtual void FinalPass();
  virtual bool HasFinalPass() const
    { return this->info.FinalPass? true:false; }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual std::string GetName() const { return info.Name; }

  static const char* LastName;
  static void TrapsForSignals(int sig)
    {
      fprintf(stderr, "CMake loaded command %s crashed with signal: %d.\n",
              cmLoadedCommand::LastName, sig);
    }
  static void InstallSignalHandlers(const char* name, int remove = 0)
    {
      cmLoadedCommand::LastName = name;
      if(!name)
        {
        cmLoadedCommand::LastName = "????";
        }

      if(!remove)
        {
        signal(SIGSEGV, TrapsForSignalsCFunction);
#ifdef SIGBUS
        signal(SIGBUS,  TrapsForSignalsCFunction);
#endif
        signal(SIGILL,  TrapsForSignalsCFunction);
        }
      else
        {
        signal(SIGSEGV, 0);
#ifdef SIGBUS
        signal(SIGBUS,  0);
#endif
        signal(SIGILL,  0);
        }
    }

  cmTypeMacro(cmLoadedCommand, cmCommand);

  cmLoadedCommandInfo info;
};

extern "C" void TrapsForSignalsCFunction(int sig)
{
  cmLoadedCommand::TrapsForSignals(sig);
}


const char* cmLoadedCommand::LastName = 0;

bool cmLoadedCommand::InitialPass(std::vector<std::string> const& args,
                                  cmExecutionStatus &)
{
  if (!info.InitialPass)
    {
    return true;
    }

  // clear the error string
  if (this->info.Error)
    {
    free(this->info.Error);
    }

  // create argc and argv and then invoke the command
  int argc = static_cast<int> (args.size());
  char **argv = 0;
  if (argc)
    {
    argv = (char **)malloc(argc*sizeof(char *));
    }
  int i;
  for (i = 0; i < argc; ++i)
    {
    argv[i] = strdup(args[i].c_str());
    }
  cmLoadedCommand::InstallSignalHandlers(info.Name);
  int result = info.InitialPass((void *)&info,
                                (void *)this->Makefile,argc,argv);
  cmLoadedCommand::InstallSignalHandlers(info.Name, 1);
  cmFreeArguments(argc,argv);

  if (result)
    {
    return true;
    }

  /* Initial Pass must have failed so set the error string */
  if (this->info.Error)
    {
    this->SetError(this->info.Error);
    }
  return false;
}

void cmLoadedCommand::FinalPass()
{
  if (this->info.FinalPass)
    {
    cmLoadedCommand::InstallSignalHandlers(info.Name);
    this->info.FinalPass((void *)&this->info,(void *)this->Makefile);
    cmLoadedCommand::InstallSignalHandlers(info.Name, 1);
    }
}

cmLoadedCommand::~cmLoadedCommand()
{
  if (this->info.Destructor)
    {
    cmLoadedCommand::InstallSignalHandlers(info.Name);
    this->info.Destructor((void *)&this->info);
    cmLoadedCommand::InstallSignalHandlers(info.Name, 1);
    }
  if (this->info.Error)
    {
    free(this->info.Error);
    }
}

// cmLoadCommandCommand
bool cmLoadCommandCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if(this->Disallowed(cmPolicies::CMP0031,
      "The load_command command should not be called; see CMP0031."))
    { return true; }
  if(args.size() < 1 )
    {
    return true;
    }

  // Construct a variable to report what file was loaded, if any.
  // Start by removing the definition in case of failure.
  std::string reportVar = "CMAKE_LOADED_COMMAND_";
  reportVar += args[0];
  this->Makefile->RemoveDefinition(reportVar);

  // the file must exist
  std::string moduleName =
    this->Makefile->GetRequiredDefinition("CMAKE_SHARED_MODULE_PREFIX");
  moduleName += "cm" + args[0];
  moduleName +=
    this->Makefile->GetRequiredDefinition("CMAKE_SHARED_MODULE_SUFFIX");

  // search for the file
  std::vector<std::string> path;
  for (unsigned int j = 1; j < args.size(); j++)
    {
    // expand variables
    std::string exp = args[j];
    cmSystemTools::ExpandRegistryValues(exp);

    // Glob the entry in case of wildcards.
    cmSystemTools::GlobDirs(exp, path);
    }

  // Try to find the program.
  std::string fullPath = cmSystemTools::FindFile(moduleName.c_str(), path);
  if (fullPath == "")
    {
    std::ostringstream e;
    e << "Attempt to load command failed from file \""
      << moduleName << "\"";
    this->SetError(e.str());
    return false;
    }

  // try loading the shared library / dll
  cmsys::DynamicLoader::LibraryHandle lib
    = cmDynamicLoader::OpenLibrary(fullPath.c_str());
  if(!lib)
    {
    std::string err = "Attempt to load the library ";
    err += fullPath + " failed.";
    const char* error = cmsys::DynamicLoader::LastError();
    if ( error )
      {
      err += " Additional error info is:\n";
      err += error;
      }
    this->SetError(err);
    return false;
    }

  // Report what file was loaded for this command.
  this->Makefile->AddDefinition(reportVar, fullPath.c_str());

  // find the init function
  std::string initFuncName = args[0] + "Init";
  CM_INIT_FUNCTION initFunction
    = (CM_INIT_FUNCTION)
    cmsys::DynamicLoader::GetSymbolAddress(lib, initFuncName.c_str());
  if ( !initFunction )
    {
    initFuncName = "_";
    initFuncName += args[0];
    initFuncName += "Init";
    initFunction = (CM_INIT_FUNCTION)(
      cmsys::DynamicLoader::GetSymbolAddress(lib, initFuncName.c_str()));
    }
  // if the symbol is found call it to set the name on the
  // function blocker
  if(initFunction)
    {
    // create a function blocker and set it up
    cmLoadedCommand *f = new cmLoadedCommand();
    (*initFunction)(&f->info);
    this->Makefile->GetState()->AddCommand(f);
    return true;
    }
  this->SetError("Attempt to load command failed. "
                 "No init function found.");
  return false;
}

