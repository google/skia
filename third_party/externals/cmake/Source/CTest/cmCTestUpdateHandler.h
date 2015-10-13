/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmCTestUpdateHandler_h
#define cmCTestUpdateHandler_h


#include "cmCTestGenericHandler.h"
#include "cmListFileCache.h"

/** \class cmCTestUpdateHandler
 * \brief A class that handles ctest -S invocations
 *
 */
class cmCTestUpdateHandler : public cmCTestGenericHandler
{
public:
  cmTypeMacro(cmCTestUpdateHandler, cmCTestGenericHandler);

  /*
   * The main entry point for this class
   */
  int ProcessHandler();

  cmCTestUpdateHandler();

  enum {
    e_UNKNOWN = 0,
    e_CVS,
    e_SVN,
    e_BZR,
    e_GIT,
    e_HG,
    e_P4,
    e_LAST
  };

  /**
   * Initialize handler
   */
  virtual void Initialize();

private:
  // Some structures needed for update
  struct StringPair :
    public std::pair<std::string, std::string>{};
  struct UpdateFiles : public std::vector<StringPair>{};

  // Determine the type of version control
  int DetermineType(const char* cmd, const char* type);

  // The VCS command to update the working tree.
  std::string UpdateCommand;
  int UpdateType;

  int DetectVCS(const char* dir);
  bool SelectVCS();
};

#endif
