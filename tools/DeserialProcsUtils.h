/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef DeserialProcsUtils_DEFINED
#define DeserialProcsUtils_DEFINED

#include "include/core/SkSerialProcs.h"

namespace ToolUtils {

// Returns the default SkDeserialProcs used by Skia's tools when Deserializing Skps. This adds
// default values for the SkDeserialImageProc and SkDeserialTypefaceProc..
SkDeserialProcs get_default_skp_deserial_procs();

}  // namespace ToolUtils

#endif  // ToolUtils_DEFINED
