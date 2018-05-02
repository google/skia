/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlUtil_DEFINED
#define GrMtlUtil_DEFINED

#include "GrTypes.h"

#import <Metal/Metal.h>

/**
 * Returns the Metal texture format for the given GrPixelConfig
 */
bool GrPixelConfigToMTLFormat(GrPixelConfig config, MTLPixelFormat* format);

/**
* Returns the GrPixelConfig for the given Metal texture format
*/
GrPixelConfig GrMTLFormatToPixelConfig(MTLPixelFormat format);

/**
 * Returns true if the given vulkan texture format is sRGB encoded.
 * Also provides the non-sRGB version, if there is one.
 */
bool GrMTLFormatIsSRGB(MTLPixelFormat format, MTLPixelFormat* linearFormat);

#endif
