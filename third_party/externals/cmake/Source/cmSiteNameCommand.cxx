/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmSiteNameCommand.h"

#include <cmsys/RegularExpression.hxx>

// cmSiteNameCommand
bool cmSiteNameCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if(args.size() != 1 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  std::vector<std::string> paths;
  paths.push_back("/usr/bsd");
  paths.push_back("/usr/sbin");
  paths.push_back("/usr/bin");
  paths.push_back("/bin");
  paths.push_back("/sbin");
  paths.push_back("/usr/local/bin");

  const char* cacheValue
    = this->Makefile->GetDefinition(args[0]);
  if(cacheValue)
    {
    return true;
    }

  const char *temp = this->Makefile->GetDefinition("HOSTNAME");
  std::string hostname_cmd;
  if(temp)
    {
    hostname_cmd = temp;
    }
  else
    {
    hostname_cmd = cmSystemTools::FindProgram("hostname", paths);
    }

  std::string siteName = "unknown";
#if defined(_WIN32) && !defined(__CYGWIN__)
  std::string host;
  if(cmSystemTools::ReadRegistryValue
     ("HKEY_LOCAL_MACHINE\\System\\CurrentControlSet\\"
      "Control\\ComputerName\\ComputerName;ComputerName", host))
    {
    siteName = host;
    }
#else
  // try to find the hostname for this computer
  if (!cmSystemTools::IsOff(hostname_cmd.c_str()))
    {
    std::string host;
    cmSystemTools::RunSingleCommand(hostname_cmd.c_str(),
      &host, 0, 0, 0, cmSystemTools::OUTPUT_NONE);

    // got the hostname
    if (!host.empty())
      {
      // remove any white space from the host name
      std::string hostRegExp = "[ \t\n\r]*([^\t\n\r ]*)[ \t\n\r]*";
      cmsys::RegularExpression hostReg (hostRegExp.c_str());
      if (hostReg.find(host.c_str()))
        {
        // strip whitespace
        host = hostReg.match(1);
        }

      if(!host.empty())
        {
        siteName = host;
        }
      }
    }
#endif
  this->Makefile->
    AddCacheDefinition(args[0],
                       siteName.c_str(),
                       "Name of the computer/site where compile is being run",
                       cmState::STRING);

  return true;
}

