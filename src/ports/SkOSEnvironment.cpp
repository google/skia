
/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOSEnvironment.h"
#include <stdlib.h>

void sk_setenv(const char* key, const char* value) {
#ifdef SK_BUILD_FOR_WIN32
    _putenv_s(key, value);
#else
    setenv(key, value, 1);
#endif
}

