/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmInstallFilesCommand_h
#define cmInstallFilesCommand_h

#include "cmCommand.h"

/** \class cmInstallFilesCommand
 * \brief Specifies where to install some files
 *
 * cmInstallFilesCommand specifies the relative path where a list of
 * files should be installed.
 */
class cmInstallFilesCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    return new cmInstallFilesCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus &status);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual std::string GetName() const { return "install_files";}

  /**
   * This is called at the end after all the information
   * specified by the command is accumulated. Most commands do
   * not implement this method.  At this point, reading and
   * writing to the cache can be done.
   */
  virtual void FinalPass();
  virtual bool HasFinalPass() const { return !this->IsFilesForm; }

  /** This command is kept for compatibility with older CMake versions. */
  virtual bool IsDiscouraged() const
    {
    return true;
    }

  cmTypeMacro(cmInstallFilesCommand, cmCommand);

protected:
  void CreateInstallGenerator() const;
  std::string FindInstallSource(const char* name) const;

 private:
  std::vector<std::string> FinalArgs;
  bool IsFilesForm;
  std::string Destination;
  std::vector<std::string> Files;
};


#endif
