/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlCppUtil_DEFINED
#define GrMtlCppUtil_DEFINED

#include "include/core/SkImage.h"
#include "include/gpu/mtl/GrMtlTypes.h"

class GrBackendFormat;

// Utilities that can be used from cpp files (rather than .mm).

GrMTLPixelFormat GrGetMTLPixelFormatFromMtlTextureInfo(const GrMtlTextureInfo&);

size_t GrMtlBackendFormatBytesPerBlock(const GrBackendFormat& format);

int GrMtlBackendFormatStencilBits(const GrBackendFormat& format);

uint32_t GrMtlFormatChannels(GrMTLPixelFormat);

SkImage::CompressionType GrMtlBackendFormatToCompressionType(const GrBackendFormat& format);

/**
 * Gets the sample count of a texture held by GrMtlTextureInfo or zero if the texture is nil.
 */
int GrMtlTextureInfoSampleCount(const GrMtlTextureInfo&);

#if defined(SK_DEBUG) || GR_TEST_UTILS
const char* GrMtlFormatToStr(GrMTLPixelFormat mtlFormat);
bool GrMtlFormatIsBGRA8(GrMTLPixelFormat mtlFormat);
#endif

#endif
