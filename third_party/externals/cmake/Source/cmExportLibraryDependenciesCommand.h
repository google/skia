/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmExportLibraryDependenciesCommand_h
#define cmExportLibraryDependenciesCommand_h

#include "cmCommand.h"

class cmExportLibraryDependenciesCommand : public cmCommand
{
public:
  cmTypeMacro(cmExportLibraryDependenciesCommand, cmCommand);
  virtual cmCommand* Clone() { return new cmExportLibraryDependenciesCommand; }
  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus &status);
  virtual std::string GetName() const { return "export_library_dependencies";}
  virtual bool IsDiscouraged() const { return true; }

  virtual void FinalPass();
  virtual bool HasFinalPass() const { return true; }

private:
  std::string Filename;
  bool Append;
  void ConstFinalPass() const;
};


#endif
