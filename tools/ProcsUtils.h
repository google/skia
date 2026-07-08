/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef ProcsUtils_DEFINED
#define ProcsUtils_DEFINED

#include "include/core/SkSerialProcs.h"

namespace ToolUtils {

// Returns the default SkDeserialProcs used by Skia's tools when Deserializing Skps. This adds
// default values for the SkDeserialImageProc and SkDeserialTypefaceProc.
SkDeserialProcs default_deserial_procs();

// Returns the default SkSerialProcs used by Skia's tools when serializing Skps. This adds
// default values for the SkSerialImageProc.
SkSerialProcs default_serial_procs();

}  // namespace ToolUtils

#endif  // ToolUtils_DEFINED
