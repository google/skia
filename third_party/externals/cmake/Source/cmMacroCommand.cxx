/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmMacroCommand.h"

#include "cmake.h"
#include "cmAlgorithms.h"

// define the class for macro commands
class cmMacroHelperCommand : public cmCommand
{
public:
  cmMacroHelperCommand() {}

  ///! clean up any memory allocated by the macro
  ~cmMacroHelperCommand() {}

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
    cmMacroHelperCommand *newC = new cmMacroHelperCommand;
    // we must copy when we clone
    newC->Args = this->Args;
    newC->Functions = this->Functions;
    newC->FilePath = this->FilePath;
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

  cmTypeMacro(cmMacroHelperCommand, cmCommand);

  std::vector<std::string> Args;
  std::vector<cmListFileFunction> Functions;
  cmPolicies::PolicyMap Policies;
  std::string FilePath;
};


bool cmMacroHelperCommand::InvokeInitialPass
(const std::vector<cmListFileArgument>& args,
 cmExecutionStatus &inStatus)
{
  // Expand the argument list to the macro.
  std::vector<std::string> expandedArgs;
  this->Makefile->ExpandArguments(args, expandedArgs);

  // make sure the number of arguments passed is at least the number
  // required by the signature
  if (expandedArgs.size() < this->Args.size() - 1)
    {
    std::string errorMsg =
      "Macro invoked with incorrect arguments for macro named: ";
    errorMsg += this->Args[0];
    this->SetError(errorMsg);
    return false;
    }

  // Enforce matching logical blocks inside the macro.
  cmMakefile::LexicalPushPop lexScope(this->Makefile);

  // Push a weak policy scope which restores the policies recorded at
  // macro creation.
  cmMakefile::PolicyPushPop polScope(this->Makefile, true, this->Policies);

  // set the value of argc
  std::ostringstream argcDefStream;
  argcDefStream << expandedArgs.size();
  std::string argcDef = argcDefStream.str();

  std::vector<std::string>::const_iterator eit
      = expandedArgs.begin() + (this->Args.size() - 1);
  std::string expandedArgn = cmJoin(cmRange(eit, expandedArgs.end()), ";");
  std::string expandedArgv = cmJoin(expandedArgs, ";");
  std::vector<std::string> variables;
  variables.reserve(this->Args.size() - 1);
  for (unsigned int j = 1; j < this->Args.size(); ++j)
    {
    variables.push_back("${" + this->Args[j] + "}");
    }
  std::vector<std::string> argVs;
  argVs.reserve(expandedArgs.size());
  char argvName[60];
  for (unsigned int j = 0; j < expandedArgs.size(); ++j)
    {
    sprintf(argvName,"${ARGV%i}",j);
    argVs.push_back(argvName);
    }
  if(!this->Functions.empty())
    {
    this->FilePath = this->Functions[0].FilePath;
    }
  // Invoke all the functions that were collected in the block.
  cmListFileFunction newLFF;
  // for each function
  for(unsigned int c = 0; c < this->Functions.size(); ++c)
    {
    // Replace the formal arguments and then invoke the command.
    newLFF.Arguments.clear();
    newLFF.Arguments.reserve(this->Functions[c].Arguments.size());
    newLFF.Name = this->Functions[c].Name;
    newLFF.FilePath = this->Functions[c].FilePath;
    newLFF.Line = this->Functions[c].Line;

    // for each argument of the current function
    for (std::vector<cmListFileArgument>::iterator k =
           this->Functions[c].Arguments.begin();
         k != this->Functions[c].Arguments.end(); ++k)
      {
      // Set the FilePath on the arguments to match the function since it is
      // not stored and the original values may be freed
      k->FilePath = this->FilePath.c_str();

      cmListFileArgument arg;
      arg.Value = k->Value;
      if(k->Delim != cmListFileArgument::Bracket)
        {
        // replace formal arguments
        for (unsigned int j = 0; j < variables.size(); ++j)
          {
          cmSystemTools::ReplaceString(arg.Value, variables[j].c_str(),
                                       expandedArgs[j].c_str());
          }
        // replace argc
        cmSystemTools::ReplaceString(arg.Value, "${ARGC}",argcDef.c_str());

        cmSystemTools::ReplaceString(arg.Value, "${ARGN}",
                                     expandedArgn.c_str());
        cmSystemTools::ReplaceString(arg.Value, "${ARGV}",
                                     expandedArgv.c_str());

        // if the current argument of the current function has ${ARGV in it
        // then try replacing ARGV values
        if (arg.Value.find("${ARGV") != std::string::npos)
          {
          for (unsigned int t = 0; t < expandedArgs.size(); ++t)
            {
            cmSystemTools::ReplaceString(arg.Value, argVs[t].c_str(),
                                         expandedArgs[t].c_str());
            }
          }
        }
      arg.Delim = k->Delim;
      arg.FilePath = k->FilePath;
      arg.Line = k->Line;
      newLFF.Arguments.push_back(arg);
      }
    cmExecutionStatus status;
    if(!this->Makefile->ExecuteCommand(newLFF, status) ||
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
      inStatus.SetReturnInvoked(true);
      return true;
      }
    if (status.GetBreakInvoked())
      {
      inStatus.SetBreakInvoked(true);
      return true;
      }
    }
  return true;
}

bool cmMacroFunctionBlocker::
IsFunctionBlocked(const cmListFileFunction& lff, cmMakefile &mf,
                  cmExecutionStatus &)
{
  // record commands until we hit the ENDMACRO
  // at the ENDMACRO call we shift gears and start looking for invocations
  if(!cmSystemTools::Strucmp(lff.Name.c_str(),"macro"))
    {
    this->Depth++;
    }
  else if(!cmSystemTools::Strucmp(lff.Name.c_str(),"endmacro"))
    {
    // if this is the endmacro for this macro then execute
    if (!this->Depth)
      {
      mf.AddMacro(this->Args[0].c_str());
      // create a new command and add it to cmake
      cmMacroHelperCommand *f = new cmMacroHelperCommand();
      f->Args = this->Args;
      f->Functions = this->Functions;
      mf.RecordPolicies(f->Policies);
      std::string newName = "_" + this->Args[0];
      mf.GetState()->RenameCommand(this->Args[0], newName);
      mf.GetState()->AddCommand(f);

      // remove the function blocker now that the macro is defined
      mf.RemoveFunctionBlocker(this, lff);
      return true;
      }
    else
      {
      // decrement for each nested macro that ends
      this->Depth--;
      }
    }

  // if it wasn't an endmacro and we are not executing then we must be
  // recording
  this->Functions.push_back(lff);
  return true;
}


bool cmMacroFunctionBlocker::
ShouldRemove(const cmListFileFunction& lff, cmMakefile &mf)
{
  if(!cmSystemTools::Strucmp(lff.Name.c_str(),"endmacro"))
    {
    std::vector<std::string> expandedArguments;
    mf.ExpandArguments(lff.Arguments, expandedArguments);
    // if the endmacro has arguments make sure they
    // match the arguments of the macro
    if ((expandedArguments.empty() ||
         (expandedArguments[0] == this->Args[0])))
      {
      return true;
      }
    }

  return false;
}

bool cmMacroCommand::InitialPass(std::vector<std::string> const& args,
                                 cmExecutionStatus &)
{
  if(args.size() < 1)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  // create a function blocker
  cmMacroFunctionBlocker *f = new cmMacroFunctionBlocker();
  f->Args.insert(f->Args.end(), args.begin(), args.end());
  this->Makefile->AddFunctionBlocker(f);
  return true;
}

