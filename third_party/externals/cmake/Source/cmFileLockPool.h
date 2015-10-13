/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2014 Ruslan Baratov

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmFileLockPool_h
#define cmFileLockPool_h

#include "cmStandardIncludes.h"

#include <list>

class cmFileLockResult;
class cmFileLock;

class cmFileLockPool
{
 public:
  cmFileLockPool();
  ~cmFileLockPool();

  //@{
  /**
    * @brief Function scope control.
    */
  void PushFunctionScope();
  void PopFunctionScope();
  //@}

  //@{
  /**
    * @brief File scope control.
    */
  void PushFileScope();
  void PopFileScope();
  //@}

  //@{
  /**
    * @brief Lock the file in given scope.
    * @param timeoutSec Lock timeout. If -1 try until success or fatal error.
    */
  cmFileLockResult LockFunctionScope(
      const std::string& filename, unsigned long timeoutSec
  );
  cmFileLockResult LockFileScope(
      const std::string& filename, unsigned long timeoutSec
  );
  cmFileLockResult LockProcessScope(
      const std::string& filename, unsigned long timeoutSec
  );
  //@}

  /**
    * @brief Unlock the file explicitly.
    */
  cmFileLockResult Release(const std::string& filename);

 private:
  cmFileLockPool(const cmFileLockPool&);
  cmFileLockPool& operator=(const cmFileLockPool&);

  bool IsAlreadyLocked(const std::string& filename) const;

  class ScopePool
  {
   public:
    ScopePool();
    ~ScopePool();

    cmFileLockResult Lock(
        const std::string& filename, unsigned long timeoutSec
    );
    cmFileLockResult Release(const std::string& filename);
    bool IsAlreadyLocked(const std::string& filename) const;

   private:
    ScopePool(const ScopePool&);
    ScopePool& operator=(const ScopePool&);

    typedef std::list<cmFileLock*> List;
    typedef List::iterator It;
    typedef List::const_iterator CIt;

    List Locks;
  };

  typedef std::list<ScopePool*> List;

  typedef List::iterator It;
  typedef List::const_iterator CIt;

  List FunctionScopes;
  List FileScopes;
  ScopePool ProcessScope;
};

#endif // cmFileLockPool_h
