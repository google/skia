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

#include "test/platform_thread.h"

namespace sfntly {

#if defined (WIN32)

DWORD __stdcall ThreadFunc(void* params) {
  PlatformThread::Delegate* delegate =
      static_cast<PlatformThread::Delegate*>(params);
  delegate->ThreadMain();
  return 0;
}

// static
bool PlatformThread::Create(Delegate* delegate,
                            PlatformThreadHandle* thread_handle) {
  assert(thread_handle);
  *thread_handle = CreateThread(NULL, 0, ThreadFunc, delegate, 0, NULL);
  if (!(*thread_handle)) {
    return false;
  }

  return true;
}

// static
void PlatformThread::Join(PlatformThreadHandle thread_handle) {
  assert(thread_handle);
  DWORD result = WaitForSingleObject(thread_handle, INFINITE);
  assert(result == WAIT_OBJECT_0);
  CloseHandle(thread_handle);
}

// static
void PlatformThread::Sleep(int32_t duration_ms) {
  ::Sleep(duration_ms);
}

#else

void* ThreadFunc(void* params) {
  PlatformThread::Delegate* delegate =
      static_cast<PlatformThread::Delegate*>(params);
  delegate->ThreadMain();
  return NULL;
}

// static
bool PlatformThread::Create(Delegate* delegate,
                            PlatformThreadHandle* thread_handle) {
  assert(thread_handle);

  bool success = false;
  pthread_attr_t attributes;
  pthread_attr_init(&attributes);
  success = !pthread_create(thread_handle, &attributes, ThreadFunc, delegate);
  pthread_attr_destroy(&attributes);

  return success;
}

// static
void PlatformThread::Join(PlatformThreadHandle thread_handle) {
  assert(thread_handle);
  pthread_join(thread_handle, NULL);
}

// static
void PlatformThread::Sleep(int32_t duration_ms) {
  struct timespec sleep_time, remaining;

  // Contains the portion of duration_ms >= 1 sec.
  sleep_time.tv_sec = duration_ms / 1000;
  duration_ms -= sleep_time.tv_sec * 1000;

  // Contains the portion of duration_ms < 1 sec.
  sleep_time.tv_nsec = duration_ms * 1000 * 1000;  // nanoseconds.

  while (nanosleep(&sleep_time, &remaining) == -1 && errno == EINTR)
    sleep_time = remaining;
}

#endif  // WIN32

}  // namespace sfntly
