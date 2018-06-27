/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlUtil_DEFINED
#define GrMtlUtil_DEFINED

#import <Metal/Metal.h>

#include "mtl/GrMtlTypes.h"
#include "GrTypesPriv.h"


/**
 * Returns the Metal texture format for the given GrPixelConfig
 */
bool GrPixelConfigToMTLFormat(GrPixelConfig config, MTLPixelFormat* format);

/**
* Returns the GrPixelConfig for the given Metal texture format
*/
GrPixelConfig GrMTLFormatToPixelConfig(MTLPixelFormat format);

/**
 * Extracts texture info from mtlTexture and puts it into outInfo
 */
void ExtractMTLTextureInfo(const id<MTLTexture> mtlTexture, GrMtlTextureInfo* outInfo);

/**
 * Returns id<MTLTexture> from void pointer
 */
id<MTLTexture> TransferTexture(const void* mtlTexture, GrWrapOwnership);

#endif
