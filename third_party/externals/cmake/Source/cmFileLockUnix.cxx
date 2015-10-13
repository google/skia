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

#include <errno.h> // errno
#include <stdio.h> // SEEK_SET
#include <fcntl.h>
#include <unistd.h>
#include "cmSystemTools.h"

cmFileLock::cmFileLock(): File(-1)
{
}

cmFileLockResult cmFileLock::Release()
{
  if (this->Filename.empty())
    {
    return cmFileLockResult::MakeOk();
    }
  const int lockResult = this->LockFile(F_SETLK, F_UNLCK);

  this->Filename = "";

  ::close(this->File);
  this->File = -1;

  if (lockResult == 0)
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
  this->File = ::open(this->Filename.c_str(), O_RDWR);
  if (this->File == -1)
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
  if (this->LockFile(F_SETLKW, F_WRLCK) == -1)
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
  while (true)
    {
    if (this->LockFile(F_SETLK, F_WRLCK) == -1)
      {
      if (errno != EACCES && errno != EAGAIN)
        {
        return cmFileLockResult::MakeSystem();
        }
      }
    else
      {
      return cmFileLockResult::MakeOk();
      }
    if (seconds == 0)
      {
      return cmFileLockResult::MakeTimeout();
      }
    --seconds;
    cmSystemTools::Delay(1000);
    }
}

int cmFileLock::LockFile(int cmd, int type)
{
  struct ::flock lock;
  lock.l_start = 0;
  lock.l_len = 0; // lock all bytes
  lock.l_pid = 0; // unused (for F_GETLK only)
  lock.l_type = static_cast<short>(type); // exclusive lock
  lock.l_whence = SEEK_SET;
  return ::fcntl(this->File, cmd, &lock);
}
