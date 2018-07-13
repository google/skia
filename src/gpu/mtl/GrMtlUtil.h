/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlUtil_DEFINED
#define GrMtlUtil_DEFINED

#import <Metal/Metal.h>

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
 * Returns a id<MTLTexture> to the MTLTexture pointed at by the const void*. Will use
 * __bridge_transfer if we are adopting ownership.
 */
id<MTLTexture> GrGetMTLTexture(const void* mtlTexture, GrWrapOwnership);

/**
 * Returns a const void* to whatever the id object is pointing to. Always uses __bridge.
 */
const void* GrGetPtrFromId(id idObject);

/**
 * Returns a const void* to whatever the id object is pointing to. Always uses __bridge_retained.
 */
const void* GrReleaseId(id idObject);

/**
 * Returns a MTLTextureDescriptor which describes the MTLTexture. Useful when creating a duplicate
 * MTLTexture without the same storage allocation.
 */
MTLTextureDescriptor* GrGetMTLTextureDescriptor(id<MTLTexture> mtlTexture);

#endif
