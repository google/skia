/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "tools/timer/Timer.h"

SkString HumanizeMs(double ms) {
    if (ms > 60e+3)  return SkStringPrintf("%.3gm", ms/60e+3);
    if (ms >  1e+3)  return SkStringPrintf("%.3gs",  ms/1e+3);
    if (ms <  1e-3)  return SkStringPrintf("%.3gns", ms*1e+6);
#ifdef SK_BUILD_FOR_WIN
    if (ms < 1)      return SkStringPrintf("%.3gus", ms*1e+3);
#else
    if (ms < 1)      return SkStringPrintf("%.3gµs", ms*1e+3);
#endif
    return SkStringPrintf("%.3gms", ms);
}
