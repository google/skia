/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmQTWrapUICommand_h
#define cmQTWrapUICommand_h

#include "cmCommand.h"

#include "cmSourceFile.h"

/** \class cmQTWrapUICommand
 * \brief Create .h and .cxx files rules for Qt user interfaces files
 *
 * cmQTWrapUICommand is used to create wrappers for Qt classes into normal C++
 */
class cmQTWrapUICommand : public cmCommand
{
public:
  cmTypeMacro(cmQTWrapUICommand, cmCommand);
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    return new cmQTWrapUICommand;
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
  virtual std::string GetName() const { return "qt_wrap_ui";}
};



#endif
