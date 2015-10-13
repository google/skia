/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmInstallCommandArguments_h
#define cmInstallCommandArguments_h

#include "cmStandardIncludes.h"
#include "cmCommandArgumentsHelper.h"

class cmInstallCommandArguments
{
  public:
    cmInstallCommandArguments(const std::string& defaultComponent);
    void SetGenericArguments(cmInstallCommandArguments* args)
                                               {this->GenericArguments = args;}
    void Parse(const std::vector<std::string>* args,
               std::vector<std::string>* unconsumedArgs);

    // Compute destination path.and check permissions
    bool Finalize();

    const std::string& GetDestination() const;
    const std::string& GetComponent() const;
    const std::string& GetRename() const;
    const std::string& GetPermissions() const;
    const std::vector<std::string>& GetConfigurations() const;
    bool GetOptional() const;
    bool GetNamelinkOnly() const;
    bool GetNamelinkSkip() const;

    // once HandleDirectoryMode() is also switched to using
    // cmInstallCommandArguments then these two functions can become non-static
    // private member functions without arguments
    static bool CheckPermissions(const std::string& onePerm,
                                 std::string& perm);
    cmCommandArgumentsHelper Parser;
    cmCommandArgumentGroup ArgumentGroup;
  private:
    cmInstallCommandArguments(); // disabled
    cmCAString Destination;
    cmCAString Component;
    cmCAString Rename;
    cmCAStringVector Permissions;
    cmCAStringVector Configurations;
    cmCAEnabler Optional;
    cmCAEnabler NamelinkOnly;
    cmCAEnabler NamelinkSkip;

    std::string DestinationString;
    std::string PermissionsString;

    cmInstallCommandArguments* GenericArguments;
    static const char* PermissionsTable[];
    static const std::string EmptyString;
    std::string DefaultComponentName;
    bool CheckPermissions();
};

class cmInstallCommandIncludesArgument
{
  public:
    cmInstallCommandIncludesArgument();
    void Parse(const std::vector<std::string>* args,
               std::vector<std::string>* unconsumedArgs);

    const std::vector<std::string>& GetIncludeDirs() const;

  private:
    std::vector<std::string> IncludeDirs;
};

#endif
