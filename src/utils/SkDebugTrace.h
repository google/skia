
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkUserTrace_DEFINED
#define SkUserTrace_DEFINED

/* Sample implementation of SkUserTrace that routes all of the
   trace macros to debug output stream.
   To use this, redefine SK_USER_TRACE_INCLUDE_FILE in
   include/config/SkUserConfig.h to point to this file
*/
#define SK_TRACE_EVENT0(event) \
  SkDebugf("Trace: %s\n", event)
#define SK_TRACE_EVENT1(event, name1, value1) \
  SkDebugf("Trace: %s (%s=%s)\n", event, name1, value1)
#define SK_TRACE_EVENT2(event, name1, value1, name2, value2) \
  SkDebugf("Trace: %s (%s=%s, %s=%s)\n", event, name1, value1, name2, value2)

#endif
