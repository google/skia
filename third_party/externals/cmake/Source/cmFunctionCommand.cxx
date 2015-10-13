/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmFunctionCommand.h"

#include "cmake.h"

// define the class for function commands
class cmFunctionHelperCommand : public cmCommand
{
public:
  cmFunctionHelperCommand() {}

  ///! clean up any memory allocated by the function
  ~cmFunctionHelperCommand() {}

  /**
   * This is used to avoid including this command
   * in documentation. This is mainly used by
   * cmMacroHelperCommand and cmFunctionHelperCommand
   * which cannot provide appropriate documentation.
   */
  virtual bool ShouldAppearInDocumentation() const
    {
    return false;
    }

  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
  {
    cmFunctionHelperCommand *newC = new cmFunctionHelperCommand;
    // we must copy when we clone
    newC->Args = this->Args;
    newC->Functions = this->Functions;
    newC->Policies = this->Policies;
    return newC;
  }

  /**
   * This determines if the command is invoked when in script mode.
   */
  virtual bool IsScriptable() const { return true; }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InvokeInitialPass(const std::vector<cmListFileArgument>& args,
                                 cmExecutionStatus &);

  virtual bool InitialPass(std::vector<std::string> const&,
                           cmExecutionStatus &) { return false; }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual std::string GetName() const { return this->Args[0]; }

  cmTypeMacro(cmFunctionHelperCommand, cmCommand);

  std::vector<std::string> Args;
  std::vector<cmListFileFunction> Functions;
  cmPolicies::PolicyMap Policies;
};


bool cmFunctionHelperCommand::InvokeInitialPass
(const std::vector<cmListFileArgument>& args,
 cmExecutionStatus & inStatus)
{
  // Expand the argument list to the function.
  std::vector<std::string> expandedArgs;
  this->Makefile->ExpandArguments(args, expandedArgs);

  // make sure the number of arguments passed is at least the number
  // required by the signature
  if (expandedArgs.size() < this->Args.size() - 1)
    {
    std::string errorMsg =
      "Function invoked with incorrect arguments for function named: ";
    errorMsg += this->Args[0];
    this->SetError(errorMsg);
    return false;
    }

  // we push a scope on the makefile
  cmMakefile::ScopePushPop varScope(this->Makefile);
  cmMakefile::LexicalPushPop lexScope(this->Makefile);
  static_cast<void>(varScope);

  // Push a weak policy scope which restores the policies recorded at
  // function creation.
  cmMakefile::PolicyPushPop polScope(this->Makefile, true, this->Policies);

  // set the value of argc
  std::ostringstream strStream;
  strStream << expandedArgs.size();
  this->Makefile->AddDefinition("ARGC",strStream.str().c_str());
  this->Makefile->MarkVariableAsUsed("ARGC");

  // set the values for ARGV0 ARGV1 ...
  for (unsigned int t = 0; t < expandedArgs.size(); ++t)
    {
    std::ostringstream tmpStream;
    tmpStream << "ARGV" << t;
    this->Makefile->AddDefinition(tmpStream.str(),
                                  expandedArgs[t].c_str());
    this->Makefile->MarkVariableAsUsed(tmpStream.str());
    }

  // define the formal arguments
  for (unsigned int j = 1; j < this->Args.size(); ++j)
    {
    this->Makefile->AddDefinition(this->Args[j],
                                  expandedArgs[j-1].c_str());
    }

  // define ARGV and ARGN
  std::string argvDef = cmJoin(expandedArgs, ";");
  std::vector<std::string>::const_iterator eit
      = expandedArgs.begin() + (this->Args.size()-1);
  std::string argnDef = cmJoin(cmRange(eit, expandedArgs.end()), ";");
  this->Makefile->AddDefinition("ARGV", argvDef.c_str());
  this->Makefile->MarkVariableAsUsed("ARGV");
  this->Makefile->AddDefinition("ARGN", argnDef.c_str());
  this->Makefile->MarkVariableAsUsed("ARGN");

  // Invoke all the functions that were collected in the block.
  // for each function
  for(unsigned int c = 0; c < this->Functions.size(); ++c)
    {
    cmExecutionStatus status;
    if (!this->Makefile->ExecuteCommand(this->Functions[c],status) ||
        status.GetNestedError())
      {
      // The error message should have already included the call stack
      // so we do not need to report an error here.
      lexScope.Quiet();
      polScope.Quiet();
      inStatus.SetNestedError(true);
      return false;
      }
    if (status.GetReturnInvoked())
      {
      return true;
      }
    }

  // pop scope on the makefile
  return true;
}

bool cmFunctionFunctionBlocker::
IsFunctionBlocked(const cmListFileFunction& lff, cmMakefile &mf,
                  cmExecutionStatus &)
{
  // record commands until we hit the ENDFUNCTION
  // at the ENDFUNCTION call we shift gears and start looking for invocations
  if(!cmSystemTools::Strucmp(lff.Name.c_str(),"function"))
    {
    this->Depth++;
    }
  else if(!cmSystemTools::Strucmp(lff.Name.c_str(),"endfunction"))
    {
    // if this is the endfunction for this function then execute
    if (!this->Depth)
      {
      // create a new command and add it to cmake
      cmFunctionHelperCommand *f = new cmFunctionHelperCommand();
      f->Args = this->Args;
      f->Functions = this->Functions;
      mf.RecordPolicies(f->Policies);

      // Set the FilePath on the arguments to match the function since it is
      // not stored and the original values may be freed
      for (unsigned int i = 0; i < f->Functions.size(); ++i)
        {
        for (unsigned int j = 0; j < f->Functions[i].Arguments.size(); ++j)
          {
          f->Functions[i].Arguments[j].FilePath =
            f->Functions[i].FilePath.c_str();
          }
        }

      std::string newName = "_" + this->Args[0];
      mf.GetState()->RenameCommand(this->Args[0], newName);
      mf.GetState()->AddCommand(f);

      // remove the function blocker now that the function is defined
      mf.RemoveFunctionBlocker(this, lff);
      return true;
      }
    else
      {
      // decrement for each nested function that ends
      this->Depth--;
      }
    }

  // if it wasn't an endfunction and we are not executing then we must be
  // recording
  this->Functions.push_back(lff);
  return true;
}


bool cmFunctionFunctionBlocker::
ShouldRemove(const cmListFileFunction& lff, cmMakefile &mf)
{
  if(!cmSystemTools::Strucmp(lff.Name.c_str(),"endfunction"))
    {
    std::vector<std::string> expandedArguments;
    mf.ExpandArguments(lff.Arguments, expandedArguments);
    // if the endfunction has arguments then make sure
    // they match the ones in the opening function command
    if ((expandedArguments.empty() ||
         (expandedArguments[0] == this->Args[0])))
      {
      return true;
      }
    }

  return false;
}

bool cmFunctionCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if(args.size() < 1)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  // create a function blocker
  cmFunctionFunctionBlocker *f = new cmFunctionFunctionBlocker();
  f->Args.insert(f->Args.end(), args.begin(), args.end());
  this->Makefile->AddFunctionBlocker(f);
  return true;
}

