/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSLTraceUtils_DEFINED
#define SkSLTraceUtils_DEFINED

#include "include/core/SkRefCnt.h"

class SkStream;
class SkWStream;

namespace SkSL { class DebugTracePriv; }

namespace SkSLTraceUtils {
sk_sp<SkSL::DebugTracePriv> ReadTrace(SkStream*);
void WriteTrace(const SkSL::DebugTracePriv&, SkWStream*);
}

#endif
