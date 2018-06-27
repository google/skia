/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlTrampoline_DEFINED
#define GrMtlTrampoline_DEFINED

#include "GrTypes.h"
#include "GrTypesPriv.h"
#include "SkRefCnt.h"

class GrContext;
class GrGpu;
struct GrContextOptions;
struct GrMtlTextureInfo;
typedef unsigned int GrMTLPixelFormat;

/*
 * This class is used to hold functions which trampoline from the Ganesh cpp code to the GrMtl
 * objective-c files.
 */
class GrMtlTrampoline {
public:
    static sk_sp<GrGpu> MakeGpu(GrContext*, const GrContextOptions&, void* device, void* queue);

    static GrPixelConfig GrMTLFormatToPixelConfig(GrMTLPixelFormat format);

    static void ExtractMTLTextureInfo(const void* mtlTexture, GrMtlTextureInfo* outInfo);
};

#endif

