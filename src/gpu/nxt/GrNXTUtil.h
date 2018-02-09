/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTypes.h"
#include "nxt/nxt.h"

GrPixelConfig GrNXTFormatToPixelConfig(nxtTextureFormat format);
nxtTextureFormat GrPixelConfigToNXTFormat(GrPixelConfig config);
