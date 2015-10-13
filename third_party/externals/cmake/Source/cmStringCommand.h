/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmStringCommand_h
#define cmStringCommand_h

#include "cmCommand.h"

class cmMakefile;
namespace cmsys
{
  class RegularExpression;
}

/** \class cmStringCommand
 * \brief Common string operations
 *
 */
class cmStringCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    return new cmStringCommand;
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
  virtual std::string GetName() const { return "string";}

  cmTypeMacro(cmStringCommand, cmCommand);
protected:
  bool HandleConfigureCommand(std::vector<std::string> const& args);
  bool HandleAsciiCommand(std::vector<std::string> const& args);
  bool HandleRegexCommand(std::vector<std::string> const& args);
  bool RegexMatch(std::vector<std::string> const& args);
  bool RegexMatchAll(std::vector<std::string> const& args);
  bool RegexReplace(std::vector<std::string> const& args);
  bool HandleHashCommand(std::vector<std::string> const& args);
  bool HandleToUpperLowerCommand(std::vector<std::string> const& args,
                                 bool toUpper);
  bool HandleCompareCommand(std::vector<std::string> const& args);
  bool HandleReplaceCommand(std::vector<std::string> const& args);
  bool HandleLengthCommand(std::vector<std::string> const& args);
  bool HandleSubstringCommand(std::vector<std::string> const& args);
  bool HandleConcatCommand(std::vector<std::string> const& args);
  bool HandleStripCommand(std::vector<std::string> const& args);
  bool HandleRandomCommand(std::vector<std::string> const& args);
  bool HandleFindCommand(std::vector<std::string> const& args);
  bool HandleTimestampCommand(std::vector<std::string> const& args);
  bool HandleMakeCIdentifierCommand(std::vector<std::string> const& args);
  bool HandleGenexStripCommand(std::vector<std::string> const& args);
  bool HandleUuidCommand(std::vector<std::string> const& args);

  class RegexReplacement
  {
  public:
    RegexReplacement(const char* s): number(-1), value(s) {}
    RegexReplacement(const std::string& s): number(-1), value(s) {}
    RegexReplacement(int n): number(n), value() {}
    RegexReplacement() {}
    int number;
    std::string value;
  };
};


#endif
