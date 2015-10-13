/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmOutputRequiredFilesCommand_h
#define cmOutputRequiredFilesCommand_h

#include "cmCommand.h"
#include "cmMakeDepend.h"

class cmOutputRequiredFilesCommand : public cmCommand
{
public:
  cmTypeMacro(cmOutputRequiredFilesCommand, cmCommand);
  virtual cmCommand* Clone() { return new cmOutputRequiredFilesCommand; }
  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus &status);
  virtual std::string GetName() const { return "output_required_files";}
  virtual bool IsDiscouraged() const { return true; }

  void ListDependencies(cmDependInformation const *info,
                        FILE *fout,
                        std::set<cmDependInformation const*> *visited);
private:
  std::string File;
  std::string OutputFile;
};



#endif
