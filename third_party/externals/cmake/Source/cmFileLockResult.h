/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2014 Ruslan Baratov

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmFileLockResult_h
#define cmFileLockResult_h

#include "cmStandardIncludes.h"

#if defined(_WIN32)
# include <windows.h> // DWORD
#endif

/**
  * @brief Result of the locking/unlocking file.
  * @note See @c cmFileLock
  */
class cmFileLockResult
{
 public:
#if defined(_WIN32)
  typedef DWORD Error;
#else
  typedef int Error;
#endif

  /**
    * @brief Successful lock/unlock.
    */
  static cmFileLockResult MakeOk();

  /**
    * @brief Lock/Unlock failed. Read error/GetLastError.
    */
  static cmFileLockResult MakeSystem();

  /**
    * @brief Lock/Unlock failed. Timeout reached.
    */
  static cmFileLockResult MakeTimeout();

  /**
    * @brief File already locked.
    */
  static cmFileLockResult MakeAlreadyLocked();

  /**
    * @brief Internal error.
    */
  static cmFileLockResult MakeInternal();

  /**
    * @brief Try to lock with function guard outside of the function
    */
  static cmFileLockResult MakeNoFunction();

  bool IsOk() const;
  std::string GetOutputMessage() const;

 private:
  enum ErrorType
  {
    OK,
    SYSTEM,
    TIMEOUT,
    ALREADY_LOCKED,
    INTERNAL,
    NO_FUNCTION
  };

  cmFileLockResult(ErrorType type, Error errorValue);

  ErrorType Type;
  Error ErrorValue;
};

#endif // cmFileLockResult_h
