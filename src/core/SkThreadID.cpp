/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkThreadID.h"

#ifdef SK_BUILD_FOR_WIN
    #include <windows.h>
    SkThreadID SkGetThreadID() { return GetCurrentThreadId(); }
#else
    #include <pthread.h>
    SkThreadID SkGetThreadID() { return (int64_t)pthread_self(); }
#endif
