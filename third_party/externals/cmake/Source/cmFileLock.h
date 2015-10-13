/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2014 Ruslan Baratov

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmFileLock_h
#define cmFileLock_h

#include "cmStandardIncludes.h"

#if defined(_WIN32)
# include <windows.h> // HANDLE
#endif

class cmFileLockResult;

/**
  * @brief Cross-platform file locking.
  * @details Under the hood this class use 'fcntl' for Unix-like platforms and
  * 'LockFileEx'/'UnlockFileEx' for Win32 platform. Locks are exclusive and
  * advisory.
  */
class cmFileLock
{
 public:
  cmFileLock();
  ~cmFileLock();

  /**
    * @brief Lock the file.
    * @param timeoutSec Lock timeout. If -1 try until success or fatal error.
    */
  cmFileLockResult Lock(const std::string& filename, unsigned long timeoutSec);

  /**
    * @brief Unlock the file.
    */
  cmFileLockResult Release();

  /**
    * @brief Check file is locked by this class.
    * @details This function helps to find double locks (deadlocks) and to do
    * explicit unlocks.
    */
  bool IsLocked(const std::string& filename) const;

 private:
  cmFileLock(const cmFileLock&);
  cmFileLock& operator=(const cmFileLock&);

  cmFileLockResult OpenFile();
  cmFileLockResult LockWithoutTimeout();
  cmFileLockResult LockWithTimeout(unsigned long timeoutSec);

#if defined(_WIN32)
  typedef HANDLE FileId;
  BOOL LockFile(DWORD flags);
#else
  typedef int FileId;
  int LockFile(int cmd, int type);
#endif

  FileId File;
  std::string Filename;
};

#endif // cmFileLock_h
