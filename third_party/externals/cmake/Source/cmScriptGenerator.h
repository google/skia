/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmScriptGenerator_h
#define cmScriptGenerator_h

#include "cmStandardIncludes.h"

class cmScriptGeneratorIndent
{
public:
  cmScriptGeneratorIndent(): Level(0) {}
  cmScriptGeneratorIndent(int level): Level(level) {}
  void Write(std::ostream& os) const
    {
    for(int i=0; i < this->Level; ++i)
      {
      os << " ";
      }
    }
  cmScriptGeneratorIndent Next(int step = 2) const
    {
    return cmScriptGeneratorIndent(this->Level + step);
    }
private:
  int Level;
};
inline std::ostream& operator<<(std::ostream& os,
                                cmScriptGeneratorIndent const& indent)
{
  indent.Write(os);
  return os;
}

/** \class cmScriptGenerator
 * \brief Support class for generating install and test scripts.
 *
 */
class cmScriptGenerator
{
public:
  cmScriptGenerator(const std::string& config_var,
                    std::vector<std::string> const& configurations);
  virtual ~cmScriptGenerator();

  void Generate(std::ostream& os, const std::string& config,
                std::vector<std::string> const& configurationTypes);

  const std::vector<std::string>& GetConfigurations() const
    { return this->Configurations; }

protected:
  typedef cmScriptGeneratorIndent Indent;
  virtual void GenerateScript(std::ostream& os);
  virtual void GenerateScriptConfigs(std::ostream& os, Indent const& indent);
  virtual void GenerateScriptActions(std::ostream& os, Indent const& indent);
  virtual void GenerateScriptForConfig(std::ostream& os,
                                       const std::string& config,
                                       Indent const& indent);
  virtual void GenerateScriptNoConfig(std::ostream&, Indent const&) {}
  virtual bool NeedsScriptNoConfig() const { return false; }

  // Test if this generator does something for a given configuration.
  bool GeneratesForConfig(const std::string&);

  std::string CreateConfigTest(const std::string& config);
  std::string CreateConfigTest(std::vector<std::string> const& configs);
  std::string CreateComponentTest(const char* component);

  // Information shared by most generator types.
  std::string RuntimeConfigVariable;
  std::vector<std::string> const Configurations;

  // Information used during generation.
  std::string ConfigurationName;
  std::vector<std::string> const* ConfigurationTypes;

  // True if the subclass needs to generate an explicit rule for each
  // configuration.  False if the subclass only generates one rule for
  // all enabled configurations.
  bool ActionsPerConfig;

private:
  void GenerateScriptActionsOnce(std::ostream& os, Indent const& indent);
  void GenerateScriptActionsPerConfig(std::ostream& os, Indent const& indent);
};

#endif
