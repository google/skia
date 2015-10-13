/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmScriptGenerator.h"

#include "cmSystemTools.h"

//----------------------------------------------------------------------------
cmScriptGenerator
::cmScriptGenerator(const std::string& config_var,
                    std::vector<std::string> const& configurations):
  RuntimeConfigVariable(config_var),
  Configurations(configurations),
  ConfigurationName(""),
  ConfigurationTypes(0),
  ActionsPerConfig(false)
{
}

//----------------------------------------------------------------------------
cmScriptGenerator
::~cmScriptGenerator()
{
}

//----------------------------------------------------------------------------
void
cmScriptGenerator
::Generate(std::ostream& os, const std::string& config,
           std::vector<std::string> const& configurationTypes)
{
  this->ConfigurationName = config;
  this->ConfigurationTypes = &configurationTypes;
  this->GenerateScript(os);
  this->ConfigurationName = "";
  this->ConfigurationTypes = 0;
}

//----------------------------------------------------------------------------
static void cmScriptGeneratorEncodeConfig(const std::string& config,
                                          std::string& result)
{
  for(const char* c = config.c_str(); *c; ++c)
    {
    if(*c >= 'a' && *c <= 'z')
      {
      result += "[";
      result += static_cast<char>(*c + 'A' - 'a');
      result += *c;
      result += "]";
      }
    else if(*c >= 'A' && *c <= 'Z')
      {
      result += "[";
      result += *c;
      result += static_cast<char>(*c + 'a' - 'A');
      result += "]";
      }
    else
      {
      result += *c;
      }
    }
}

//----------------------------------------------------------------------------
std::string
cmScriptGenerator::CreateConfigTest(const std::string& config)
{
  std::string result = "\"${";
  result += this->RuntimeConfigVariable;
  result += "}\" MATCHES \"^(";
  if(!config.empty())
    {
    cmScriptGeneratorEncodeConfig(config, result);
    }
  result += ")$\"";
  return result;
}

//----------------------------------------------------------------------------
std::string
cmScriptGenerator::CreateConfigTest(std::vector<std::string> const& configs)
{
  std::string result = "\"${";
  result += this->RuntimeConfigVariable;
  result += "}\" MATCHES \"^(";
  const char* sep = "";
  for(std::vector<std::string>::const_iterator ci = configs.begin();
      ci != configs.end(); ++ci)
    {
    result += sep;
    sep = "|";
    cmScriptGeneratorEncodeConfig(*ci, result);
    }
  result += ")$\"";
  return result;
}

//----------------------------------------------------------------------------
void cmScriptGenerator::GenerateScript(std::ostream& os)
{
  // Track indentation.
  Indent indent;

  // Generate the script possibly with per-configuration code.
  this->GenerateScriptConfigs(os, indent);
}

//----------------------------------------------------------------------------
void cmScriptGenerator::GenerateScriptConfigs(std::ostream& os,
                                              Indent const& indent)
{
  if(this->ActionsPerConfig)
    {
    this->GenerateScriptActionsPerConfig(os, indent);
    }
  else
    {
    this->GenerateScriptActionsOnce(os, indent);
    }
}

//----------------------------------------------------------------------------
void cmScriptGenerator::GenerateScriptActions(std::ostream& os,
                                              Indent const& indent)
{
  if(this->ActionsPerConfig)
    {
    // This is reached for single-configuration build generators in a
    // per-config script generator.
    this->GenerateScriptForConfig(os, this->ConfigurationName, indent);
    }
}

//----------------------------------------------------------------------------
void cmScriptGenerator::GenerateScriptForConfig(std::ostream&,
                                                const std::string&,
                                                Indent const&)
{
  // No actions for this generator.
}

//----------------------------------------------------------------------------
bool cmScriptGenerator::GeneratesForConfig(const std::string& config)
{
  // If this is not a configuration-specific rule then we install.
  if(this->Configurations.empty())
    {
    return true;
    }

  // This is a configuration-specific rule.  Check if the config
  // matches this rule.
  std::string config_upper = cmSystemTools::UpperCase(config);
  for(std::vector<std::string>::const_iterator i =
        this->Configurations.begin();
      i != this->Configurations.end(); ++i)
    {
    if(cmSystemTools::UpperCase(*i) == config_upper)
      {
      return true;
      }
    }
  return false;
}

//----------------------------------------------------------------------------
void cmScriptGenerator::GenerateScriptActionsOnce(std::ostream& os,
                                                  Indent const& indent)
{
  if(this->Configurations.empty())
    {
    // This rule is for all configurations.
    this->GenerateScriptActions(os, indent);
    }
  else
    {
    // Generate a per-configuration block.
    std::string config_test = this->CreateConfigTest(this->Configurations);
    os << indent << "if(" << config_test << ")\n";
    this->GenerateScriptActions(os, indent.Next());
    os << indent << "endif(" << config_test << ")\n";
    }
}

//----------------------------------------------------------------------------
void cmScriptGenerator::GenerateScriptActionsPerConfig(std::ostream& os,
                                                       Indent const& indent)
{
  if(this->ConfigurationTypes->empty())
    {
    // In a single-configuration generator there is only one action
    // and it applies if the runtime-requested configuration is among
    // the rule's allowed configurations.  The configuration built in
    // the tree does not matter for this decision but will be used to
    // generate proper target file names into the code.
    this->GenerateScriptActionsOnce(os, indent);
    }
  else
    {
    // In a multi-configuration generator we produce a separate rule
    // in a block for each configuration that is built.  We restrict
    // the list of configurations to those to which this rule applies.
    bool first = true;
    for(std::vector<std::string>::const_iterator i =
          this->ConfigurationTypes->begin();
        i != this->ConfigurationTypes->end(); ++i)
      {
      const char* config = i->c_str();
      if(this->GeneratesForConfig(config))
        {
        // Generate a per-configuration block.
        std::string config_test = this->CreateConfigTest(config);
        os << indent << (first? "if(" : "elseif(") << config_test << ")\n";
        this->GenerateScriptForConfig(os, config, indent.Next());
        first = false;
        }
      }
    if(!first)
      {
      if(this->NeedsScriptNoConfig())
        {
        os << indent << "else()\n";
        this->GenerateScriptNoConfig(os, indent.Next());
        }
      os << indent << "endif()\n";
      }
    }
}
