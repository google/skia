/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmSetTargetPropertiesCommand.h"
#include "cmLocalGenerator.h"
#include "cmGlobalGenerator.h"

// cmSetTargetPropertiesCommand
bool cmSetTargetPropertiesCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if(args.size() < 2 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  // first collect up the list of files
  std::vector<std::string> propertyPairs;
  int numFiles = 0;
  std::vector<std::string>::const_iterator j;
  for(j= args.begin(); j != args.end();++j)
    {
    if(*j == "PROPERTIES")
      {
      // now loop through the rest of the arguments, new style
      ++j;
      if (std::distance(j, args.end()) % 2 != 0)
        {
        this->SetError("called with incorrect number of arguments.");
        return false;
        }
      propertyPairs.insert(propertyPairs.end(), j, args.end());
      break;
      }
    else
      {
      numFiles++;
      }
    }
  if(propertyPairs.empty())
    {
     this->SetError("called with illegal arguments, maybe missing "
                    "a PROPERTIES specifier?");
     return false;
    }

  // now loop over all the targets
  int i;
  for(i = 0; i < numFiles; ++i)
    {
    if (this->Makefile->IsAlias(args[i]))
      {
      this->SetError("can not be used on an ALIAS target.");
      return false;
      }
    bool ret = cmSetTargetPropertiesCommand::SetOneTarget
      (args[i],propertyPairs,this->Makefile);
    if (!ret)
      {
      std::string message = "Can not find target to add properties to: ";
      message += args[i];
      this->SetError(message);
      return false;
      }
    }
  return true;
}

bool cmSetTargetPropertiesCommand
::SetOneTarget(const std::string& tname,
               std::vector<std::string> &propertyPairs,
               cmMakefile *mf)
{
  if(cmTarget* target = mf->FindTargetToUse(tname))
    {
    // now loop through all the props and set them
    unsigned int k;
    for (k = 0; k < propertyPairs.size(); k = k + 2)
      {
      target->SetProperty(propertyPairs[k],
                          propertyPairs[k+1].c_str());
      target->CheckProperty(propertyPairs[k], mf);
      }
    }
  // if file is not already in the makefile, then add it
  else
    {
    return false;
    }
  return true;
}
