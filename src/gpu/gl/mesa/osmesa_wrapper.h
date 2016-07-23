/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Older versions of XQuartz have a bug where a header included by osmesa.h
// defines GL_GLEXT_PROTOTYPES. This will cause a redefinition warning if
// the file that includes osmesa.h already defined it. XCode 3 uses a version
// of gcc (4.2.1) that does not support the diagnostic pragma to disable a
// warning (added in 4.2.4). So we use the system_header pragma to shut GCC
// up about warnings in osmesa.h
#pragma GCC system_header
#include <GL/osmesa.h>
