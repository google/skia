/*
 * Copyright 2011 Google Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// Simple platform thread implementation used to test our cross-platform locks.
// This is a trimmed down version of Chromium base/threading/platform_thread.h.

#ifndef SFNTLY_CPP_SRC_TEST_PLATFORM_THREAD_H_
#define SFNTLY_CPP_SRC_TEST_PLATFORM_THREAD_H_

#if defined (WIN32)
#include <windows.h>
#else  // Assume pthread
#include <errno.h>
#include <pthread.h>
#include <time.h>
#endif // if defined (WIN32)

#include "sfntly/port/type.h"

namespace sfntly {

#if defined (WIN32)
typedef HANDLE PlatformThreadHandle;
const PlatformThreadHandle kNullThreadHandle = NULL;
#else  // Assume pthread
typedef pthread_t PlatformThreadHandle;
const PlatformThreadHandle kNullThreadHandle = 0;
#endif

class PlatformThread {
 public:
  class Delegate {
   public:
     virtual ~Delegate() {}
     virtual void ThreadMain() = 0;
  };

  // Sleeps for the specified duration (units are milliseconds).
  static void Sleep(int32_t duration_ms);

  // Creates a new thread using default stack size.  Upon success,
  // |*thread_handle| will be assigned a handle to the newly created thread,
  // and |delegate|'s ThreadMain method will be executed on the newly created
  // thread.
  // NOTE: When you are done with the thread handle, you must call Join to
  // release system resources associated with the thread.  You must ensure that
  // the Delegate object outlives the thread.
  static bool Create(Delegate* delegate, PlatformThreadHandle* thread_handle);

  // Joins with a thread created via the Create function.  This function blocks
  // the caller until the designated thread exits.  This will invalidate
  // |thread_handle|.
  static void Join(PlatformThreadHandle thread_handle);

private:
  PlatformThread() {}
  NO_COPY_AND_ASSIGN(PlatformThread);
};

}  // namespace sfntly

#endif  // SFNTLY_CPP_SRC_TEST_PLATFORM_THREAD_H_
