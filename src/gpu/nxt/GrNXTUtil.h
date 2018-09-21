/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTypesPriv.h"
#include "dawn/dawncpp.h"

GrPixelConfig GrNXTFormatToPixelConfig(dawn::TextureFormat format);
dawn::TextureFormat GrPixelConfigToNXTFormat(GrPixelConfig config);
