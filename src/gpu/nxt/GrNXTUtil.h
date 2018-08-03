/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTypesPriv.h"
#include "nxt/nxtcpp.h"

GrPixelConfig GrNXTFormatToPixelConfig(nxt::TextureFormat format);
nxt::TextureFormat GrPixelConfigToNXTFormat(GrPixelConfig config);
