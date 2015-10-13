/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmListCommand_h
#define cmListCommand_h

#include "cmCommand.h"

/** \class cmListCommand
 * \brief Common list operations
 *
 */
class cmListCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    return new cmListCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus &status);

  /**
   * This determines if the command is invoked when in script mode.
   */
  virtual bool IsScriptable() const { return true; }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual std::string GetName() const { return "list";}

  cmTypeMacro(cmListCommand, cmCommand);
protected:
  bool HandleLengthCommand(std::vector<std::string> const& args);
  bool HandleGetCommand(std::vector<std::string> const& args);
  bool HandleAppendCommand(std::vector<std::string> const& args);
  bool HandleFindCommand(std::vector<std::string> const& args);
  bool HandleInsertCommand(std::vector<std::string> const& args);
  bool HandleRemoveAtCommand(std::vector<std::string> const& args);
  bool HandleRemoveItemCommand(std::vector<std::string> const& args);
  bool HandleRemoveDuplicatesCommand(std::vector<std::string> const& args);
  bool HandleSortCommand(std::vector<std::string> const& args);
  bool HandleReverseCommand(std::vector<std::string> const& args);


  bool GetList(std::vector<std::string>& list, const std::string& var);
  bool GetListString(std::string& listString, const std::string& var);
};


#endif
