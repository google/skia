/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2014 Ruslan Baratov

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include "cmFileLock.h"

#include <windows.h> // CreateFileW
#include "cmSystemTools.h"

cmFileLock::cmFileLock(): File(INVALID_HANDLE_VALUE)
{
}

cmFileLockResult cmFileLock::Release()
{
  if (this->Filename.empty())
    {
    return cmFileLockResult::MakeOk();
    }
  const unsigned long len = static_cast<unsigned long>(-1);
  static OVERLAPPED overlapped;
  const DWORD reserved = 0;
  const BOOL unlockResult = UnlockFileEx(
      File,
      reserved,
      len,
      len,
      &overlapped
  );

  this->Filename = "";

  CloseHandle(this->File);
  this->File = INVALID_HANDLE_VALUE;

  if (unlockResult)
    {
    return cmFileLockResult::MakeOk();
    }
  else
    {
    return cmFileLockResult::MakeSystem();
    }
}

cmFileLockResult cmFileLock::OpenFile()
{
  const DWORD access = GENERIC_READ | GENERIC_WRITE;
  const DWORD shareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
  const PSECURITY_ATTRIBUTES security = NULL;
  const DWORD attr = 0;
  const HANDLE templ = NULL;
  this->File = CreateFileW(
      cmSystemTools::ConvertToWindowsExtendedPath(this->Filename).c_str(),
      access,
      shareMode,
      security,
      OPEN_EXISTING,
      attr,
      templ
  );
  if (this->File == INVALID_HANDLE_VALUE)
    {
    return cmFileLockResult::MakeSystem();
    }
  else
    {
    return cmFileLockResult::MakeOk();
    }
}

cmFileLockResult cmFileLock::LockWithoutTimeout()
{
  if (!this->LockFile(LOCKFILE_EXCLUSIVE_LOCK))
    {
    return cmFileLockResult::MakeSystem();
    }
  else
    {
    return cmFileLockResult::MakeOk();
    }
}

cmFileLockResult cmFileLock::LockWithTimeout(unsigned long seconds)
{
  const DWORD flags = LOCKFILE_EXCLUSIVE_LOCK | LOCKFILE_FAIL_IMMEDIATELY;
  while (true)
    {
    const BOOL result = this->LockFile(flags);
    if (result)
      {
      return cmFileLockResult::MakeOk();
      }
    const DWORD error = GetLastError();
    if (error != ERROR_LOCK_VIOLATION)
      {
      return cmFileLockResult::MakeSystem();
      }
    if (seconds == 0)
      {
      return cmFileLockResult::MakeTimeout();
      }
    --seconds;
    cmSystemTools::Delay(1000);
    }
}

BOOL cmFileLock::LockFile(DWORD flags)
{
  const DWORD reserved = 0;
  const unsigned long len = static_cast<unsigned long>(-1);
  static OVERLAPPED overlapped;
  return LockFileEx(
      this->File,
      flags,
      reserved,
      len,
      len,
      &overlapped
  );
}
