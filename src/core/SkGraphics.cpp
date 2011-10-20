
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkGraphics.h"

#include "Sk64.h"
#include "SkBlitter.h"
#include "SkCanvas.h"
#include "SkFloat.h"
#include "SkGeometry.h"
#include "SkGlobals.h"
#include "SkMath.h"
#include "SkMatrix.h"
#include "SkPath.h"
#include "SkPathEffect.h"
#include "SkRandom.h"
#include "SkRefCnt.h"
#include "SkScalerContext.h"
#include "SkShader.h"
#include "SkStream.h"
#include "SkTSearch.h"
#include "SkTime.h"
#include "SkUtils.h"
#include "SkXfermode.h"

#define typesizeline(type)  { #type , sizeof(type) }

#ifdef BUILD_EMBOSS_TABLE
    extern void SkEmbossMask_BuildTable();
#endif

#ifdef BUILD_RADIALGRADIENT_TABLE
    extern void SkRadialGradient_BuildTable();
#endif

void SkGraphics::Init() {
    SkGlobals::Init();

#ifdef BUILD_EMBOSS_TABLE
    SkEmbossMask_BuildTable();
#endif
#ifdef BUILD_RADIALGRADIENT_TABLE
    SkRadialGradient_BuildTable();
#endif

#ifdef SK_DEBUGx
    int i;

    static const struct {
        const char* fTypeName;
        size_t      fSizeOf;
    } gTypeSize[] = {
        typesizeline(char),
        typesizeline(short),
        typesizeline(int),
        typesizeline(long),
        typesizeline(size_t),
        typesizeline(void*),

        typesizeline(S8CPU),
        typesizeline(U8CPU),
        typesizeline(S16CPU),
        typesizeline(U16CPU),

        typesizeline(SkPoint),
        typesizeline(SkRect),
        typesizeline(SkMatrix),
        typesizeline(SkPath),
        typesizeline(SkGlyph),
        typesizeline(SkRefCnt),

        typesizeline(SkPaint),
        typesizeline(SkCanvas),
        typesizeline(SkBlitter),
        typesizeline(SkShader),
        typesizeline(SkXfermode),
        typesizeline(SkPathEffect)
    };

#ifdef SK_CPU_BENDIAN
    SkDebugf("SkGraphics: big-endian\n");
#else
    SkDebugf("SkGraphics: little-endian\n");
#endif

    {
        char    test = 0xFF;
        int     itest = test;   // promote to int, see if it sign-extended
        if (itest < 0)
            SkDebugf("SkGraphics: char is signed\n");
        else
            SkDebugf("SkGraphics: char is unsigned\n");
    }
    for (i = 0; i < (int)SK_ARRAY_COUNT(gTypeSize); i++) {
        SkDebugf("SkGraphics: sizeof(%s) = %d\n",
                 gTypeSize[i].fTypeName, gTypeSize[i].fSizeOf);
    }

#endif
}

///////////////////////////////////////////////////////////////////////////////

#include "SkGlyphCache.h"
#include "SkTypefaceCache.h"

void SkGraphics::Term() {
    SkGraphics::SetFontCacheUsed(0);
    SkTypefaceCache::PurgeAll();
    SkGlobals::Term();
}

size_t SkGraphics::GetFontCacheUsed() {
    return SkGlyphCache::GetCacheUsed();
}

bool SkGraphics::SetFontCacheUsed(size_t usageInBytes) {
    return SkGlyphCache::SetCacheUsed(usageInBytes);
}

void SkGraphics::GetVersion(int32_t* major, int32_t* minor, int32_t* patch) {
    if (major) {
        *major = SKIA_VERSION_MAJOR;
    }
    if (minor) {
        *minor = SKIA_VERSION_MINOR;
    }
    if (patch) {
        *patch = SKIA_VERSION_PATCH;
    }
}

