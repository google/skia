/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)

#ifdef SK_BUILD_FOR_MAC
#import <ApplicationServices/ApplicationServices.h>
#endif

#ifdef SK_BUILD_FOR_IOS
#include <CoreText/CoreText.h>
#include <CoreText/CTFontManager.h>
#include <CoreGraphics/CoreGraphics.h>
#include <CoreFoundation/CoreFoundation.h>
#endif

#include "include/core/SkFontMetrics.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/ports/SkTypeface_mac.h"
#include "include/private/SkColorData.h"
#include "include/private/SkFloatingPoint.h"
#include "include/private/SkMutex.h"
#include "include/private/SkOnce.h"
#include "include/private/SkTemplates.h"
#include "include/private/SkTo.h"
#include "include/utils/mac/SkCGUtils.h"
#include "src/core/SkAdvancedTypefaceMetrics.h"
#include "src/core/SkAutoMalloc.h"
#include "src/core/SkDescriptor.h"
#include "src/core/SkEndian.h"
#include "src/core/SkFontDescriptor.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkMakeUnique.h"
#include "src/core/SkMaskGamma.h"
#include "src/core/SkMathPriv.h"
#include "src/core/SkTypefaceCache.h"
#include "src/core/SkUtils.h"
#include "src/sfnt/SkOTTable_OS_2.h"
#include "src/sfnt/SkOTUtils.h"
#include "src/sfnt/SkSFNTHeader.h"
#include "src/utils/SkUTF.h"
#include "src/utils/mac/SkUniqueCFRef.h"

#include <dlfcn.h>

#include <utility>

// Experimental code to use a global lock whenever we access CG, to see if this reduces
// crashes in Chrome
#define USE_GLOBAL_MUTEX_FOR_CG_ACCESS

#ifdef USE_GLOBAL_MUTEX_FOR_CG_ACCESS
    SK_DECLARE_STATIC_MUTEX(gCGMutex);
    #define AUTO_CG_LOCK()  SkAutoMutexAcquire amc(gCGMutex)
#else
    #define AUTO_CG_LOCK()
#endif

// Set to make glyph bounding boxes visible.
#define SK_SHOW_TEXT_BLIT_COVERAGE 0

CTFontRef SkTypeface_GetCTFontRef(const SkTypeface* face) {
    return face ? (CTFontRef)face->internal_private_getCTFontRef() : nullptr;
}

class SkScalerContext_Mac;

static SkUniqueCFRef<CFStringRef> make_CFString(const char s[]) {
    return SkUniqueCFRef<CFStringRef>(CFStringCreateWithCString(nullptr, s, kCFStringEncodingUTF8));
}

// inline versions of these rect helpers

static bool CGRectIsEmpty_inline(const CGRect& rect) {
    return rect.size.width <= 0 || rect.size.height <= 0;
}

static CGFloat CGRectGetMinX_inline(const CGRect& rect) {
    return rect.origin.x;
}

static CGFloat CGRectGetMaxX_inline(const CGRect& rect) {
    return rect.origin.x + rect.size.width;
}

static CGFloat CGRectGetMinY_inline(const CGRect& rect) {
    return rect.origin.y;
}

static CGFloat CGRectGetMaxY_inline(const CGRect& rect) {
    return rect.origin.y + rect.size.height;
}

static CGFloat CGRectGetWidth_inline(const CGRect& rect) {
    return rect.size.width;
}

///////////////////////////////////////////////////////////////////////////////

static void sk_memset_rect32(uint32_t* ptr, uint32_t value,
                             int width, int height, size_t rowBytes) {
    SkASSERT(width);
    SkASSERT(width * sizeof(uint32_t) <= rowBytes);

    if (width >= 32) {
        while (height) {
            sk_memset32(ptr, value, width);
            ptr = (uint32_t*)((char*)ptr + rowBytes);
            height -= 1;
        }
        return;
    }

    rowBytes -= width * sizeof(uint32_t);

    if (width >= 8) {
        while (height) {
            int w = width;
            do {
                *ptr++ = value; *ptr++ = value;
                *ptr++ = value; *ptr++ = value;
                *ptr++ = value; *ptr++ = value;
                *ptr++ = value; *ptr++ = value;
                w -= 8;
            } while (w >= 8);
            while (--w >= 0) {
                *ptr++ = value;
            }
            ptr = (uint32_t*)((char*)ptr + rowBytes);
            height -= 1;
        }
    } else {
        while (height) {
            int w = width;
            do {
                *ptr++ = value;
            } while (--w > 0);
            ptr = (uint32_t*)((char*)ptr + rowBytes);
            height -= 1;
        }
    }
}

typedef uint32_t CGRGBPixel;

static unsigned CGRGBPixel_getAlpha(CGRGBPixel pixel) {
    return pixel & 0xFF;
}

static CGFloat ScalarToCG(SkScalar scalar) {
    if (sizeof(CGFloat) == sizeof(float)) {
        return SkScalarToFloat(scalar);
    } else {
        SkASSERT(sizeof(CGFloat) == sizeof(double));
        return (CGFloat) SkScalarToDouble(scalar);
    }
}

static SkScalar CGToScalar(CGFloat cgFloat) {
    if (sizeof(CGFloat) == sizeof(float)) {
        return SkFloatToScalar(cgFloat);
    } else {
        SkASSERT(sizeof(CGFloat) == sizeof(double));
        return SkDoubleToScalar(cgFloat);
    }
}

static float CGToFloat(CGFloat cgFloat) {
    if (sizeof(CGFloat) == sizeof(float)) {
        return cgFloat;
    } else {
        SkASSERT(sizeof(CGFloat) == sizeof(double));
        return static_cast<float>(cgFloat);
    }
}

static CGAffineTransform MatrixToCGAffineTransform(const SkMatrix& matrix) {
    return CGAffineTransformMake( ScalarToCG(matrix[SkMatrix::kMScaleX]),
                                 -ScalarToCG(matrix[SkMatrix::kMSkewY] ),
                                 -ScalarToCG(matrix[SkMatrix::kMSkewX] ),
                                  ScalarToCG(matrix[SkMatrix::kMScaleY]),
                                  ScalarToCG(matrix[SkMatrix::kMTransX]),
                                  ScalarToCG(matrix[SkMatrix::kMTransY]));
}

///////////////////////////////////////////////////////////////////////////////

#define BITMAP_INFO_RGB (kCGImageAlphaNoneSkipFirst | kCGBitmapByteOrder32Host)

/** Drawn in FontForge, reduced with fonttools ttx, converted by xxd -i,
 *  this TrueType font contains a glyph of the spider.
 *
 *  To re-forge the original bytes of the TrueType font file,
 *  remove all ',|( +0x)' from this definition,
 *  copy the data to the clipboard,
 *  run 'pbpaste | xxd -p -r - spider.ttf'.
 */
static constexpr const uint8_t kSpiderSymbol_ttf[] = {
    0x00, 0x01, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x80, 0x00, 0x03, 0x00, 0x40,
    0x47, 0x44, 0x45, 0x46, 0x00, 0x14, 0x00, 0x14, 0x00, 0x00, 0x07, 0xa8,
    0x00, 0x00, 0x00, 0x18, 0x4f, 0x53, 0x2f, 0x32, 0x8a, 0xf4, 0xfb, 0xdb,
    0x00, 0x00, 0x01, 0x48, 0x00, 0x00, 0x00, 0x60, 0x63, 0x6d, 0x61, 0x70,
    0xe0, 0x7f, 0x10, 0x7e, 0x00, 0x00, 0x01, 0xb8, 0x00, 0x00, 0x00, 0x54,
    0x67, 0x61, 0x73, 0x70, 0xff, 0xff, 0x00, 0x03, 0x00, 0x00, 0x07, 0xa0,
    0x00, 0x00, 0x00, 0x08, 0x67, 0x6c, 0x79, 0x66, 0x97, 0x0b, 0x6a, 0xf6,
    0x00, 0x00, 0x02, 0x18, 0x00, 0x00, 0x03, 0x40, 0x68, 0x65, 0x61, 0x64,
    0x0f, 0xa2, 0x24, 0x1a, 0x00, 0x00, 0x00, 0xcc, 0x00, 0x00, 0x00, 0x36,
    0x68, 0x68, 0x65, 0x61, 0x0e, 0xd3, 0x07, 0x3f, 0x00, 0x00, 0x01, 0x04,
    0x00, 0x00, 0x00, 0x24, 0x68, 0x6d, 0x74, 0x78, 0x10, 0x03, 0x00, 0x44,
    0x00, 0x00, 0x01, 0xa8, 0x00, 0x00, 0x00, 0x0e, 0x6c, 0x6f, 0x63, 0x61,
    0x01, 0xb4, 0x00, 0x28, 0x00, 0x00, 0x02, 0x0c, 0x00, 0x00, 0x00, 0x0a,
    0x6d, 0x61, 0x78, 0x70, 0x00, 0x4a, 0x01, 0x4d, 0x00, 0x00, 0x01, 0x28,
    0x00, 0x00, 0x00, 0x20, 0x6e, 0x61, 0x6d, 0x65, 0xc3, 0xe5, 0x39, 0xd4,
    0x00, 0x00, 0x05, 0x58, 0x00, 0x00, 0x02, 0x28, 0x70, 0x6f, 0x73, 0x74,
    0xff, 0x03, 0x00, 0x67, 0x00, 0x00, 0x07, 0x80, 0x00, 0x00, 0x00, 0x20,
    0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x0b, 0x0f, 0x08, 0x1d,
    0x5f, 0x0f, 0x3c, 0xf5, 0x00, 0x0b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xd1, 0x97, 0xa8, 0x5a, 0x00, 0x00, 0x00, 0x00, 0xd6, 0xe8, 0x32, 0x33,
    0x00, 0x03, 0xff, 0x3b, 0x08, 0x00, 0x05, 0x55, 0x00, 0x00, 0x00, 0x08,
    0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
    0x05, 0x55, 0xff, 0x3b, 0x01, 0x79, 0x08, 0x00, 0x00, 0x03, 0x00, 0x00,
    0x08, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x01, 0x00, 0x00,
    0x00, 0x04, 0x01, 0x1c, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
    0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x40, 0x00, 0x2e,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x08, 0x00, 0x01, 0x90, 0x00, 0x05,
    0x00, 0x00, 0x05, 0x33, 0x05, 0x99, 0x00, 0x00, 0x01, 0x1e, 0x05, 0x33,
    0x05, 0x99, 0x00, 0x00, 0x03, 0xd7, 0x00, 0x66, 0x02, 0x12, 0x00, 0x00,
    0x05, 0x00, 0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x73, 0x6b, 0x69, 0x61, 0x00, 0xc0, 0x00, 0x00, 0xf0, 0x21,
    0x06, 0x66, 0xfe, 0x66, 0x01, 0x79, 0x05, 0x55, 0x00, 0xc5, 0x80, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x20, 0x00, 0x01, 0x08, 0x00, 0x00, 0x44, 0x00, 0x00, 0x00, 0x00,
    0x08, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
    0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x04, 0x00, 0x48,
    0x00, 0x00, 0x00, 0x0e, 0x00, 0x08, 0x00, 0x02, 0x00, 0x06, 0x00, 0x00,
    0x00, 0x09, 0x00, 0x0d, 0x00, 0x1d, 0x00, 0x21, 0xf0, 0x21, 0xff, 0xff,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x0d, 0x00, 0x1d, 0x00, 0x21,
    0xf0, 0x21, 0xff, 0xff, 0x00, 0x01, 0xff, 0xf9, 0xff, 0xf5, 0xff, 0xe4,
    0xff, 0xe2, 0x0f, 0xe2, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14,
    0x00, 0x14, 0x00, 0x14, 0x01, 0xa0, 0x00, 0x00, 0x00, 0x02, 0x00, 0x44,
    0x00, 0x00, 0x02, 0x64, 0x05, 0x55, 0x00, 0x03, 0x00, 0x07, 0x00, 0x00,
    0x33, 0x11, 0x21, 0x11, 0x25, 0x21, 0x11, 0x21, 0x44, 0x02, 0x20, 0xfe,
    0x24, 0x01, 0x98, 0xfe, 0x68, 0x05, 0x55, 0xfa, 0xab, 0x44, 0x04, 0xcd,
    0x00, 0x04, 0x00, 0x03, 0xff, 0x3b, 0x08, 0x00, 0x05, 0x4c, 0x00, 0x15,
    0x00, 0x1d, 0x00, 0x25, 0x01, 0x1b, 0x00, 0x00, 0x01, 0x36, 0x37, 0x36,
    0x27, 0x26, 0x07, 0x06, 0x06, 0x23, 0x22, 0x27, 0x26, 0x27, 0x26, 0x07,
    0x06, 0x17, 0x16, 0x17, 0x16, 0x32, 0x37, 0x32, 0x35, 0x34, 0x23, 0x22,
    0x15, 0x14, 0x27, 0x32, 0x35, 0x34, 0x23, 0x22, 0x15, 0x14, 0x03, 0x32,
    0x17, 0x30, 0x17, 0x31, 0x36, 0x37, 0x36, 0x37, 0x36, 0x37, 0x36, 0x33,
    0x32, 0x33, 0x16, 0x33, 0x32, 0x17, 0x16, 0x07, 0x06, 0x23, 0x22, 0x27,
    0x26, 0x27, 0x26, 0x23, 0x22, 0x07, 0x07, 0x06, 0x07, 0x06, 0x07, 0x06,
    0x1f, 0x02, 0x37, 0x36, 0x37, 0x36, 0x33, 0x32, 0x17, 0x17, 0x16, 0x33,
    0x16, 0x17, 0x16, 0x07, 0x06, 0x23, 0x22, 0x27, 0x27, 0x26, 0x23, 0x22,
    0x07, 0x06, 0x07, 0x06, 0x17, 0x16, 0x17, 0x16, 0x33, 0x32, 0x33, 0x32,
    0x37, 0x36, 0x37, 0x36, 0x17, 0x16, 0x1f, 0x02, 0x16, 0x17, 0x16, 0x15,
    0x14, 0x23, 0x22, 0x27, 0x27, 0x26, 0x27, 0x27, 0x26, 0x27, 0x26, 0x07,
    0x06, 0x07, 0x06, 0x17, 0x16, 0x17, 0x16, 0x15, 0x14, 0x07, 0x06, 0x07,
    0x06, 0x23, 0x22, 0x27, 0x26, 0x07, 0x06, 0x07, 0x06, 0x15, 0x14, 0x17,
    0x16, 0x17, 0x16, 0x15, 0x14, 0x07, 0x06, 0x23, 0x22, 0x27, 0x26, 0x27,
    0x26, 0x35, 0x34, 0x37, 0x36, 0x37, 0x36, 0x37, 0x34, 0x27, 0x26, 0x07,
    0x06, 0x07, 0x06, 0x0f, 0x02, 0x06, 0x23, 0x22, 0x27, 0x26, 0x35, 0x34,
    0x37, 0x37, 0x36, 0x37, 0x36, 0x37, 0x36, 0x37, 0x36, 0x27, 0x26, 0x27,
    0x26, 0x07, 0x06, 0x07, 0x06, 0x07, 0x06, 0x07, 0x07, 0x06, 0x23, 0x22,
    0x27, 0x26, 0x35, 0x34, 0x37, 0x36, 0x37, 0x37, 0x36, 0x37, 0x37, 0x36,
    0x37, 0x36, 0x37, 0x36, 0x35, 0x34, 0x27, 0x26, 0x27, 0x26, 0x27, 0x26,
    0x23, 0x22, 0x07, 0x06, 0x07, 0x06, 0x07, 0x06, 0x27, 0x26, 0x27, 0x26,
    0x27, 0x26, 0x35, 0x34, 0x37, 0x36, 0x37, 0x36, 0x37, 0x36, 0x33, 0x32,
    0x17, 0x16, 0x33, 0x32, 0x37, 0x36, 0x35, 0x34, 0x37, 0x36, 0x37, 0x36,
    0x33, 0x04, 0xf5, 0x23, 0x13, 0x11, 0x14, 0x16, 0x1d, 0x1b, 0x4c, 0x1f,
    0x0e, 0x2d, 0x23, 0x14, 0x2c, 0x13, 0x18, 0x25, 0x2c, 0x10, 0x3c, 0x71,
    0x1d, 0x5c, 0x5c, 0x3f, 0xae, 0x5c, 0x5c, 0x3f, 0x6a, 0x27, 0x31, 0x5b,
    0x09, 0x27, 0x36, 0x03, 0x0a, 0x26, 0x35, 0x2e, 0x09, 0x08, 0xc6, 0x13,
    0x81, 0x17, 0x20, 0x18, 0x21, 0x1e, 0x04, 0x04, 0x15, 0x5c, 0x22, 0x26,
    0x48, 0x56, 0x3b, 0x10, 0x21, 0x01, 0x0c, 0x06, 0x06, 0x0f, 0x31, 0x44,
    0x3c, 0x52, 0x4a, 0x1d, 0x11, 0x3f, 0xb4, 0x71, 0x01, 0x26, 0x06, 0x0d,
    0x15, 0x1a, 0x2a, 0x13, 0x53, 0xaa, 0x42, 0x1d, 0x0a, 0x33, 0x20, 0x21,
    0x2b, 0x01, 0x02, 0x3e, 0x21, 0x09, 0x02, 0x02, 0x0f, 0x2d, 0x4b, 0x0a,
    0x22, 0x15, 0x20, 0x1f, 0x72, 0x8b, 0x2d, 0x2f, 0x1d, 0x1f, 0x0e, 0x25,
    0x3f, 0x4d, 0x1b, 0x63, 0x2a, 0x2c, 0x14, 0x22, 0x18, 0x1c, 0x0f, 0x08,
    0x2a, 0x08, 0x08, 0x0d, 0x3b, 0x4c, 0x52, 0x74, 0x27, 0x71, 0x2e, 0x01,
    0x0c, 0x10, 0x15, 0x0d, 0x06, 0x0d, 0x05, 0x01, 0x06, 0x2c, 0x28, 0x14,
    0x1b, 0x05, 0x04, 0x10, 0x06, 0x12, 0x08, 0x0a, 0x16, 0x27, 0x03, 0x0d,
    0x30, 0x4c, 0x4c, 0x4b, 0x1f, 0x0b, 0x22, 0x26, 0x0d, 0x15, 0x0d, 0x2d,
    0x68, 0x34, 0x14, 0x3c, 0x25, 0x12, 0x04, 0x10, 0x18, 0x0b, 0x09, 0x30,
    0x2b, 0x44, 0x66, 0x14, 0x47, 0x47, 0x59, 0x73, 0x25, 0x05, 0x03, 0x1f,
    0x01, 0x08, 0x3f, 0x48, 0x4b, 0x4b, 0x76, 0x2f, 0x49, 0x2d, 0x22, 0x24,
    0x0c, 0x15, 0x08, 0x0e, 0x33, 0x03, 0x44, 0x4c, 0x10, 0x46, 0x13, 0x1f,
    0x27, 0x1b, 0x1d, 0x13, 0x02, 0x24, 0x08, 0x02, 0x42, 0x0e, 0x4d, 0x3c,
    0x19, 0x1b, 0x40, 0x2b, 0x2b, 0x1e, 0x16, 0x11, 0x04, 0x1f, 0x11, 0x04,
    0x18, 0x11, 0x35, 0x01, 0xa3, 0x13, 0x24, 0x1f, 0x0b, 0x0c, 0x19, 0x19,
    0x18, 0x13, 0x0f, 0x0c, 0x1a, 0x18, 0x1f, 0x19, 0x1e, 0x07, 0x1a, 0xc3,
    0x54, 0x51, 0x54, 0x51, 0x04, 0x53, 0x51, 0x54, 0x50, 0x02, 0x48, 0x1a,
    0x31, 0x18, 0x55, 0x74, 0x04, 0x0e, 0x09, 0x0d, 0x06, 0x10, 0x16, 0x1b,
    0x24, 0x01, 0x04, 0x0b, 0x04, 0x10, 0x3f, 0x0a, 0x41, 0x02, 0x41, 0x20,
    0x06, 0x12, 0x16, 0x21, 0x17, 0x2a, 0x1e, 0x15, 0x40, 0x27, 0x11, 0x0e,
    0x1e, 0x11, 0x15, 0x1f, 0x43, 0x13, 0x1a, 0x10, 0x15, 0x1b, 0x04, 0x09,
    0x4d, 0x2a, 0x0f, 0x19, 0x0a, 0x0a, 0x03, 0x05, 0x15, 0x3c, 0x64, 0x21,
    0x4b, 0x2e, 0x21, 0x28, 0x13, 0x47, 0x44, 0x19, 0x3f, 0x11, 0x18, 0x0b,
    0x0a, 0x07, 0x18, 0x0d, 0x07, 0x24, 0x2c, 0x2b, 0x21, 0x32, 0x10, 0x48,
    0x2a, 0x2d, 0x1e, 0x1a, 0x01, 0x0c, 0x43, 0x59, 0x28, 0x4e, 0x1c, 0x0d,
    0x5d, 0x24, 0x14, 0x0a, 0x05, 0x1f, 0x24, 0x32, 0x46, 0x3e, 0x5f, 0x3e,
    0x44, 0x1a, 0x30, 0x15, 0x0d, 0x07, 0x18, 0x2b, 0x03, 0x0d, 0x1a, 0x28,
    0x28, 0x57, 0xb2, 0x29, 0x27, 0x40, 0x2c, 0x23, 0x16, 0x63, 0x58, 0x1a,
    0x0a, 0x18, 0x11, 0x23, 0x08, 0x1b, 0x29, 0x05, 0x04, 0x0b, 0x15, 0x0d,
    0x14, 0x0b, 0x2a, 0x29, 0x5a, 0x62, 0x01, 0x19, 0x1e, 0x05, 0x05, 0x26,
    0x42, 0x42, 0x2a, 0x2a, 0x3f, 0x0d, 0x0f, 0x09, 0x05, 0x07, 0x01, 0x0b,
    0x25, 0x3e, 0x0d, 0x17, 0x11, 0x01, 0x03, 0x0d, 0x13, 0x20, 0x19, 0x11,
    0x03, 0x02, 0x01, 0x04, 0x11, 0x04, 0x05, 0x1b, 0x3d, 0x10, 0x29, 0x20,
    0x04, 0x04, 0x0a, 0x07, 0x04, 0x1f, 0x15, 0x20, 0x3e, 0x0f, 0x2a, 0x1e,
    0x00, 0x00, 0x00, 0x1b, 0x01, 0x4a, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x1b, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x00, 0x0c, 0x00, 0x1b, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x02, 0x00, 0x07, 0x00, 0x27, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x03, 0x00, 0x0c, 0x00, 0x1b, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x04, 0x00, 0x0c, 0x00, 0x1b, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x05, 0x00, 0x02, 0x00, 0x2e, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x06, 0x00, 0x0c, 0x00, 0x1b, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x0d, 0x00, 0x1b, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x0e, 0x00, 0x1a, 0x00, 0x30, 0x00, 0x03, 0x00, 0x00, 0x04, 0x09,
    0x00, 0x00, 0x00, 0x36, 0x00, 0x4a, 0x00, 0x03, 0x00, 0x00, 0x04, 0x09,
    0x00, 0x01, 0x00, 0x18, 0x00, 0x80, 0x00, 0x03, 0x00, 0x00, 0x04, 0x09,
    0x00, 0x02, 0x00, 0x0e, 0x00, 0x98, 0x00, 0x03, 0x00, 0x00, 0x04, 0x09,
    0x00, 0x03, 0x00, 0x18, 0x00, 0x80, 0x00, 0x03, 0x00, 0x00, 0x04, 0x09,
    0x00, 0x04, 0x00, 0x18, 0x00, 0x80, 0x00, 0x03, 0x00, 0x00, 0x04, 0x09,
    0x00, 0x05, 0x00, 0x04, 0x00, 0xa6, 0x00, 0x03, 0x00, 0x00, 0x04, 0x09,
    0x00, 0x06, 0x00, 0x18, 0x00, 0x80, 0x00, 0x03, 0x00, 0x00, 0x04, 0x09,
    0x00, 0x0d, 0x00, 0x36, 0x00, 0x4a, 0x00, 0x03, 0x00, 0x00, 0x04, 0x09,
    0x00, 0x0e, 0x00, 0x34, 0x00, 0xaa, 0x00, 0x03, 0x00, 0x01, 0x04, 0x09,
    0x00, 0x00, 0x00, 0x36, 0x00, 0x4a, 0x00, 0x03, 0x00, 0x01, 0x04, 0x09,
    0x00, 0x01, 0x00, 0x18, 0x00, 0x80, 0x00, 0x03, 0x00, 0x01, 0x04, 0x09,
    0x00, 0x02, 0x00, 0x0e, 0x00, 0x98, 0x00, 0x03, 0x00, 0x01, 0x04, 0x09,
    0x00, 0x03, 0x00, 0x18, 0x00, 0x80, 0x00, 0x03, 0x00, 0x01, 0x04, 0x09,
    0x00, 0x04, 0x00, 0x18, 0x00, 0x80, 0x00, 0x03, 0x00, 0x01, 0x04, 0x09,
    0x00, 0x05, 0x00, 0x04, 0x00, 0xa6, 0x00, 0x03, 0x00, 0x01, 0x04, 0x09,
    0x00, 0x06, 0x00, 0x18, 0x00, 0x80, 0x00, 0x03, 0x00, 0x01, 0x04, 0x09,
    0x00, 0x0d, 0x00, 0x36, 0x00, 0x4a, 0x00, 0x03, 0x00, 0x01, 0x04, 0x09,
    0x00, 0x0e, 0x00, 0x34, 0x00, 0xaa, 0x43, 0x6f, 0x70, 0x79, 0x72, 0x69,
    0x67, 0x68, 0x74, 0x20, 0x28, 0x63, 0x29, 0x20, 0x32, 0x30, 0x31, 0x35,
    0x2c, 0x20, 0x47, 0x6f, 0x6f, 0x67, 0x6c, 0x65, 0x2e, 0x53, 0x70, 0x69,
    0x64, 0x65, 0x72, 0x53, 0x79, 0x6d, 0x62, 0x6f, 0x6c, 0x52, 0x65, 0x67,
    0x75, 0x6c, 0x61, 0x72, 0x56, 0x31, 0x68, 0x74, 0x74, 0x70, 0x3a, 0x2f,
    0x2f, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x73, 0x2e, 0x73, 0x69, 0x6c,
    0x2e, 0x6f, 0x72, 0x67, 0x2f, 0x4f, 0x46, 0x4c, 0x00, 0x43, 0x00, 0x6f,
    0x00, 0x70, 0x00, 0x79, 0x00, 0x72, 0x00, 0x69, 0x00, 0x67, 0x00, 0x68,
    0x00, 0x74, 0x00, 0x20, 0x00, 0x28, 0x00, 0x63, 0x00, 0x29, 0x00, 0x20,
    0x00, 0x32, 0x00, 0x30, 0x00, 0x31, 0x00, 0x35, 0x00, 0x2c, 0x00, 0x20,
    0x00, 0x47, 0x00, 0x6f, 0x00, 0x6f, 0x00, 0x67, 0x00, 0x6c, 0x00, 0x65,
    0x00, 0x2e, 0x00, 0x53, 0x00, 0x70, 0x00, 0x69, 0x00, 0x64, 0x00, 0x65,
    0x00, 0x72, 0x00, 0x53, 0x00, 0x79, 0x00, 0x6d, 0x00, 0x62, 0x00, 0x6f,
    0x00, 0x6c, 0x00, 0x52, 0x00, 0x65, 0x00, 0x67, 0x00, 0x75, 0x00, 0x6c,
    0x00, 0x61, 0x00, 0x72, 0x00, 0x56, 0x00, 0x31, 0x00, 0x68, 0x00, 0x74,
    0x00, 0x74, 0x00, 0x70, 0x00, 0x3a, 0x00, 0x2f, 0x00, 0x2f, 0x00, 0x73,
    0x00, 0x63, 0x00, 0x72, 0x00, 0x69, 0x00, 0x70, 0x00, 0x74, 0x00, 0x73,
    0x00, 0x2e, 0x00, 0x73, 0x00, 0x69, 0x00, 0x6c, 0x00, 0x2e, 0x00, 0x6f,
    0x00, 0x72, 0x00, 0x67, 0x00, 0x2f, 0x00, 0x4f, 0x00, 0x46, 0x00, 0x4c,
    0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x66,
    0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
    0xff, 0xff, 0x00, 0x02, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x0c, 0x00, 0x14, 0x00, 0x04, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
    0x00, 0x02, 0x00, 0x00
};

enum class SmoothBehavior {
    none, // SmoothFonts produces no effect.
    some, // SmoothFonts produces some effect, but not subpixel coverage.
    subpixel, // SmoothFonts produces some effect and provides subpixel coverage.
};

/**
 * There does not appear to be a publicly accessable API for determining if lcd
 * font smoothing will be applied if we request it. The main issue is that if
 * smoothing is applied a gamma of 2.0 will be used, if not a gamma of 1.0.
 */
static SmoothBehavior smooth_behavior() {
    static SmoothBehavior gSmoothBehavior = []{
        uint32_t noSmoothBitmap[16][16] = {};
        uint32_t smoothBitmap[16][16] = {};

        SkUniqueCFRef<CGColorSpaceRef> colorspace(CGColorSpaceCreateDeviceRGB());
        SkUniqueCFRef<CGContextRef> noSmoothContext(
                CGBitmapContextCreate(&noSmoothBitmap, 16, 16, 8, 16*4,
                                      colorspace.get(), BITMAP_INFO_RGB));
        SkUniqueCFRef<CGContextRef> smoothContext(
                CGBitmapContextCreate(&smoothBitmap, 16, 16, 8, 16*4,
                                      colorspace.get(), BITMAP_INFO_RGB));

        SkUniqueCFRef<CGDataProviderRef> data(
                CGDataProviderCreateWithData(nullptr, kSpiderSymbol_ttf,
                                             SK_ARRAY_COUNT(kSpiderSymbol_ttf), nullptr));
        SkUniqueCFRef<CGFontRef> cgFont(CGFontCreateWithDataProvider(data.get()));
        SkASSERT(cgFont);
        SkUniqueCFRef<CTFontRef> ctFont(
                CTFontCreateWithGraphicsFont(cgFont.get(), 16, nullptr, nullptr));
        SkASSERT(ctFont);

        CGContextSetShouldSmoothFonts(noSmoothContext.get(), false);
        CGContextSetShouldAntialias(noSmoothContext.get(), true);
        CGContextSetTextDrawingMode(noSmoothContext.get(), kCGTextFill);
        CGContextSetGrayFillColor(noSmoothContext.get(), 1, 1);

        CGContextSetShouldSmoothFonts(smoothContext.get(), true);
        CGContextSetShouldAntialias(smoothContext.get(), true);
        CGContextSetTextDrawingMode(smoothContext.get(), kCGTextFill);
        CGContextSetGrayFillColor(smoothContext.get(), 1, 1);

        CGPoint point = CGPointMake(0, 3);
        CGGlyph spiderGlyph = 3;
        CTFontDrawGlyphs(ctFont.get(), &spiderGlyph, &point, 1, noSmoothContext.get());
        CTFontDrawGlyphs(ctFont.get(), &spiderGlyph, &point, 1, smoothContext.get());

        // For debugging.
        //SkUniqueCFRef<CGImageRef> image(CGBitmapContextCreateImage(noSmoothContext()));
        //SkUniqueCFRef<CGImageRef> image(CGBitmapContextCreateImage(smoothContext()));

        SmoothBehavior smoothBehavior = SmoothBehavior::none;
        for (int x = 0; x < 16; ++x) {
            for (int y = 0; y < 16; ++y) {
                uint32_t smoothPixel = smoothBitmap[x][y];
                uint32_t r = (smoothPixel >> 16) & 0xFF;
                uint32_t g = (smoothPixel >>  8) & 0xFF;
                uint32_t b = (smoothPixel >>  0) & 0xFF;
                if (r != g || r != b) {
                    return SmoothBehavior::subpixel;
                }
                if (noSmoothBitmap[x][y] != smoothPixel) {
                    smoothBehavior = SmoothBehavior::some;
                }
            }
        }
        return smoothBehavior;
    }();
    return gSmoothBehavior;
}

class Offscreen {
public:
    Offscreen()
        : fRGBSpace(nullptr)
        , fCG(nullptr)
        , fDoAA(false)
        , fDoLCD(false)
    {
        fSize.set(0, 0);
    }

    CGRGBPixel* getCG(const SkScalerContext_Mac& context, const SkGlyph& glyph,
                      CGGlyph glyphID, size_t* rowBytesPtr, bool generateA8FromLCD);

private:
    enum {
        kSize = 32 * 32 * sizeof(CGRGBPixel)
    };
    SkAutoSMalloc<kSize> fImageStorage;
    SkUniqueCFRef<CGColorSpaceRef> fRGBSpace;

    // cached state
    SkUniqueCFRef<CGContextRef> fCG;
    SkISize fSize;
    bool fDoAA;
    bool fDoLCD;

    static int RoundSize(int dimension) {
        return SkNextPow2(dimension);
    }
};

///////////////////////////////////////////////////////////////////////////////

static bool find_dict_CGFloat(CFDictionaryRef dict, CFStringRef name, CGFloat* value) {
    CFNumberRef num;
    return CFDictionaryGetValueIfPresent(dict, name, (const void**)&num)
        && CFNumberIsFloatType(num)
        && CFNumberGetValue(num, kCFNumberCGFloatType, value);
}

template <typename S, typename D, typename C> struct LinearInterpolater {
    struct Mapping {
        S src_val;
        D dst_val;
    };
    constexpr LinearInterpolater(Mapping const mapping[], int mappingCount)
        : fMapping(mapping), fMappingCount(mappingCount) {}

    static D map(S value, S src_min, S src_max, D dst_min, D dst_max) {
        SkASSERT(src_min < src_max);
        SkASSERT(dst_min <= dst_max);
        return C()(dst_min + (((value - src_min) * (dst_max - dst_min)) / (src_max - src_min)));
    }

    D map(S val) const {
        // -Inf to [0]
        if (val < fMapping[0].src_val) {
            return fMapping[0].dst_val;
        }

        // Linear from [i] to [i+1]
        for (int i = 0; i < fMappingCount - 1; ++i) {
            if (val < fMapping[i+1].src_val) {
                return map(val, fMapping[i].src_val, fMapping[i+1].src_val,
                                fMapping[i].dst_val, fMapping[i+1].dst_val);
            }
        }

        // From [n] to +Inf
        // if (fcweight < Inf)
        return fMapping[fMappingCount - 1].dst_val;
    }

    Mapping const * fMapping;
    int fMappingCount;
};

struct RoundCGFloatToInt {
    int operator()(CGFloat s) { return s + 0.5; }
};
struct CGFloatIdentity {
    CGFloat operator()(CGFloat s) { return s; }
};

/** Returns the [-1, 1] CTFontDescriptor weights for the
 *  <0, 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000> CSS weights.
 *
 *  It is assumed that the values will be interpolated linearly between these points.
 *  NSFontWeightXXX were added in 10.11, appear in 10.10, but do not appear in 10.9.
 *  The actual values appear to be stable, but they may change in the future without notice.
 */
static CGFloat(&get_NSFontWeight_mapping())[11] {

    // Declarations in <AppKit/AppKit.h> on macOS, <UIKit/UIKit.h> on iOS
#ifdef SK_BUILD_FOR_MAC
#  define SK_KIT_FONT_WEIGHT_PREFIX "NS"
#endif
#ifdef SK_BUILD_FOR_IOS
#  define SK_KIT_FONT_WEIGHT_PREFIX "UI"
#endif
    static constexpr struct {
        CGFloat defaultValue;
        const char* name;
    } nsFontWeightLoaderInfos[] = {
        { -0.80f, SK_KIT_FONT_WEIGHT_PREFIX "FontWeightUltraLight" },
        { -0.60f, SK_KIT_FONT_WEIGHT_PREFIX "FontWeightThin" },
        { -0.40f, SK_KIT_FONT_WEIGHT_PREFIX "FontWeightLight" },
        {  0.00f, SK_KIT_FONT_WEIGHT_PREFIX "FontWeightRegular" },
        {  0.23f, SK_KIT_FONT_WEIGHT_PREFIX "FontWeightMedium" },
        {  0.30f, SK_KIT_FONT_WEIGHT_PREFIX "FontWeightSemibold" },
        {  0.40f, SK_KIT_FONT_WEIGHT_PREFIX "FontWeightBold" },
        {  0.56f, SK_KIT_FONT_WEIGHT_PREFIX "FontWeightHeavy" },
        {  0.62f, SK_KIT_FONT_WEIGHT_PREFIX "FontWeightBlack" },
    };

    static_assert(SK_ARRAY_COUNT(nsFontWeightLoaderInfos) == 9, "");
    static CGFloat nsFontWeights[11];
    static SkOnce once;
    once([&] {
        size_t i = 0;
        nsFontWeights[i++] = -1.00;
        for (const auto& nsFontWeightLoaderInfo : nsFontWeightLoaderInfos) {
            void* nsFontWeightValuePtr = dlsym(RTLD_DEFAULT, nsFontWeightLoaderInfo.name);
            if (nsFontWeightValuePtr) {
                nsFontWeights[i++] = *(static_cast<CGFloat*>(nsFontWeightValuePtr));
            } else {
                nsFontWeights[i++] = nsFontWeightLoaderInfo.defaultValue;
            }
        }
        nsFontWeights[i++] = 1.00;
    });
    return nsFontWeights;
}

/** Convert the [0, 1000] CSS weight to [-1, 1] CTFontDescriptor weight (for system fonts).
 *
 *  The -1 to 1 weights reported by CTFontDescriptors have different mappings depending on if the
 *  CTFont is native or created from a CGDataProvider.
 */
static CGFloat fontstyle_to_ct_weight(int fontstyleWeight) {
    using Interpolator = LinearInterpolater<int, CGFloat, CGFloatIdentity>;

    // Note that Mac supports the old OS2 version A so 0 through 10 are as if multiplied by 100.
    // However, on this end we can't tell, so this is ignored.

    static Interpolator::Mapping nativeWeightMappings[11];
    static SkOnce once;
    once([&] {
        CGFloat(&nsFontWeights)[11] = get_NSFontWeight_mapping();
        for (int i = 0; i < 11; ++i) {
            nativeWeightMappings[i].src_val = i * 100;
            nativeWeightMappings[i].dst_val = nsFontWeights[i];
        }
    });
    static constexpr Interpolator nativeInterpolator(
            nativeWeightMappings, SK_ARRAY_COUNT(nativeWeightMappings));

    return nativeInterpolator.map(fontstyleWeight);
}


/** Convert the [-1, 1] CTFontDescriptor weight to [0, 1000] CSS weight.
 *
 *  The -1 to 1 weights reported by CTFontDescriptors have different mappings depending on if the
 *  CTFont is native or created from a CGDataProvider.
 */
static int ct_weight_to_fontstyle(CGFloat cgWeight, bool fromDataProvider) {
    using Interpolator = LinearInterpolater<CGFloat, int, RoundCGFloatToInt>;

    // Note that Mac supports the old OS2 version A so 0 through 10 are as if multiplied by 100.
    // However, on this end we can't tell, so this is ignored.

    /** This mapping for CGDataProvider created fonts is determined by creating font data with every
     *  weight, creating a CTFont, and asking the CTFont for its weight. See the TypefaceStyle test
     *  in tests/TypefaceTest.cpp for the code used to determine these values.
     */
    static constexpr Interpolator::Mapping dataProviderWeightMappings[] = {
        { -1.00,    0 },
        { -0.70,  100 },
        { -0.50,  200 },
        { -0.23,  300 },
        {  0.00,  400 },
        {  0.20,  500 },
        {  0.30,  600 },
        {  0.40,  700 },
        {  0.60,  800 },
        {  0.80,  900 },
        {  1.00, 1000 },
    };
    static constexpr Interpolator dataProviderInterpolator(
            dataProviderWeightMappings, SK_ARRAY_COUNT(dataProviderWeightMappings));

    static Interpolator::Mapping nativeWeightMappings[11];
    static SkOnce once;
    once([&] {
        CGFloat(&nsFontWeights)[11] = get_NSFontWeight_mapping();
        for (int i = 0; i < 11; ++i) {
            nativeWeightMappings[i].src_val = nsFontWeights[i];
            nativeWeightMappings[i].dst_val = i * 100;
        }
    });
    static constexpr Interpolator nativeInterpolator(
            nativeWeightMappings, SK_ARRAY_COUNT(nativeWeightMappings));

    return fromDataProvider ? dataProviderInterpolator.map(cgWeight)
                            : nativeInterpolator.map(cgWeight);
}

/** Convert the [0, 10] CSS weight to [-1, 1] CTFontDescriptor width. */
static int fontstyle_to_ct_width(int fontstyleWidth) {
    using Interpolator = LinearInterpolater<int, CGFloat, CGFloatIdentity>;

    // Values determined by creating font data with every width, creating a CTFont,
    // and asking the CTFont for its width. See TypefaceStyle test for basics.
    static constexpr Interpolator::Mapping widthMappings[] = {
        {  0, -0.5 },
        { 10,  0.5 },
    };
    static constexpr Interpolator interpolator(widthMappings, SK_ARRAY_COUNT(widthMappings));
    return interpolator.map(fontstyleWidth);
}

/** Convert the [-1, 1] CTFontDescriptor width to [0, 10] CSS weight. */
static int ct_width_to_fontstyle(CGFloat cgWidth) {
    using Interpolator = LinearInterpolater<CGFloat, int, RoundCGFloatToInt>;

    // Values determined by creating font data with every width, creating a CTFont,
    // and asking the CTFont for its width. See TypefaceStyle test for basics.
    static constexpr Interpolator::Mapping widthMappings[] = {
        { -0.5,  0 },
        {  0.5, 10 },
    };
    static constexpr Interpolator interpolator(widthMappings, SK_ARRAY_COUNT(widthMappings));
    return interpolator.map(cgWidth);
}

static SkFontStyle fontstyle_from_descriptor(CTFontDescriptorRef desc, bool fromDataProvider) {
    SkUniqueCFRef<CFTypeRef> traits(CTFontDescriptorCopyAttribute(desc, kCTFontTraitsAttribute));
    if (!traits || CFDictionaryGetTypeID() != CFGetTypeID(traits.get())) {
        return SkFontStyle();
    }
    SkUniqueCFRef<CFDictionaryRef> fontTraitsDict(static_cast<CFDictionaryRef>(traits.release()));

    CGFloat weight, width, slant;
    if (!find_dict_CGFloat(fontTraitsDict.get(), kCTFontWeightTrait, &weight)) {
        weight = 0;
    }
    if (!find_dict_CGFloat(fontTraitsDict.get(), kCTFontWidthTrait, &width)) {
        width = 0;
    }
    if (!find_dict_CGFloat(fontTraitsDict.get(), kCTFontSlantTrait, &slant)) {
        slant = 0;
    }

    return SkFontStyle(ct_weight_to_fontstyle(weight, fromDataProvider),
                       ct_width_to_fontstyle(width),
                       slant ? SkFontStyle::kItalic_Slant
                             : SkFontStyle::kUpright_Slant);
}

class SkTypeface_Mac : public SkTypeface {
public:
    SkTypeface_Mac(SkUniqueCFRef<CTFontRef> fontRef, SkUniqueCFRef<CFTypeRef> resourceRef,
                   const SkFontStyle& fs, bool isFixedPitch,
                   std::unique_ptr<SkStreamAsset> providedData)
        : SkTypeface(fs, isFixedPitch)
        , fFontRef(std::move(fontRef))
        , fOriginatingCFTypeRef(std::move(resourceRef))
        , fHasColorGlyphs(
                SkToBool(CTFontGetSymbolicTraits(fFontRef.get()) & kCTFontColorGlyphsTrait))
        , fStream(std::move(providedData))
        , fIsFromStream(fStream)
    {
        SkASSERT(fFontRef);
    }

    SkUniqueCFRef<CTFontRef> fFontRef;
    SkUniqueCFRef<CFTypeRef> fOriginatingCFTypeRef;
    const bool fHasColorGlyphs;

protected:
    int onGetUPEM() const override;
    std::unique_ptr<SkStreamAsset> onOpenStream(int* ttcIndex) const override;
    std::unique_ptr<SkFontData> onMakeFontData() const override;
    int onGetVariationDesignPosition(SkFontArguments::VariationPosition::Coordinate coordinates[],
                                     int coordinateCount) const override;
    void onGetFamilyName(SkString* familyName) const override;
    SkTypeface::LocalizedStrings* onCreateFamilyNameIterator() const override;
    int onGetTableTags(SkFontTableTag tags[]) const override;
    size_t onGetTableData(SkFontTableTag, size_t offset, size_t length, void* data) const override;
    SkScalerContext* onCreateScalerContext(const SkScalerContextEffects&,
                                           const SkDescriptor*) const override;
    void onFilterRec(SkScalerContextRec*) const override;
    void onGetFontDescriptor(SkFontDescriptor*, bool*) const override;
    void getGlyphToUnicodeMap(SkUnichar*) const override;
    std::unique_ptr<SkAdvancedTypefaceMetrics> onGetAdvancedMetrics() const override;
    void onCharsToGlyphs(const SkUnichar* chars, int count, SkGlyphID glyphs[]) const override;
    int onCountGlyphs() const override;
    void getPostScriptGlyphNames(SkString*) const override {}
    int onGetVariationDesignParameters(SkFontParameters::Variation::Axis parameters[],
                                       int parameterCount) const override
    {
        return -1;
    }
    sk_sp<SkTypeface> onMakeClone(const SkFontArguments&) const override {
        return nullptr;
    }

    void* onGetCTFontRef() const override { return (void*)fFontRef.get(); }

private:
    mutable std::unique_ptr<SkStreamAsset> fStream;
    bool fIsFromStream;
    mutable SkOnce fInitStream;

    typedef SkTypeface INHERITED;
};

static bool find_by_CTFontRef(SkTypeface* cached, void* context) {
    CTFontRef self = (CTFontRef)context;
    CTFontRef other = (CTFontRef)cached->internal_private_getCTFontRef();

    return CFEqual(self, other);
}

/** Creates a typeface, searching the cache if isLocalStream is false. */
static sk_sp<SkTypeface> create_from_CTFontRef(SkUniqueCFRef<CTFontRef> font,
                                               SkUniqueCFRef<CFTypeRef> resource,
                                               std::unique_ptr<SkStreamAsset> providedData) {
    SkASSERT(font);
    const bool isFromStream(providedData);

    if (!isFromStream) {
        sk_sp<SkTypeface> face = SkTypefaceCache::FindByProcAndRef(find_by_CTFontRef,
                                                                   (void*)font.get());
        if (face) {
            return face;
        }
    }

    SkUniqueCFRef<CTFontDescriptorRef> desc(CTFontCopyFontDescriptor(font.get()));
    SkFontStyle style = fontstyle_from_descriptor(desc.get(), isFromStream);
    CTFontSymbolicTraits traits = CTFontGetSymbolicTraits(font.get());
    bool isFixedPitch = SkToBool(traits & kCTFontMonoSpaceTrait);

    sk_sp<SkTypeface> face(new SkTypeface_Mac(std::move(font), std::move(resource),
                                              style, isFixedPitch, std::move(providedData)));
    if (!isFromStream) {
        SkTypefaceCache::Add(face);
    }
    return face;
}

/** Creates a typeface from a descriptor, searching the cache. */
static sk_sp<SkTypeface> create_from_desc(CTFontDescriptorRef desc) {
    SkUniqueCFRef<CTFontRef> ctFont(CTFontCreateWithFontDescriptor(desc, 0, nullptr));
    if (!ctFont) {
        return nullptr;
    }

    return create_from_CTFontRef(std::move(ctFont), nullptr, nullptr);
}

static SkUniqueCFRef<CTFontDescriptorRef> create_descriptor(const char familyName[],
                                                            const SkFontStyle& style) {
    SkUniqueCFRef<CFMutableDictionaryRef> cfAttributes(
            CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
                                      &kCFTypeDictionaryKeyCallBacks,
                                      &kCFTypeDictionaryValueCallBacks));

    SkUniqueCFRef<CFMutableDictionaryRef> cfTraits(
            CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
                                      &kCFTypeDictionaryKeyCallBacks,
                                      &kCFTypeDictionaryValueCallBacks));

    if (!cfAttributes || !cfTraits) {
        return nullptr;
    }

    // CTFontTraits (symbolic)
    // macOS 14 and iOS 12 seem to behave badly when kCTFontSymbolicTrait is set.

    // CTFontTraits (weight)
    CGFloat ctWeight = fontstyle_to_ct_weight(style.weight());
    SkUniqueCFRef<CFNumberRef> cfFontWeight(
            CFNumberCreate(kCFAllocatorDefault, kCFNumberCGFloatType, &ctWeight));
    if (cfFontWeight) {
        CFDictionaryAddValue(cfTraits.get(), kCTFontWeightTrait, cfFontWeight.get());
    }
    // CTFontTraits (width)
    CGFloat ctWidth = fontstyle_to_ct_width(style.width());
    SkUniqueCFRef<CFNumberRef> cfFontWidth(
            CFNumberCreate(kCFAllocatorDefault, kCFNumberCGFloatType, &ctWidth));
    if (cfFontWidth) {
        CFDictionaryAddValue(cfTraits.get(), kCTFontWidthTrait, cfFontWidth.get());
    }
    // CTFontTraits (slant)
    CGFloat ctSlant = style.slant() == SkFontStyle::kUpright_Slant ? 0 : 1;
    SkUniqueCFRef<CFNumberRef> cfFontSlant(
            CFNumberCreate(kCFAllocatorDefault, kCFNumberCGFloatType, &ctSlant));
    if (cfFontSlant) {
        CFDictionaryAddValue(cfTraits.get(), kCTFontSlantTrait, cfFontSlant.get());
    }
    // CTFontTraits
    CFDictionaryAddValue(cfAttributes.get(), kCTFontTraitsAttribute, cfTraits.get());

    // CTFontFamilyName
    if (familyName) {
        SkUniqueCFRef<CFStringRef> cfFontName = make_CFString(familyName);
        if (cfFontName) {
            CFDictionaryAddValue(cfAttributes.get(), kCTFontFamilyNameAttribute, cfFontName.get());
        }
    }

    return SkUniqueCFRef<CTFontDescriptorRef>(
            CTFontDescriptorCreateWithAttributes(cfAttributes.get()));
}

/** Creates a typeface from a name, searching the cache. */
static sk_sp<SkTypeface> create_from_name(const char familyName[], const SkFontStyle& style) {
    SkUniqueCFRef<CTFontDescriptorRef> desc = create_descriptor(familyName, style);
    if (!desc) {
        return nullptr;
    }
    return create_from_desc(desc.get());
}

///////////////////////////////////////////////////////////////////////////////

/*  This function is visible on the outside. It first searches the cache, and if
 *  not found, returns a new entry (after adding it to the cache).
 */
SkTypeface* SkCreateTypefaceFromCTFont(CTFontRef font, CFTypeRef resource) {
    CFRetain(font);
    if (resource) {
        CFRetain(resource);
    }
    return create_from_CTFontRef(SkUniqueCFRef<CTFontRef>(font),
                                 SkUniqueCFRef<CFTypeRef>(resource),
                                 nullptr).release();
}

static const char* map_css_names(const char* name) {
    static const struct {
        const char* fFrom;  // name the caller specified
        const char* fTo;    // "canonical" name we map to
    } gPairs[] = {
        { "sans-serif", "Helvetica" },
        { "serif",      "Times"     },
        { "monospace",  "Courier"   }
    };

    for (size_t i = 0; i < SK_ARRAY_COUNT(gPairs); i++) {
        if (strcmp(name, gPairs[i].fFrom) == 0) {
            return gPairs[i].fTo;
        }
    }
    return name;    // no change
}

///////////////////////////////////////////////////////////////////////////////

class SkScalerContext_Mac : public SkScalerContext {
public:
    SkScalerContext_Mac(sk_sp<SkTypeface_Mac>, const SkScalerContextEffects&, const SkDescriptor*);

protected:
    unsigned generateGlyphCount(void) override;
    bool generateAdvance(SkGlyph* glyph) override;
    void generateMetrics(SkGlyph* glyph) override;
    void generateImage(const SkGlyph& glyph) override;
    bool generatePath(SkGlyphID glyph, SkPath* path) override;
    void generateFontMetrics(SkFontMetrics*) override;

private:
    static void CTPathElement(void *info, const CGPathElement *element);

    Offscreen fOffscreen;

    /** Unrotated variant of fCTFont.
     *
     *  In 10.10.1 CTFontGetAdvancesForGlyphs applies the font transform to the width of the
     *  advances, but always sets the height to 0. This font is used to get the advances of the
     *  unrotated glyph, and then the rotation is applied separately.
     *
     *  CT vertical metrics are pre-rotated (in em space, before transform) 90deg clock-wise.
     *  This makes kCTFontOrientationDefault dangerous, because the metrics from
     *  kCTFontOrientationHorizontal are in a different space from kCTFontOrientationVertical.
     *  With kCTFontOrientationVertical the advances must be unrotated.
     *
     *  Sometimes, creating a copy of a CTFont with the same size but different trasform will select
     *  different underlying font data. As a result, avoid ever creating more than one CTFont per
     *  SkScalerContext to ensure that only one CTFont is used.
     *
     *  As a result of the above (and other constraints) this font contains the size, but not the
     *  transform. The transform must always be applied separately.
     */
    SkUniqueCFRef<CTFontRef> fCTFont;

    /** The transform without the font size. */
    CGAffineTransform fTransform;
    CGAffineTransform fInvTransform;

    SkUniqueCFRef<CGFontRef> fCGFont;
    uint16_t fGlyphCount;
    const bool fDoSubPosition;

    friend class Offscreen;

    typedef SkScalerContext INHERITED;
};

// CTFontCreateCopyWithAttributes or CTFontCreateCopyWithSymbolicTraits cannot be used on 10.10
// and later, as they will return different underlying fonts depending on the size requested.
// It is not possible to use descriptors with CTFontCreateWithFontDescriptor, since that does not
// work with non-system fonts. As a result, create the strike specific CTFonts from the underlying
// CGFont.
static SkUniqueCFRef<CTFontRef> ctfont_create_exact_copy(CTFontRef baseFont, CGFloat textSize,
                                                         const CGAffineTransform* transform)
{
    SkUniqueCFRef<CGFontRef> baseCGFont(CTFontCopyGraphicsFont(baseFont, nullptr));

    // The last parameter (CTFontDescriptorRef attributes) *must* be nullptr.
    // If non-nullptr then with fonts with variation axes, the copy will fail in
    // CGFontVariationFromDictCallback when it assumes kCGFontVariationAxisName is CFNumberRef
    // which it quite obviously is not.

    // Because we cannot setup the CTFont descriptor to match, the same restriction applies here
    // as other uses of CTFontCreateWithGraphicsFont which is that such CTFonts should not escape
    // the scaler context, since they aren't 'normal'.
    return SkUniqueCFRef<CTFontRef>(
            CTFontCreateWithGraphicsFont(baseCGFont.get(), textSize, transform, nullptr));
}

SkScalerContext_Mac::SkScalerContext_Mac(sk_sp<SkTypeface_Mac> typeface,
                                         const SkScalerContextEffects& effects,
                                         const SkDescriptor* desc)
        : INHERITED(std::move(typeface), effects, desc)
        , fDoSubPosition(SkToBool(fRec.fFlags & kSubpixelPositioning_Flag))

{
    AUTO_CG_LOCK();

    CTFontRef ctFont = (CTFontRef)this->getTypeface()->internal_private_getCTFontRef();
    CFIndex numGlyphs = CTFontGetGlyphCount(ctFont);
    SkASSERT(numGlyphs >= 1 && numGlyphs <= 0xFFFF);
    fGlyphCount = SkToU16(numGlyphs);

    // CT on (at least) 10.9 will size color glyphs down from the requested size, but not up.
    // As a result, it is necessary to know the actual device size and request that.
    SkVector scale;
    SkMatrix skTransform;
    bool invertible = fRec.computeMatrices(SkScalerContextRec::kVertical_PreMatrixScale,
                                           &scale, &skTransform, nullptr, nullptr, nullptr);
    fTransform = MatrixToCGAffineTransform(skTransform);
    // CGAffineTransformInvert documents that if the transform is non-invertible it will return the
    // passed transform unchanged. It does so, but then also prints a message to stdout. Avoid this.
    if (invertible) {
        fInvTransform = CGAffineTransformInvert(fTransform);
    } else {
        fInvTransform = fTransform;
    }

    // The transform contains everything except the requested text size.
    // Some properties, like 'trak', are based on the text size (before applying the matrix).
    CGFloat textSize = ScalarToCG(scale.y());
    fCTFont = ctfont_create_exact_copy(ctFont, textSize, nullptr);
    fCGFont.reset(CTFontCopyGraphicsFont(fCTFont.get(), nullptr));
}

CGRGBPixel* Offscreen::getCG(const SkScalerContext_Mac& context, const SkGlyph& glyph,
                             CGGlyph glyphID, size_t* rowBytesPtr,
                             bool generateA8FromLCD) {
    if (!fRGBSpace) {
        //It doesn't appear to matter what color space is specified.
        //Regular blends and antialiased text are always (s*a + d*(1-a))
        //and subpixel antialiased text is always g=2.0.
        fRGBSpace.reset(CGColorSpaceCreateDeviceRGB());
    }

    // default to kBW_Format
    bool doAA = false;
    bool doLCD = false;

    if (SkMask::kBW_Format != glyph.fMaskFormat) {
        doLCD = true;
        doAA = true;
    }

    // FIXME: lcd smoothed un-hinted rasterization unsupported.
    if (!generateA8FromLCD && SkMask::kA8_Format == glyph.fMaskFormat) {
        doLCD = false;
        doAA = true;
    }

    // If this font might have color glyphs, disable LCD as there's no way to support it.
    // CoreText doesn't tell us which format it ended up using, so we can't detect it.
    // A8 will end up black on transparent, but TODO: we can detect gray and set to A8.
    if (SkMask::kARGB32_Format == glyph.fMaskFormat) {
        doLCD = false;
    }

    size_t rowBytes = fSize.fWidth * sizeof(CGRGBPixel);
    if (!fCG || fSize.fWidth < glyph.fWidth || fSize.fHeight < glyph.fHeight) {
        if (fSize.fWidth < glyph.fWidth) {
            fSize.fWidth = RoundSize(glyph.fWidth);
        }
        if (fSize.fHeight < glyph.fHeight) {
            fSize.fHeight = RoundSize(glyph.fHeight);
        }

        rowBytes = fSize.fWidth * sizeof(CGRGBPixel);
        void* image = fImageStorage.reset(rowBytes * fSize.fHeight);
        const CGImageAlphaInfo alpha = (SkMask::kARGB32_Format == glyph.fMaskFormat)
                                     ? kCGImageAlphaPremultipliedFirst
                                     : kCGImageAlphaNoneSkipFirst;
        const CGBitmapInfo bitmapInfo = kCGBitmapByteOrder32Host | alpha;
        fCG.reset(CGBitmapContextCreate(image, fSize.fWidth, fSize.fHeight, 8,
                                        rowBytes, fRGBSpace.get(), bitmapInfo));

        // Skia handles quantization and subpixel positioning,
        // so disable quantization and enabe subpixel positioning in CG.
        CGContextSetAllowsFontSubpixelQuantization(fCG.get(), false);
        CGContextSetShouldSubpixelQuantizeFonts(fCG.get(), false);

        // Because CG always draws from the horizontal baseline,
        // if there is a non-integral translation from the horizontal origin to the vertical origin,
        // then CG cannot draw the glyph in the correct location without subpixel positioning.
        CGContextSetAllowsFontSubpixelPositioning(fCG.get(), true);
        CGContextSetShouldSubpixelPositionFonts(fCG.get(), true);

        CGContextSetTextDrawingMode(fCG.get(), kCGTextFill);

        // Draw black on white to create mask. (Special path exists to speed this up in CG.)
        CGContextSetGrayFillColor(fCG.get(), 0.0f, 1.0f);

        // force our checks below to happen
        fDoAA = !doAA;
        fDoLCD = !doLCD;

        CGContextSetTextMatrix(fCG.get(), context.fTransform);
    }

    if (fDoAA != doAA) {
        CGContextSetShouldAntialias(fCG.get(), doAA);
        fDoAA = doAA;
    }
    if (fDoLCD != doLCD) {
        CGContextSetShouldSmoothFonts(fCG.get(), doLCD);
        fDoLCD = doLCD;
    }

    CGRGBPixel* image = (CGRGBPixel*)fImageStorage.get();
    // skip rows based on the glyph's height
    image += (fSize.fHeight - glyph.fHeight) * fSize.fWidth;

    // Erase to white (or transparent black if it's a color glyph, to not composite against white).
    uint32_t bgColor = (SkMask::kARGB32_Format != glyph.fMaskFormat) ? 0xFFFFFFFF : 0x00000000;
    sk_memset_rect32(image, bgColor, glyph.fWidth, glyph.fHeight, rowBytes);

    float subX = 0;
    float subY = 0;
    if (context.fDoSubPosition) {
        subX = SkFixedToFloat(glyph.getSubXFixed());
        subY = SkFixedToFloat(glyph.getSubYFixed());
    }

    CGPoint point = CGPointMake(-glyph.fLeft + subX, glyph.fTop + glyph.fHeight - subY);
    // Prior to 10.10, CTFontDrawGlyphs acted like CGContextShowGlyphsAtPositions and took
    // 'positions' which are in text space. The glyph location (in device space) must be
    // mapped into text space, so that CG can convert it back into device space.
    // In 10.10.1, this is handled directly in CTFontDrawGlyphs.
    //
    // However, in 10.10.2 color glyphs no longer rotate based on the font transform.
    // So always make the font transform identity and place the transform on the context.
    point = CGPointApplyAffineTransform(point, context.fInvTransform);

    CTFontDrawGlyphs(context.fCTFont.get(), &glyphID, &point, 1, fCG.get());

    SkASSERT(rowBytesPtr);
    *rowBytesPtr = rowBytes;
    return image;
}

unsigned SkScalerContext_Mac::generateGlyphCount(void) {
    return fGlyphCount;
}

bool SkScalerContext_Mac::generateAdvance(SkGlyph* glyph) {
    return false;
}

void SkScalerContext_Mac::generateMetrics(SkGlyph* glyph) {
    AUTO_CG_LOCK();

    glyph->fMaskFormat = fRec.fMaskFormat;

    const CGGlyph cgGlyph = (CGGlyph) glyph->getGlyphID();
    glyph->zeroMetrics();

    // The following block produces cgAdvance in CG units (pixels, y up).
    CGSize cgAdvance;
    CTFontGetAdvancesForGlyphs(fCTFont.get(), kCTFontOrientationHorizontal,
                               &cgGlyph, &cgAdvance, 1);
    cgAdvance = CGSizeApplyAffineTransform(cgAdvance, fTransform);
    glyph->fAdvanceX =  CGToFloat(cgAdvance.width);
    glyph->fAdvanceY = -CGToFloat(cgAdvance.height);

    // The following produces skBounds in SkGlyph units (pixels, y down),
    // or returns early if skBounds would be empty.
    SkRect skBounds;

    // Glyphs are always drawn from the horizontal origin. The caller must manually use the result
    // of CTFontGetVerticalTranslationsForGlyphs to calculate where to draw the glyph for vertical
    // glyphs. As a result, always get the horizontal bounds of a glyph and translate it if the
    // glyph is vertical. This avoids any diagreement between the various means of retrieving
    // vertical metrics.
    {
        // CTFontGetBoundingRectsForGlyphs produces cgBounds in CG units (pixels, y up).
        CGRect cgBounds;
        CTFontGetBoundingRectsForGlyphs(fCTFont.get(), kCTFontOrientationHorizontal,
                                        &cgGlyph, &cgBounds, 1);
        cgBounds = CGRectApplyAffineTransform(cgBounds, fTransform);

        // BUG?
        // 0x200B (zero-advance space) seems to return a huge (garbage) bounds, when
        // it should be empty. So, if we see a zero-advance, we check if it has an
        // empty path or not, and if so, we jam the bounds to 0. Hopefully a zero-advance
        // is rare, so we won't incur a big performance cost for this extra check.
        if (0 == cgAdvance.width && 0 == cgAdvance.height) {
            SkUniqueCFRef<CGPathRef> path(CTFontCreatePathForGlyph(fCTFont.get(), cgGlyph,nullptr));
            if (!path || CGPathIsEmpty(path.get())) {
                return;
            }
        }

        if (CGRectIsEmpty_inline(cgBounds)) {
            return;
        }

        // Convert cgBounds to SkGlyph units (pixels, y down).
        skBounds = SkRect::MakeXYWH(cgBounds.origin.x, -cgBounds.origin.y - cgBounds.size.height,
                                    cgBounds.size.width, cgBounds.size.height);
    }

    // Currently the bounds are based on being rendered at (0,0).
    // The top left must not move, since that is the base from which subpixel positioning is offset.
    if (fDoSubPosition) {
        skBounds.fRight += SkFixedToFloat(glyph->getSubXFixed());
        skBounds.fBottom += SkFixedToFloat(glyph->getSubYFixed());
    }

    // We're trying to pack left and top into int16_t,
    // and width and height into uint16_t, after outsetting by 1.
    if (!SkRect::MakeXYWH(-32767, -32767, 65535, 65535).contains(skBounds)) {
        return;
    }

    SkIRect skIBounds;
    skBounds.roundOut(&skIBounds);
    // Expand the bounds by 1 pixel, to give CG room for anti-aliasing.
    // Note that this outset is to allow room for LCD smoothed glyphs. However, the correct outset
    // is not currently known, as CG dilates the outlines by some percentage.
    // Note that if this context is A8 and not back-forming from LCD, there is no need to outset.
    skIBounds.outset(1, 1);
    glyph->fLeft = SkToS16(skIBounds.fLeft);
    glyph->fTop = SkToS16(skIBounds.fTop);
    glyph->fWidth = SkToU16(skIBounds.width());
    glyph->fHeight = SkToU16(skIBounds.height());
}

#include "include/private/SkColorData.h"

static constexpr uint8_t sk_pow2_table(size_t i) {
    return SkToU8(((i * i + 128) / 255));
}

/**
 *  This will invert the gamma applied by CoreGraphics, so we can get linear
 *  values.
 *
 *  CoreGraphics obscurely defaults to 2.0 as the subpixel coverage gamma value.
 *  The color space used does not appear to affect this choice.
 */
static constexpr auto gLinearCoverageFromCGLCDValue = SkMakeArray<256>(sk_pow2_table);

static void cgpixels_to_bits(uint8_t dst[], const CGRGBPixel src[], int count) {
    while (count > 0) {
        uint8_t mask = 0;
        for (int i = 7; i >= 0; --i) {
            mask |= ((CGRGBPixel_getAlpha(*src++) >> 7) ^ 0x1) << i;
            if (0 == --count) {
                break;
            }
        }
        *dst++ = mask;
    }
}

template<bool APPLY_PREBLEND>
static inline uint8_t rgb_to_a8(CGRGBPixel rgb, const uint8_t* table8) {
    U8CPU r = 0xFF - ((rgb >> 16) & 0xFF);
    U8CPU g = 0xFF - ((rgb >>  8) & 0xFF);
    U8CPU b = 0xFF - ((rgb >>  0) & 0xFF);
    U8CPU lum = sk_apply_lut_if<APPLY_PREBLEND>(SkComputeLuminance(r, g, b), table8);
#if SK_SHOW_TEXT_BLIT_COVERAGE
    lum = SkTMax(lum, (U8CPU)0x30);
#endif
    return lum;
}
template<bool APPLY_PREBLEND>
static void rgb_to_a8(const CGRGBPixel* SK_RESTRICT cgPixels, size_t cgRowBytes,
                      const SkGlyph& glyph, const uint8_t* table8) {
    const int width = glyph.fWidth;
    size_t dstRB = glyph.rowBytes();
    uint8_t* SK_RESTRICT dst = (uint8_t*)glyph.fImage;

    for (int y = 0; y < glyph.fHeight; y++) {
        for (int i = 0; i < width; ++i) {
            dst[i] = rgb_to_a8<APPLY_PREBLEND>(cgPixels[i], table8);
        }
        cgPixels = SkTAddOffset<const CGRGBPixel>(cgPixels, cgRowBytes);
        dst = SkTAddOffset<uint8_t>(dst, dstRB);
    }
}

template<bool APPLY_PREBLEND>
static inline uint16_t rgb_to_lcd16(CGRGBPixel rgb, const uint8_t* tableR,
                                                    const uint8_t* tableG,
                                                    const uint8_t* tableB) {
    U8CPU r = sk_apply_lut_if<APPLY_PREBLEND>(0xFF - ((rgb >> 16) & 0xFF), tableR);
    U8CPU g = sk_apply_lut_if<APPLY_PREBLEND>(0xFF - ((rgb >>  8) & 0xFF), tableG);
    U8CPU b = sk_apply_lut_if<APPLY_PREBLEND>(0xFF - ((rgb >>  0) & 0xFF), tableB);
#if SK_SHOW_TEXT_BLIT_COVERAGE
    r = SkTMax(r, (U8CPU)0x30);
    g = SkTMax(g, (U8CPU)0x30);
    b = SkTMax(b, (U8CPU)0x30);
#endif
    return SkPack888ToRGB16(r, g, b);
}
template<bool APPLY_PREBLEND>
static void rgb_to_lcd16(const CGRGBPixel* SK_RESTRICT cgPixels, size_t cgRowBytes, const SkGlyph& glyph,
                         const uint8_t* tableR, const uint8_t* tableG, const uint8_t* tableB) {
    const int width = glyph.fWidth;
    size_t dstRB = glyph.rowBytes();
    uint16_t* SK_RESTRICT dst = (uint16_t*)glyph.fImage;

    for (int y = 0; y < glyph.fHeight; y++) {
        for (int i = 0; i < width; i++) {
            dst[i] = rgb_to_lcd16<APPLY_PREBLEND>(cgPixels[i], tableR, tableG, tableB);
        }
        cgPixels = SkTAddOffset<const CGRGBPixel>(cgPixels, cgRowBytes);
        dst = SkTAddOffset<uint16_t>(dst, dstRB);
    }
}

static SkPMColor cgpixels_to_pmcolor(CGRGBPixel rgb) {
    U8CPU a = (rgb >> 24) & 0xFF;
    U8CPU r = (rgb >> 16) & 0xFF;
    U8CPU g = (rgb >>  8) & 0xFF;
    U8CPU b = (rgb >>  0) & 0xFF;
#if SK_SHOW_TEXT_BLIT_COVERAGE
    a = SkTMax(a, (U8CPU)0x30);
#endif
    return SkPackARGB32(a, r, g, b);
}

void SkScalerContext_Mac::generateImage(const SkGlyph& glyph) {
    CGGlyph cgGlyph = SkTo<CGGlyph>(glyph.getGlyphID());

    // FIXME: lcd smoothed un-hinted rasterization unsupported.
    bool requestSmooth = fRec.getHinting() != SkFontHinting::kNone;

    // Draw the glyph
    size_t cgRowBytes;
    CGRGBPixel* cgPixels = fOffscreen.getCG(*this, glyph, cgGlyph, &cgRowBytes, requestSmooth);
    if (cgPixels == nullptr) {
        return;
    }

    // Fix the glyph
    if ((glyph.fMaskFormat == SkMask::kLCD16_Format) ||
        (glyph.fMaskFormat == SkMask::kA8_Format
         && requestSmooth
         && smooth_behavior() != SmoothBehavior::none))
    {
        const uint8_t* linear = gLinearCoverageFromCGLCDValue.data();

        //Note that the following cannot really be integrated into the
        //pre-blend, since we may not be applying the pre-blend; when we aren't
        //applying the pre-blend it means that a filter wants linear anyway.
        //Other code may also be applying the pre-blend, so we'd need another
        //one with this and one without.
        CGRGBPixel* addr = cgPixels;
        for (int y = 0; y < glyph.fHeight; ++y) {
            for (int x = 0; x < glyph.fWidth; ++x) {
                int r = (addr[x] >> 16) & 0xFF;
                int g = (addr[x] >>  8) & 0xFF;
                int b = (addr[x] >>  0) & 0xFF;
                addr[x] = (linear[r] << 16) | (linear[g] << 8) | linear[b];
            }
            addr = SkTAddOffset<CGRGBPixel>(addr, cgRowBytes);
        }
    }

    // Convert glyph to mask
    switch (glyph.fMaskFormat) {
        case SkMask::kLCD16_Format: {
            if (fPreBlend.isApplicable()) {
                rgb_to_lcd16<true>(cgPixels, cgRowBytes, glyph,
                                   fPreBlend.fR, fPreBlend.fG, fPreBlend.fB);
            } else {
                rgb_to_lcd16<false>(cgPixels, cgRowBytes, glyph,
                                    fPreBlend.fR, fPreBlend.fG, fPreBlend.fB);
            }
        } break;
        case SkMask::kA8_Format: {
            if (fPreBlend.isApplicable()) {
                rgb_to_a8<true>(cgPixels, cgRowBytes, glyph, fPreBlend.fG);
            } else {
                rgb_to_a8<false>(cgPixels, cgRowBytes, glyph, fPreBlend.fG);
            }
        } break;
        case SkMask::kBW_Format: {
            const int width = glyph.fWidth;
            size_t dstRB = glyph.rowBytes();
            uint8_t* dst = (uint8_t*)glyph.fImage;
            for (int y = 0; y < glyph.fHeight; y++) {
                cgpixels_to_bits(dst, cgPixels, width);
                cgPixels = SkTAddOffset<CGRGBPixel>(cgPixels, cgRowBytes);
                dst = SkTAddOffset<uint8_t>(dst, dstRB);
            }
        } break;
        case SkMask::kARGB32_Format: {
            const int width = glyph.fWidth;
            size_t dstRB = glyph.rowBytes();
            SkPMColor* dst = (SkPMColor*)glyph.fImage;
            for (int y = 0; y < glyph.fHeight; y++) {
                for (int x = 0; x < width; ++x) {
                    dst[x] = cgpixels_to_pmcolor(cgPixels[x]);
                }
                cgPixels = SkTAddOffset<CGRGBPixel>(cgPixels, cgRowBytes);
                dst = SkTAddOffset<SkPMColor>(dst, dstRB);
            }
        } break;
        default:
            SkDEBUGFAIL("unexpected mask format");
            break;
    }
}

/*
 *  Our subpixel resolution is only 2 bits in each direction, so a scale of 4
 *  seems sufficient, and possibly even correct, to allow the hinted outline
 *  to be subpixel positioned.
 */
#define kScaleForSubPixelPositionHinting (4.0f)

bool SkScalerContext_Mac::generatePath(SkGlyphID glyph, SkPath* path) {
    AUTO_CG_LOCK();

    SkScalar scaleX = SK_Scalar1;
    SkScalar scaleY = SK_Scalar1;

    CGAffineTransform xform = fTransform;
    /*
     *  For subpixel positioning, we want to return an unhinted outline, so it
     *  can be positioned nicely at fractional offsets. However, we special-case
     *  if the baseline of the (horizontal) text is axis-aligned. In those cases
     *  we want to retain hinting in the direction orthogonal to the baseline.
     *  e.g. for horizontal baseline, we want to retain hinting in Y.
     *  The way we remove hinting is to scale the font by some value (4) in that
     *  direction, ask for the path, and then scale the path back down.
     */
    if (fDoSubPosition) {
        // start out by assuming that we want no hining in X and Y
        scaleX = scaleY = kScaleForSubPixelPositionHinting;
        // now see if we need to restore hinting for axis-aligned baselines
        switch (this->computeAxisAlignmentForHText()) {
            case kX_SkAxisAlignment:
                scaleY = SK_Scalar1; // want hinting in the Y direction
                break;
            case kY_SkAxisAlignment:
                scaleX = SK_Scalar1; // want hinting in the X direction
                break;
            default:
                break;
        }

        CGAffineTransform scale(CGAffineTransformMakeScale(ScalarToCG(scaleX), ScalarToCG(scaleY)));
        xform = CGAffineTransformConcat(fTransform, scale);
    }

    CGGlyph cgGlyph = SkTo<CGGlyph>(glyph);
    SkUniqueCFRef<CGPathRef> cgPath(CTFontCreatePathForGlyph(fCTFont.get(), cgGlyph, &xform));

    path->reset();
    if (!cgPath) {
        return false;
    }

    CGPathApply(cgPath.get(), path, SkScalerContext_Mac::CTPathElement);
    if (fDoSubPosition) {
        SkMatrix m;
        m.setScale(SkScalarInvert(scaleX), SkScalarInvert(scaleY));
        path->transform(m);
    }
    return true;
}

void SkScalerContext_Mac::generateFontMetrics(SkFontMetrics* metrics) {
    if (nullptr == metrics) {
        return;
    }

    AUTO_CG_LOCK();

    CGRect theBounds = CTFontGetBoundingBox(fCTFont.get());

    metrics->fTop          = CGToScalar(-CGRectGetMaxY_inline(theBounds));
    metrics->fAscent       = CGToScalar(-CTFontGetAscent(fCTFont.get()));
    metrics->fDescent      = CGToScalar( CTFontGetDescent(fCTFont.get()));
    metrics->fBottom       = CGToScalar(-CGRectGetMinY_inline(theBounds));
    metrics->fLeading      = CGToScalar( CTFontGetLeading(fCTFont.get()));
    metrics->fAvgCharWidth = CGToScalar( CGRectGetWidth_inline(theBounds));
    metrics->fXMin         = CGToScalar( CGRectGetMinX_inline(theBounds));
    metrics->fXMax         = CGToScalar( CGRectGetMaxX_inline(theBounds));
    metrics->fMaxCharWidth = metrics->fXMax - metrics->fXMin;
    metrics->fXHeight      = CGToScalar( CTFontGetXHeight(fCTFont.get()));
    metrics->fCapHeight    = CGToScalar( CTFontGetCapHeight(fCTFont.get()));
    metrics->fUnderlineThickness = CGToScalar( CTFontGetUnderlineThickness(fCTFont.get()));
    metrics->fUnderlinePosition = -CGToScalar( CTFontGetUnderlinePosition(fCTFont.get()));

    metrics->fFlags = 0;
    metrics->fFlags |= SkFontMetrics::kUnderlineThicknessIsValid_Flag;
    metrics->fFlags |= SkFontMetrics::kUnderlinePositionIsValid_Flag;

    // See https://bugs.chromium.org/p/skia/issues/detail?id=6203
    // At least on 10.12.3 with memory based fonts the x-height is always 0.6666 of the ascent and
    // the cap-height is always 0.8888 of the ascent. It appears that the values from the 'OS/2'
    // table are read, but then overwritten if the font is not a system font. As a result, if there
    // is a valid 'OS/2' table available use the values from the table if they aren't too strange.
    struct OS2HeightMetrics {
        SK_OT_SHORT sxHeight;
        SK_OT_SHORT sCapHeight;
    } heights;
    size_t bytesRead = this->getTypeface()->getTableData(
            SkTEndian_SwapBE32(SkOTTableOS2::TAG), offsetof(SkOTTableOS2, version.v2.sxHeight),
            sizeof(heights), &heights);
    if (bytesRead == sizeof(heights)) {
        // 'fontSize' is correct because the entire resolved size is set by the constructor.
        CGFloat fontSize = CTFontGetSize(this->fCTFont.get());
        unsigned upem = CTFontGetUnitsPerEm(this->fCTFont.get());
        unsigned maxSaneHeight = upem * 2;
        uint16_t xHeight = SkEndian_SwapBE16(heights.sxHeight);
        if (xHeight && xHeight < maxSaneHeight) {
            metrics->fXHeight = CGToScalar(xHeight * fontSize / upem);
        }
        uint16_t capHeight = SkEndian_SwapBE16(heights.sCapHeight);
        if (capHeight && capHeight < maxSaneHeight) {
            metrics->fCapHeight = CGToScalar(capHeight * fontSize / upem);
        }
    }
}

void SkScalerContext_Mac::CTPathElement(void *info, const CGPathElement *element) {
    SkPath* skPath = (SkPath*)info;

    // Process the path element
    switch (element->type) {
        case kCGPathElementMoveToPoint:
            skPath->moveTo(element->points[0].x, -element->points[0].y);
            break;

        case kCGPathElementAddLineToPoint:
            skPath->lineTo(element->points[0].x, -element->points[0].y);
            break;

        case kCGPathElementAddQuadCurveToPoint:
            skPath->quadTo(element->points[0].x, -element->points[0].y,
                           element->points[1].x, -element->points[1].y);
            break;

        case kCGPathElementAddCurveToPoint:
            skPath->cubicTo(element->points[0].x, -element->points[0].y,
                            element->points[1].x, -element->points[1].y,
                            element->points[2].x, -element->points[2].y);
            break;

        case kCGPathElementCloseSubpath:
            skPath->close();
            break;

        default:
            SkDEBUGFAIL("Unknown path element!");
            break;
        }
}


///////////////////////////////////////////////////////////////////////////////

// Returns nullptr on failure
// Call must still manage its ownership of provider
static sk_sp<SkTypeface> create_from_dataProvider(SkUniqueCFRef<CGDataProviderRef> provider,
                                                  std::unique_ptr<SkStreamAsset> providedData,
                                                  int ttcIndex) {
    if (ttcIndex != 0) {
        return nullptr;
    }
    SkUniqueCFRef<CGFontRef> cg(CGFontCreateWithDataProvider(provider.get()));
    if (!cg) {
        return nullptr;
    }
    SkUniqueCFRef<CTFontRef> ct(CTFontCreateWithGraphicsFont(cg.get(), 0, nullptr, nullptr));
    if (!ct) {
        return nullptr;
    }
    return create_from_CTFontRef(std::move(ct), nullptr, std::move(providedData));
}

// Web fonts added to the CTFont registry do not return their character set.
// Iterate through the font in this case. The existing caller caches the result,
// so the performance impact isn't too bad.
static void populate_glyph_to_unicode_slow(CTFontRef ctFont, CFIndex glyphCount,
                                           SkUnichar* out) {
    sk_bzero(out, glyphCount * sizeof(SkUnichar));
    UniChar unichar = 0;
    while (glyphCount > 0) {
        CGGlyph glyph;
        if (CTFontGetGlyphsForCharacters(ctFont, &unichar, &glyph, 1)) {
            if (out[glyph] == 0) {
                out[glyph] = unichar;
                --glyphCount;
            }
        }
        if (++unichar == 0) {
            break;
        }
    }
}

// Construct Glyph to Unicode table.
// Unicode code points that require conjugate pairs in utf16 are not
// supported.
static void populate_glyph_to_unicode(CTFontRef ctFont, CFIndex glyphCount,
                                      SkUnichar* glyphToUnicode) {
    sk_bzero(glyphToUnicode, sizeof(SkUnichar) * glyphCount);
    SkUniqueCFRef<CFCharacterSetRef> charSet(CTFontCopyCharacterSet(ctFont));
    if (!charSet) {
        populate_glyph_to_unicode_slow(ctFont, glyphCount, glyphToUnicode);
        return;
    }

    SkUniqueCFRef<CFDataRef> bitmap(
            CFCharacterSetCreateBitmapRepresentation(nullptr, charSet.get()));
    if (!bitmap) {
        return;
    }
    CFIndex length = CFDataGetLength(bitmap.get());
    if (!length) {
        return;
    }
    if (length > 8192) {
        // TODO: Add support for Unicode above 0xFFFF
        // Consider only the BMP portion of the Unicode character points.
        // The bitmap may contain other planes, up to plane 16.
        // See http://developer.apple.com/library/ios/#documentation/CoreFoundation/Reference/CFCharacterSetRef/Reference/reference.html
        length = 8192;
    }
    const UInt8* bits = CFDataGetBytePtr(bitmap.get());
    sk_bzero(glyphToUnicode, glyphCount * sizeof(SkUnichar));
    for (int i = 0; i < length; i++) {
        int mask = bits[i];
        if (!mask) {
            continue;
        }
        for (int j = 0; j < 8; j++) {
            CGGlyph glyph;
            UniChar unichar = static_cast<UniChar>((i << 3) + j);
            if (mask & (1 << j) && CTFontGetGlyphsForCharacters(ctFont, &unichar, &glyph, 1)) {
                if (glyphToUnicode[glyph] == 0) {
                    glyphToUnicode[glyph] = unichar;
                }
            }
        }
    }
}

/** Assumes src and dst are not nullptr. */
static void CFStringToSkString(CFStringRef src, SkString* dst) {
    // Reserve enough room for the worst-case string,
    // plus 1 byte for the trailing null.
    CFIndex length = CFStringGetMaximumSizeForEncoding(CFStringGetLength(src),
                                                       kCFStringEncodingUTF8) + 1;
    dst->resize(length);
    CFStringGetCString(src, dst->writable_str(), length, kCFStringEncodingUTF8);
    // Resize to the actual UTF-8 length used, stripping the null character.
    dst->resize(strlen(dst->c_str()));
}

void SkTypeface_Mac::getGlyphToUnicodeMap(SkUnichar* dstArray) const {
    AUTO_CG_LOCK();
    SkUniqueCFRef<CTFontRef> ctFont =
            ctfont_create_exact_copy(fFontRef.get(), CTFontGetUnitsPerEm(fFontRef.get()), nullptr);
    CFIndex glyphCount = CTFontGetGlyphCount(ctFont.get());
    populate_glyph_to_unicode(ctFont.get(), glyphCount, dstArray);
}

std::unique_ptr<SkAdvancedTypefaceMetrics> SkTypeface_Mac::onGetAdvancedMetrics() const {

    AUTO_CG_LOCK();

    SkUniqueCFRef<CTFontRef> ctFont =
            ctfont_create_exact_copy(fFontRef.get(), CTFontGetUnitsPerEm(fFontRef.get()), nullptr);

    std::unique_ptr<SkAdvancedTypefaceMetrics> info(new SkAdvancedTypefaceMetrics);

    {
        SkUniqueCFRef<CFStringRef> fontName(CTFontCopyPostScriptName(ctFont.get()));
        if (fontName.get()) {
            CFStringToSkString(fontName.get(), &info->fPostScriptName);
            info->fFontName = info->fPostScriptName;
        }
    }

    // In 10.10 and earlier, CTFontCopyVariationAxes and CTFontCopyVariation do not work when
    // applied to fonts which started life with CGFontCreateWithDataProvider (they simply always
    // return nullptr). As a result, we are limited to CGFontCopyVariationAxes and
    // CGFontCopyVariations here until support for 10.10 and earlier is removed.
    SkUniqueCFRef<CGFontRef> cgFont(CTFontCopyGraphicsFont(ctFont.get(), nullptr));
    if (cgFont) {
        SkUniqueCFRef<CFArrayRef> cgAxes(CGFontCopyVariationAxes(cgFont.get()));
        if (cgAxes && CFArrayGetCount(cgAxes.get()) > 0) {
            info->fFlags |= SkAdvancedTypefaceMetrics::kMultiMaster_FontFlag;
        }
    }

    SkOTTableOS2_V4::Type fsType;
    if (sizeof(fsType) == this->getTableData(SkTEndian_SwapBE32(SkOTTableOS2::TAG),
                                             offsetof(SkOTTableOS2_V4, fsType),
                                             sizeof(fsType),
                                             &fsType)) {
        SkOTUtils::SetAdvancedTypefaceFlags(fsType, info.get());
    }

    // If it's not a truetype font, mark it as 'other'. Assume that TrueType
    // fonts always have both glyf and loca tables. At the least, this is what
    // sfntly needs to subset the font. CTFontCopyAttribute() does not always
    // succeed in determining this directly.
    if (!this->getTableSize('glyf') || !this->getTableSize('loca')) {
        return info;
    }

    info->fType = SkAdvancedTypefaceMetrics::kTrueType_Font;
    CTFontSymbolicTraits symbolicTraits = CTFontGetSymbolicTraits(ctFont.get());
    if (symbolicTraits & kCTFontMonoSpaceTrait) {
        info->fStyle |= SkAdvancedTypefaceMetrics::kFixedPitch_Style;
    }
    if (symbolicTraits & kCTFontItalicTrait) {
        info->fStyle |= SkAdvancedTypefaceMetrics::kItalic_Style;
    }
    CTFontStylisticClass stylisticClass = symbolicTraits & kCTFontClassMaskTrait;
    if (stylisticClass >= kCTFontOldStyleSerifsClass && stylisticClass <= kCTFontSlabSerifsClass) {
        info->fStyle |= SkAdvancedTypefaceMetrics::kSerif_Style;
    } else if (stylisticClass & kCTFontScriptsClass) {
        info->fStyle |= SkAdvancedTypefaceMetrics::kScript_Style;
    }
    info->fItalicAngle = (int16_t) CTFontGetSlantAngle(ctFont.get());
    info->fAscent = (int16_t) CTFontGetAscent(ctFont.get());
    info->fDescent = (int16_t) CTFontGetDescent(ctFont.get());
    info->fCapHeight = (int16_t) CTFontGetCapHeight(ctFont.get());
    CGRect bbox = CTFontGetBoundingBox(ctFont.get());

    SkRect r;
    r.set( CGToScalar(CGRectGetMinX_inline(bbox)),   // Left
           CGToScalar(CGRectGetMaxY_inline(bbox)),   // Top
           CGToScalar(CGRectGetMaxX_inline(bbox)),   // Right
           CGToScalar(CGRectGetMinY_inline(bbox)));  // Bottom

    r.roundOut(&(info->fBBox));

    // Figure out a good guess for StemV - Min width of i, I, !, 1.
    // This probably isn't very good with an italic font.
    int16_t min_width = SHRT_MAX;
    info->fStemV = 0;
    static const UniChar stem_chars[] = {'i', 'I', '!', '1'};
    const size_t count = sizeof(stem_chars) / sizeof(stem_chars[0]);
    CGGlyph glyphs[count];
    CGRect boundingRects[count];
    if (CTFontGetGlyphsForCharacters(ctFont.get(), stem_chars, glyphs, count)) {
        CTFontGetBoundingRectsForGlyphs(ctFont.get(), kCTFontOrientationHorizontal,
                                        glyphs, boundingRects, count);
        for (size_t i = 0; i < count; i++) {
            int16_t width = (int16_t) boundingRects[i].size.width;
            if (width > 0 && width < min_width) {
                min_width = width;
                info->fStemV = min_width;
            }
        }
    }
    return info;
}

///////////////////////////////////////////////////////////////////////////////

static SK_SFNT_ULONG get_font_type_tag(CTFontRef ctFont) {
    SkUniqueCFRef<CFNumberRef> fontFormatRef(
            static_cast<CFNumberRef>(CTFontCopyAttribute(ctFont, kCTFontFormatAttribute)));
    if (!fontFormatRef) {
        return 0;
    }

    SInt32 fontFormatValue;
    if (!CFNumberGetValue(fontFormatRef.get(), kCFNumberSInt32Type, &fontFormatValue)) {
        return 0;
    }

    switch (fontFormatValue) {
        case kCTFontFormatOpenTypePostScript:
            return SkSFNTHeader::fontType_OpenTypeCFF::TAG;
        case kCTFontFormatOpenTypeTrueType:
            return SkSFNTHeader::fontType_WindowsTrueType::TAG;
        case kCTFontFormatTrueType:
            return SkSFNTHeader::fontType_MacTrueType::TAG;
        case kCTFontFormatPostScript:
            return SkSFNTHeader::fontType_PostScript::TAG;
        case kCTFontFormatBitmap:
            return SkSFNTHeader::fontType_MacTrueType::TAG;
        case kCTFontFormatUnrecognized:
        default:
            return 0;
    }
}

std::unique_ptr<SkStreamAsset> SkTypeface_Mac::onOpenStream(int* ttcIndex) const {
    *ttcIndex = 0;

    fInitStream([this]{
    if (fStream) {
        return;
    }

    SK_SFNT_ULONG fontType = get_font_type_tag(fFontRef.get());

    // get table tags
    int numTables = this->countTables();
    SkTDArray<SkFontTableTag> tableTags;
    tableTags.setCount(numTables);
    this->getTableTags(tableTags.begin());

    // CT seems to be unreliable in being able to obtain the type,
    // even if all we want is the first four bytes of the font resource.
    // Just the presence of the FontForge 'FFTM' table seems to throw it off.
    if (fontType == 0) {
        fontType = SkSFNTHeader::fontType_WindowsTrueType::TAG;

        // see https://skbug.com/7630#c7
        bool couldBeCFF = false;
        constexpr SkFontTableTag CFFTag = SkSetFourByteTag('C', 'F', 'F', ' ');
        constexpr SkFontTableTag CFF2Tag = SkSetFourByteTag('C', 'F', 'F', '2');
        for (int tableIndex = 0; tableIndex < numTables; ++tableIndex) {
            if (CFFTag == tableTags[tableIndex] || CFF2Tag == tableTags[tableIndex]) {
                couldBeCFF = true;
            }
        }
        if (couldBeCFF) {
            fontType = SkSFNTHeader::fontType_OpenTypeCFF::TAG;
        }
    }

    // Sometimes CoreGraphics incorrectly thinks a font is kCTFontFormatPostScript.
    // It is exceedingly unlikely that this is the case, so double check
    // (see https://crbug.com/809763 ).
    if (fontType == SkSFNTHeader::fontType_PostScript::TAG) {
        // see if there are any required 'typ1' tables (see Adobe Technical Note #5180)
        bool couldBeTyp1 = false;
        constexpr SkFontTableTag TYPE1Tag = SkSetFourByteTag('T', 'Y', 'P', '1');
        constexpr SkFontTableTag CIDTag = SkSetFourByteTag('C', 'I', 'D', ' ');
        for (int tableIndex = 0; tableIndex < numTables; ++tableIndex) {
            if (TYPE1Tag == tableTags[tableIndex] || CIDTag == tableTags[tableIndex]) {
                couldBeTyp1 = true;
            }
        }
        if (!couldBeTyp1) {
            fontType = SkSFNTHeader::fontType_OpenTypeCFF::TAG;
        }
    }

    // get the table sizes and accumulate the total size of the font
    SkTDArray<size_t> tableSizes;
    size_t totalSize = sizeof(SkSFNTHeader) + sizeof(SkSFNTHeader::TableDirectoryEntry) * numTables;
    for (int tableIndex = 0; tableIndex < numTables; ++tableIndex) {
        size_t tableSize = this->getTableSize(tableTags[tableIndex]);
        totalSize += (tableSize + 3) & ~3;
        *tableSizes.append() = tableSize;
    }

    // reserve memory for stream, and zero it (tables must be zero padded)
    fStream.reset(new SkMemoryStream(totalSize));
    char* dataStart = (char*)fStream->getMemoryBase();
    sk_bzero(dataStart, totalSize);
    char* dataPtr = dataStart;

    // compute font header entries
    uint16_t entrySelector = 0;
    uint16_t searchRange = 1;
    while (searchRange < numTables >> 1) {
        entrySelector++;
        searchRange <<= 1;
    }
    searchRange <<= 4;
    uint16_t rangeShift = (numTables << 4) - searchRange;

    // write font header
    SkSFNTHeader* header = (SkSFNTHeader*)dataPtr;
    header->fontType = fontType;
    header->numTables = SkEndian_SwapBE16(numTables);
    header->searchRange = SkEndian_SwapBE16(searchRange);
    header->entrySelector = SkEndian_SwapBE16(entrySelector);
    header->rangeShift = SkEndian_SwapBE16(rangeShift);
    dataPtr += sizeof(SkSFNTHeader);

    // write tables
    SkSFNTHeader::TableDirectoryEntry* entry = (SkSFNTHeader::TableDirectoryEntry*)dataPtr;
    dataPtr += sizeof(SkSFNTHeader::TableDirectoryEntry) * numTables;
    for (int tableIndex = 0; tableIndex < numTables; ++tableIndex) {
        size_t tableSize = tableSizes[tableIndex];
        this->getTableData(tableTags[tableIndex], 0, tableSize, dataPtr);
        entry->tag = SkEndian_SwapBE32(tableTags[tableIndex]);
        entry->checksum = SkEndian_SwapBE32(SkOTUtils::CalcTableChecksum((SK_OT_ULONG*)dataPtr,
                                                                         tableSize));
        entry->offset = SkEndian_SwapBE32(SkToU32(dataPtr - dataStart));
        entry->logicalLength = SkEndian_SwapBE32(SkToU32(tableSize));

        dataPtr += (tableSize + 3) & ~3;
        ++entry;
    }
    });
    return fStream->duplicate();
}

struct NonDefaultAxesContext {
    SkFixed* axisValue;
    CFArrayRef cgAxes;
};
static void set_non_default_axes(CFTypeRef key, CFTypeRef value, void* context) {
    NonDefaultAxesContext* self = static_cast<NonDefaultAxesContext*>(context);

    if (CFGetTypeID(key) != CFStringGetTypeID() || CFGetTypeID(value) != CFNumberGetTypeID()) {
        return;
    }

    // The key is a CFString which is a string from the 'name' table.
    // Search the cgAxes for an axis with this name, and use its index to store the value.
    CFIndex keyIndex = -1;
    CFStringRef keyString = static_cast<CFStringRef>(key);
    for (CFIndex i = 0; i < CFArrayGetCount(self->cgAxes); ++i) {
        CFTypeRef cgAxis = CFArrayGetValueAtIndex(self->cgAxes, i);
        if (CFGetTypeID(cgAxis) != CFDictionaryGetTypeID()) {
            continue;
        }

        CFDictionaryRef cgAxisDict = static_cast<CFDictionaryRef>(cgAxis);
        CFTypeRef cgAxisName = CFDictionaryGetValue(cgAxisDict, kCGFontVariationAxisName);
        if (!cgAxisName || CFGetTypeID(cgAxisName) != CFStringGetTypeID()) {
            continue;
        }
        CFStringRef cgAxisNameString = static_cast<CFStringRef>(cgAxisName);
        if (CFStringCompare(keyString, cgAxisNameString, 0) == kCFCompareEqualTo) {
            keyIndex = i;
            break;
        }
    }
    if (keyIndex == -1) {
        return;
    }

    CFNumberRef valueNumber = static_cast<CFNumberRef>(value);
    double valueDouble;
    if (!CFNumberGetValue(valueNumber, kCFNumberDoubleType, &valueDouble) ||
        valueDouble < SkFixedToDouble(SK_FixedMin) || SkFixedToDouble(SK_FixedMax) < valueDouble)
    {
        return;
    }
    self->axisValue[keyIndex] = SkDoubleToFixed(valueDouble);
}
static bool get_variations(CTFontRef ctFont, CFIndex* cgAxisCount,
                           SkAutoSTMalloc<4, SkFixed>* axisValues)
{
    // In 10.10 and earlier, CTFontCopyVariationAxes and CTFontCopyVariation do not work when
    // applied to fonts which started life with CGFontCreateWithDataProvider (they simply always
    // return nullptr). As a result, we are limited to CGFontCopyVariationAxes and
    // CGFontCopyVariations here until support for 10.10 and earlier is removed.
    SkUniqueCFRef<CGFontRef> cgFont(CTFontCopyGraphicsFont(ctFont, nullptr));
    if (!cgFont) {
        return false;
    }

    SkUniqueCFRef<CFDictionaryRef> cgVariations(CGFontCopyVariations(cgFont.get()));
    // If a font has no variations CGFontCopyVariations returns nullptr (instead of an empty dict).
    if (!cgVariations) {
        return false;
    }

    SkUniqueCFRef<CFArrayRef> cgAxes(CGFontCopyVariationAxes(cgFont.get()));
    if (!cgAxes) {
        return false;
    }
    *cgAxisCount = CFArrayGetCount(cgAxes.get());
    axisValues->reset(*cgAxisCount);

    // Set all of the axes to their default values.
    // Fail if any default value cannot be determined.
    for (CFIndex i = 0; i < *cgAxisCount; ++i) {
        CFTypeRef cgAxis = CFArrayGetValueAtIndex(cgAxes.get(), i);
        if (CFGetTypeID(cgAxis) != CFDictionaryGetTypeID()) {
            return false;
        }

        CFDictionaryRef cgAxisDict = static_cast<CFDictionaryRef>(cgAxis);
        CFTypeRef axisDefaultValue = CFDictionaryGetValue(cgAxisDict,
                                                          kCGFontVariationAxisDefaultValue);
        if (!axisDefaultValue || CFGetTypeID(axisDefaultValue) != CFNumberGetTypeID()) {
            return false;
        }
        CFNumberRef axisDefaultValueNumber = static_cast<CFNumberRef>(axisDefaultValue);
        double axisDefaultValueDouble;
        if (!CFNumberGetValue(axisDefaultValueNumber, kCFNumberDoubleType, &axisDefaultValueDouble))
        {
            return false;
        }
        if (axisDefaultValueDouble < SkFixedToDouble(SK_FixedMin) ||
                                     SkFixedToDouble(SK_FixedMax) < axisDefaultValueDouble)
        {
            return false;
        }
        (*axisValues)[(int)i] = SkDoubleToFixed(axisDefaultValueDouble);
    }

    // Override the default values with the given font's stated axis values.
    NonDefaultAxesContext c = { axisValues->get(), cgAxes.get() };
    CFDictionaryApplyFunction(cgVariations.get(), set_non_default_axes, &c);

    return true;
}
std::unique_ptr<SkFontData> SkTypeface_Mac::onMakeFontData() const {
    int index;
    std::unique_ptr<SkStreamAsset> stream(this->onOpenStream(&index));

    CFIndex cgAxisCount;
    SkAutoSTMalloc<4, SkFixed> axisValues;
    if (get_variations(fFontRef.get(), &cgAxisCount, &axisValues)) {
        return skstd::make_unique<SkFontData>(std::move(stream), index,
                                              axisValues.get(), cgAxisCount);
    }
    return skstd::make_unique<SkFontData>(std::move(stream), index, nullptr, 0);
}

/** Creates a CT variation dictionary {tag, value} from a CG variation dictionary {name, value}. */
static SkUniqueCFRef<CFDictionaryRef> ct_variation_from_cg_variation(CFDictionaryRef cgVariations,
                                                                     CFArrayRef ctAxes) {

    SkUniqueCFRef<CFMutableDictionaryRef> ctVariations(
            CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
                                      &kCFTypeDictionaryKeyCallBacks,
                                      &kCFTypeDictionaryValueCallBacks));

    CFIndex axisCount = CFArrayGetCount(ctAxes);
    for (CFIndex i = 0; i < axisCount; ++i) {
        CFTypeRef axisInfo = CFArrayGetValueAtIndex(ctAxes, i);
        if (CFDictionaryGetTypeID() != CFGetTypeID(axisInfo)) {
            return nullptr;
        }
        CFDictionaryRef axisInfoDict = static_cast<CFDictionaryRef>(axisInfo);

        // The assumption is that values produced by kCTFontVariationAxisNameKey and
        // kCGFontVariationAxisName will always be equal.
        CFTypeRef axisName = CFDictionaryGetValue(axisInfoDict, kCTFontVariationAxisNameKey);
        if (!axisName || CFGetTypeID(axisName) != CFStringGetTypeID()) {
            return nullptr;
        }

        CFTypeRef axisValue = CFDictionaryGetValue(cgVariations, axisName);
        if (!axisValue || CFGetTypeID(axisValue) != CFNumberGetTypeID()) {
            return nullptr;
        }

        CFTypeRef axisTag = CFDictionaryGetValue(axisInfoDict, kCTFontVariationAxisIdentifierKey);
        if (!axisTag || CFGetTypeID(axisTag) != CFNumberGetTypeID()) {
            return nullptr;
        }

        CFDictionaryAddValue(ctVariations.get(), axisTag, axisValue);
    }
    return std::move(ctVariations);
}

int SkTypeface_Mac::onGetVariationDesignPosition(
        SkFontArguments::VariationPosition::Coordinate coordinates[], int coordinateCount) const
{
    // The CGFont variation data does not contain the tag.

    // CTFontCopyVariationAxes returns nullptr for CGFontCreateWithDataProvider fonts with
    // macOS 10.10 and iOS 9 or earlier. When this happens, there is no API to provide the tag.
    SkUniqueCFRef<CFArrayRef> ctAxes(CTFontCopyVariationAxes(fFontRef.get()));
    if (!ctAxes) {
        return -1;
    }
    CFIndex axisCount = CFArrayGetCount(ctAxes.get());
    if (!coordinates || coordinateCount < axisCount) {
        return axisCount;
    }

    // This call always returns nullptr on 10.11 and under for CGFontCreateWithDataProvider fonts.
    // When this happens, try converting the CG variation to a CT variation.
    // On 10.12 and later, this only returns non-default variations.
    SkUniqueCFRef<CFDictionaryRef> ctVariations(CTFontCopyVariation(fFontRef.get()));
    if (!ctVariations) {
        // When 10.11 and earlier are no longer supported, the following code can be replaced with
        // return -1 and ct_variation_from_cg_variation can be removed.
        SkUniqueCFRef<CGFontRef> cgFont(CTFontCopyGraphicsFont(fFontRef.get(), nullptr));
        if (!cgFont) {
            return -1;
        }
        SkUniqueCFRef<CFDictionaryRef> cgVariations(CGFontCopyVariations(cgFont.get()));
        if (!cgVariations) {
            return -1;
        }
        ctVariations = ct_variation_from_cg_variation(cgVariations.get(), ctAxes.get());
        if (!ctVariations) {
            return -1;
        }
    }

    for (int i = 0; i < axisCount; ++i) {
        CFTypeRef axisInfo = CFArrayGetValueAtIndex(ctAxes.get(), i);
        if (CFDictionaryGetTypeID() != CFGetTypeID(axisInfo)) {
            return -1;
        }
        CFDictionaryRef axisInfoDict = static_cast<CFDictionaryRef>(axisInfo);

        CFTypeRef tag = CFDictionaryGetValue(axisInfoDict, kCTFontVariationAxisIdentifierKey);
        if (!tag || CFGetTypeID(tag) != CFNumberGetTypeID()) {
            return -1;
        }
        CFNumberRef tagNumber = static_cast<CFNumberRef>(tag);
        int64_t tagLong;
        if (!CFNumberGetValue(tagNumber, kCFNumberSInt64Type, &tagLong)) {
            return -1;
        }
        coordinates[i].axis = tagLong;

        CGFloat variationCGFloat;
        CFTypeRef variationValue = CFDictionaryGetValue(ctVariations.get(), tagNumber);
        if (variationValue) {
            if (CFGetTypeID(variationValue) != CFNumberGetTypeID()) {
                return -1;
            }
            CFNumberRef variationNumber = static_cast<CFNumberRef>(variationValue);
            if (!CFNumberGetValue(variationNumber, kCFNumberCGFloatType, &variationCGFloat)) {
                return -1;
            }
        } else {
            CFTypeRef def = CFDictionaryGetValue(axisInfoDict, kCTFontVariationAxisDefaultValueKey);
            if (!def || CFGetTypeID(def) != CFNumberGetTypeID()) {
                return -1;
            }
            CFNumberRef defNumber = static_cast<CFNumberRef>(def);
            if (!CFNumberGetValue(defNumber, kCFNumberCGFloatType, &variationCGFloat)) {
                return -1;
            }
        }
        coordinates[i].value = CGToScalar(variationCGFloat);

    }
    return axisCount;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int SkTypeface_Mac::onGetUPEM() const {
    SkUniqueCFRef<CGFontRef> cgFont(CTFontCopyGraphicsFont(fFontRef.get(), nullptr));
    return CGFontGetUnitsPerEm(cgFont.get());
}

SkTypeface::LocalizedStrings* SkTypeface_Mac::onCreateFamilyNameIterator() const {
    sk_sp<SkTypeface::LocalizedStrings> nameIter =
            SkOTUtils::LocalizedStrings_NameTable::MakeForFamilyNames(*this);
    if (!nameIter) {
        CFStringRef cfLanguageRaw;
        SkUniqueCFRef<CFStringRef> cfFamilyName(
                CTFontCopyLocalizedName(fFontRef.get(), kCTFontFamilyNameKey, &cfLanguageRaw));
        SkUniqueCFRef<CFStringRef> cfLanguage(cfLanguageRaw);

        SkString skLanguage;
        SkString skFamilyName;
        if (cfLanguage) {
            CFStringToSkString(cfLanguage.get(), &skLanguage);
        } else {
            skLanguage = "und"; //undetermined
        }
        if (cfFamilyName) {
            CFStringToSkString(cfFamilyName.get(), &skFamilyName);
        }

        nameIter = sk_make_sp<SkOTUtils::LocalizedStrings_SingleName>(skFamilyName, skLanguage);
    }
    return nameIter.release();
}

int SkTypeface_Mac::onGetTableTags(SkFontTableTag tags[]) const {
    SkUniqueCFRef<CFArrayRef> cfArray(
            CTFontCopyAvailableTables(fFontRef.get(), kCTFontTableOptionNoOptions));
    if (!cfArray) {
        return 0;
    }
    int count = SkToInt(CFArrayGetCount(cfArray.get()));
    if (tags) {
        for (int i = 0; i < count; ++i) {
            uintptr_t fontTag = reinterpret_cast<uintptr_t>(
                CFArrayGetValueAtIndex(cfArray.get(), i));
            tags[i] = static_cast<SkFontTableTag>(fontTag);
        }
    }
    return count;
}

// If, as is the case with web fonts, the CTFont data isn't available,
// the CGFont data may work. While the CGFont may always provide the
// right result, leave the CTFont code path to minimize disruption.
static SkUniqueCFRef<CFDataRef> copy_table_from_font(CTFontRef ctFont, SkFontTableTag tag) {
    SkUniqueCFRef<CFDataRef> data(CTFontCopyTable(ctFont, (CTFontTableTag) tag,
                                                  kCTFontTableOptionNoOptions));
    if (!data) {
        SkUniqueCFRef<CGFontRef> cgFont(CTFontCopyGraphicsFont(ctFont, nullptr));
        data.reset(CGFontCopyTableForTag(cgFont.get(), tag));
    }
    return data;
}

size_t SkTypeface_Mac::onGetTableData(SkFontTableTag tag, size_t offset,
                                      size_t length, void* dstData) const {
    SkUniqueCFRef<CFDataRef> srcData = copy_table_from_font(fFontRef.get(), tag);
    if (!srcData) {
        return 0;
    }

    size_t srcSize = CFDataGetLength(srcData.get());
    if (offset >= srcSize) {
        return 0;
    }
    if (length > srcSize - offset) {
        length = srcSize - offset;
    }
    if (dstData) {
        memcpy(dstData, CFDataGetBytePtr(srcData.get()) + offset, length);
    }
    return length;
}

SkScalerContext* SkTypeface_Mac::onCreateScalerContext(const SkScalerContextEffects& effects,
                                                       const SkDescriptor* desc) const {
    return new SkScalerContext_Mac(sk_ref_sp(const_cast<SkTypeface_Mac*>(this)), effects, desc);
}

void SkTypeface_Mac::onFilterRec(SkScalerContextRec* rec) const {
    if (rec->fFlags & SkScalerContext::kLCD_BGROrder_Flag ||
        rec->fFlags & SkScalerContext::kLCD_Vertical_Flag)
    {
        rec->fMaskFormat = SkMask::kA8_Format;
        // Render the glyphs as close as possible to what was requested.
        // The above turns off subpixel rendering, but the user requested it.
        // Normal hinting will cause the A8 masks to be generated from CoreGraphics subpixel masks.
        // See comments below for more details.
        rec->setHinting(SkFontHinting::kNormal);
    }

    unsigned flagsWeDontSupport = SkScalerContext::kForceAutohinting_Flag  |
                                  SkScalerContext::kLCD_BGROrder_Flag |
                                  SkScalerContext::kLCD_Vertical_Flag;

    rec->fFlags &= ~flagsWeDontSupport;

    const SmoothBehavior smoothBehavior = smooth_behavior();

    // Only two levels of hinting are supported.
    // kNo_Hinting means avoid CoreGraphics outline dilation (smoothing).
    // kNormal_Hinting means CoreGraphics outline dilation (smoothing) is allowed.
    if (rec->getHinting() != SkFontHinting::kNone) {
        rec->setHinting(SkFontHinting::kNormal);
    }
    // If smoothing has no effect, don't request it.
    if (smoothBehavior == SmoothBehavior::none) {
        rec->setHinting(SkFontHinting::kNone);
    }

    // FIXME: lcd smoothed un-hinted rasterization unsupported.
    // Tracked by http://code.google.com/p/skia/issues/detail?id=915 .
    // There is no current means to honor a request for unhinted lcd,
    // so arbitrarilly ignore the hinting request and honor lcd.

    // Hinting and smoothing should be orthogonal, but currently they are not.
    // CoreGraphics has no API to influence hinting. However, its lcd smoothed
    // output is drawn from auto-dilated outlines (the amount of which is
    // determined by AppleFontSmoothing). Its regular anti-aliased output is
    // drawn from un-dilated outlines.

    // The behavior of Skia is as follows:
    // [AA][no-hint]: generate AA using CoreGraphic's AA output.
    // [AA][yes-hint]: use CoreGraphic's LCD output and reduce it to a single
    // channel. This matches [LCD][yes-hint] in weight.
    // [LCD][no-hint]: curently unable to honor, and must pick which to respect.
    // Currenly side with LCD, effectively ignoring the hinting setting.
    // [LCD][yes-hint]: generate LCD using CoreGraphic's LCD output.
    if (rec->fMaskFormat == SkMask::kLCD16_Format) {
        if (smoothBehavior == SmoothBehavior::subpixel) {
            //CoreGraphics creates 555 masks for smoothed text anyway.
            rec->fMaskFormat = SkMask::kLCD16_Format;
            rec->setHinting(SkFontHinting::kNormal);
        } else {
            rec->fMaskFormat = SkMask::kA8_Format;
            if (smoothBehavior != SmoothBehavior::none) {
                rec->setHinting(SkFontHinting::kNormal);
            }
        }
    }

    // CoreText provides no information as to whether a glyph will be color or not.
    // Fonts may mix outlines and bitmaps, so information is needed on a glyph by glyph basis.
    // If a font contains an 'sbix' table, consider it to be a color font, and disable lcd.
    if (fHasColorGlyphs) {
        rec->fMaskFormat = SkMask::kARGB32_Format;
    }

    // Unhinted A8 masks (those not derived from LCD masks) must respect SK_GAMMA_APPLY_TO_A8.
    // All other masks can use regular gamma.
    if (SkMask::kA8_Format == rec->fMaskFormat && SkFontHinting::kNone == rec->getHinting()) {
#ifndef SK_GAMMA_APPLY_TO_A8
        // SRGBTODO: Is this correct? Do we want contrast boost?
        rec->ignorePreBlend();
#endif
    } else {
#ifndef SK_IGNORE_MAC_BLENDING_MATCH_FIX
        SkColor color = rec->getLuminanceColor();
        if (smoothBehavior == SmoothBehavior::some) {
            // CoreGraphics smoothed text without subpixel coverage blitting goes from a gamma of
            // 2.0 for black foreground to a gamma of 1.0 for white foreground. Emulate this
            // through the mask gamma by reducing the color values to 1/2.
            color = SkColorSetRGB(SkColorGetR(color) * 1/2,
                                  SkColorGetG(color) * 1/2,
                                  SkColorGetB(color) * 1/2);
        } else if (smoothBehavior == SmoothBehavior::subpixel) {
            // CoreGraphics smoothed text with subpixel coverage blitting goes from a gamma of
            // 2.0 for black foreground to a gamma of ~1.4? for white foreground. Emulate this
            // through the mask gamma by reducing the color values to 3/4.
            color = SkColorSetRGB(SkColorGetR(color) * 3/4,
                                  SkColorGetG(color) * 3/4,
                                  SkColorGetB(color) * 3/4);
        }
        rec->setLuminanceColor(color);
#endif
        // CoreGraphics dialates smoothed text to provide contrast.
        rec->setContrast(0);
    }
}

/** Takes ownership of the CFStringRef. */
static const char* get_str(CFStringRef ref, SkString* str) {
    if (nullptr == ref) {
        return nullptr;
    }
    CFStringToSkString(ref, str);
    CFRelease(ref);
    return str->c_str();
}

void SkTypeface_Mac::onGetFamilyName(SkString* familyName) const {
    get_str(CTFontCopyFamilyName(fFontRef.get()), familyName);
}

void SkTypeface_Mac::onGetFontDescriptor(SkFontDescriptor* desc,
                                         bool* isLocalStream) const {
    SkString tmpStr;

    desc->setFamilyName(get_str(CTFontCopyFamilyName(fFontRef.get()), &tmpStr));
    desc->setFullName(get_str(CTFontCopyFullName(fFontRef.get()), &tmpStr));
    desc->setPostscriptName(get_str(CTFontCopyPostScriptName(fFontRef.get()), &tmpStr));
    desc->setStyle(this->fontStyle());
    *isLocalStream = fIsFromStream;
}

void SkTypeface_Mac::onCharsToGlyphs(const SkUnichar uni[], int count, SkGlyphID glyphs[]) const {
    // Undocumented behavior of CTFontGetGlyphsForCharacters with non-bmp code points:
    // When a surrogate pair is detected, the glyph index used is the index of the high surrogate.
    // It is documented that if a mapping is unavailable, the glyph will be set to 0.

    SkAutoSTMalloc<1024, UniChar> charStorage;
    const UniChar* src; // UniChar is a UTF-16 16-bit code unit.
    int srcCount;
    const SkUnichar* utf32 = reinterpret_cast<const SkUnichar*>(uni);
    UniChar* utf16 = charStorage.reset(2 * count);
    src = utf16;
    for (int i = 0; i < count; ++i) {
        utf16 += SkUTF::ToUTF16(utf32[i], utf16);
    }
    srcCount = SkToInt(utf16 - src);

    // If there are any non-bmp code points, the provided 'glyphs' storage will be inadequate.
    SkAutoSTMalloc<1024, uint16_t> glyphStorage;
    uint16_t* macGlyphs = glyphs;
    if (srcCount > count) {
        macGlyphs = glyphStorage.reset(srcCount);
    }

    CTFontGetGlyphsForCharacters(fFontRef.get(), src, macGlyphs, srcCount);

    // If there were any non-bmp, then copy and compact.
    // If all are bmp, 'glyphs' already contains the compact glyphs.
    // If some are non-bmp, copy and compact into 'glyphs'.
    if (srcCount > count) {
        SkASSERT(glyphs != macGlyphs);
        int extra = 0;
        for (int i = 0; i < count; ++i) {
            glyphs[i] = macGlyphs[i + extra];
            if (SkUTF16_IsLeadingSurrogate(src[i + extra])) {
                ++extra;
            }
        }
    } else {
        SkASSERT(glyphs == macGlyphs);
    }
}

int SkTypeface_Mac::onCountGlyphs() const {
    return SkToInt(CTFontGetGlyphCount(fFontRef.get()));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static bool find_desc_str(CTFontDescriptorRef desc, CFStringRef name, SkString* value) {
    SkUniqueCFRef<CFStringRef> ref((CFStringRef)CTFontDescriptorCopyAttribute(desc, name));
    if (!ref) {
        return false;
    }
    CFStringToSkString(ref.get(), value);
    return true;
}

#include "include/core/SkFontMgr.h"

static inline int sqr(int value) {
    SkASSERT(SkAbs32(value) < 0x7FFF);  // check for overflow
    return value * value;
}

// We normalize each axis (weight, width, italic) to be base-900
static int compute_metric(const SkFontStyle& a, const SkFontStyle& b) {
    return sqr(a.weight() - b.weight()) +
           sqr((a.width() - b.width()) * 100) +
           sqr((a.slant() != b.slant()) * 900);
}

class SkFontStyleSet_Mac : public SkFontStyleSet {
public:
    SkFontStyleSet_Mac(CTFontDescriptorRef desc)
        : fArray(CTFontDescriptorCreateMatchingFontDescriptors(desc, nullptr))
        , fCount(0)
    {
        if (!fArray) {
            fArray.reset(CFArrayCreate(nullptr, nullptr, 0, nullptr));
        }
        fCount = SkToInt(CFArrayGetCount(fArray.get()));
    }

    int count() override {
        return fCount;
    }

    void getStyle(int index, SkFontStyle* style, SkString* name) override {
        SkASSERT((unsigned)index < (unsigned)fCount);
        CTFontDescriptorRef desc = (CTFontDescriptorRef)CFArrayGetValueAtIndex(fArray.get(), index);
        if (style) {
            *style = fontstyle_from_descriptor(desc, false);
        }
        if (name) {
            if (!find_desc_str(desc, kCTFontStyleNameAttribute, name)) {
                name->reset();
            }
        }
    }

    SkTypeface* createTypeface(int index) override {
        SkASSERT((unsigned)index < (unsigned)CFArrayGetCount(fArray.get()));
        CTFontDescriptorRef desc = (CTFontDescriptorRef)CFArrayGetValueAtIndex(fArray.get(), index);

        return create_from_desc(desc).release();
    }

    SkTypeface* matchStyle(const SkFontStyle& pattern) override {
        if (0 == fCount) {
            return nullptr;
        }
        return create_from_desc(findMatchingDesc(pattern)).release();
    }

private:
    SkUniqueCFRef<CFArrayRef> fArray;
    int fCount;

    CTFontDescriptorRef findMatchingDesc(const SkFontStyle& pattern) const {
        int bestMetric = SK_MaxS32;
        CTFontDescriptorRef bestDesc = nullptr;

        for (int i = 0; i < fCount; ++i) {
            CTFontDescriptorRef desc = (CTFontDescriptorRef)CFArrayGetValueAtIndex(fArray.get(), i);
            int metric = compute_metric(pattern, fontstyle_from_descriptor(desc, false));
            if (0 == metric) {
                return desc;
            }
            if (metric < bestMetric) {
                bestMetric = metric;
                bestDesc = desc;
            }
        }
        SkASSERT(bestDesc);
        return bestDesc;
    }
};

class SkFontMgr_Mac : public SkFontMgr {
    SkUniqueCFRef<CFArrayRef> fNames;
    int fCount;

    CFStringRef getFamilyNameAt(int index) const {
        SkASSERT((unsigned)index < (unsigned)fCount);
        return (CFStringRef)CFArrayGetValueAtIndex(fNames.get(), index);
    }

    static SkFontStyleSet* CreateSet(CFStringRef cfFamilyName) {
        SkUniqueCFRef<CFMutableDictionaryRef> cfAttr(
                 CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
                                           &kCFTypeDictionaryKeyCallBacks,
                                           &kCFTypeDictionaryValueCallBacks));

        CFDictionaryAddValue(cfAttr.get(), kCTFontFamilyNameAttribute, cfFamilyName);

        SkUniqueCFRef<CTFontDescriptorRef> desc(
                CTFontDescriptorCreateWithAttributes(cfAttr.get()));
        return new SkFontStyleSet_Mac(desc.get());
    }

    /** CTFontManagerCopyAvailableFontFamilyNames() is not always available, so we
     *  provide a wrapper here that will return an empty array if need be.
     */
    static SkUniqueCFRef<CFArrayRef> CopyAvailableFontFamilyNames() {
#ifdef SK_BUILD_FOR_IOS
        return SkUniqueCFRef<CFArrayRef>(CFArrayCreate(nullptr, nullptr, 0, nullptr));
#else
        return SkUniqueCFRef<CFArrayRef>(CTFontManagerCopyAvailableFontFamilyNames());
#endif
    }

public:
    SkFontMgr_Mac()
        : fNames(CopyAvailableFontFamilyNames())
        , fCount(fNames ? SkToInt(CFArrayGetCount(fNames.get())) : 0) {}

protected:
    int onCountFamilies() const override {
        return fCount;
    }

    void onGetFamilyName(int index, SkString* familyName) const override {
        if ((unsigned)index < (unsigned)fCount) {
            CFStringToSkString(this->getFamilyNameAt(index), familyName);
        } else {
            familyName->reset();
        }
    }

    SkFontStyleSet* onCreateStyleSet(int index) const override {
        if ((unsigned)index >= (unsigned)fCount) {
            return nullptr;
        }
        return CreateSet(this->getFamilyNameAt(index));
    }

    SkFontStyleSet* onMatchFamily(const char familyName[]) const override {
        if (!familyName) {
            return nullptr;
        }
        SkUniqueCFRef<CFStringRef> cfName = make_CFString(familyName);
        return CreateSet(cfName.get());
    }

    SkTypeface* onMatchFamilyStyle(const char familyName[],
                                   const SkFontStyle& style) const override {
        SkUniqueCFRef<CTFontDescriptorRef> desc = create_descriptor(familyName, style);
        return create_from_desc(desc.get()).release();
    }

    SkTypeface* onMatchFamilyStyleCharacter(const char familyName[],
                                            const SkFontStyle& style,
                                            const char* bcp47[], int bcp47Count,
                                            SkUnichar character) const override {
        SkUniqueCFRef<CTFontDescriptorRef> desc = create_descriptor(familyName, style);
        SkUniqueCFRef<CTFontRef> familyFont(CTFontCreateWithFontDescriptor(desc.get(), 0, nullptr));

        // kCFStringEncodingUTF32 is BE unless there is a BOM.
        // Since there is no machine endian option, explicitly state machine endian.
#ifdef SK_CPU_LENDIAN
        constexpr CFStringEncoding encoding = kCFStringEncodingUTF32LE;
#else
        constexpr CFStringEncoding encoding = kCFStringEncodingUTF32BE;
#endif
        SkUniqueCFRef<CFStringRef> string(CFStringCreateWithBytes(
                kCFAllocatorDefault, reinterpret_cast<const UInt8 *>(&character), sizeof(character),
                encoding, false));
        CFRange range = CFRangeMake(0, CFStringGetLength(string.get()));  // in UniChar units.
        SkUniqueCFRef<CTFontRef> fallbackFont(
                CTFontCreateForString(familyFont.get(), string.get(), range));
        return create_from_CTFontRef(std::move(fallbackFont), nullptr, nullptr).release();
    }

    SkTypeface* onMatchFaceStyle(const SkTypeface* familyMember,
                                 const SkFontStyle&) const override {
        return nullptr;
    }

    sk_sp<SkTypeface> onMakeFromData(sk_sp<SkData> data, int ttcIndex) const override {
        SkUniqueCFRef<CGDataProviderRef> pr(SkCreateDataProviderFromData(data));
        if (!pr) {
            return nullptr;
        }
        return create_from_dataProvider(std::move(pr), SkMemoryStream::Make(std::move(data)),
                                        ttcIndex);
    }

    sk_sp<SkTypeface> onMakeFromStreamIndex(std::unique_ptr<SkStreamAsset> stream,
                                            int ttcIndex) const override {
        SkUniqueCFRef<CGDataProviderRef> pr(SkCreateDataProviderFromStream(stream->duplicate()));
        if (!pr) {
            return nullptr;
        }
        return create_from_dataProvider(std::move(pr), std::move(stream), ttcIndex);
    }

    /** Creates a dictionary suitable for setting the axes on a CGFont. */
    static SkUniqueCFRef<CFDictionaryRef> copy_axes(CGFontRef cg, const SkFontArguments& args) {
        // The CGFont variation data is keyed by name, but lacks the tag.
        // The CTFont variation data is keyed by tag, and also has the name.
        // We would like to work with CTFont variations, but creating a CTFont font with
        // CTFont variation dictionary runs into bugs. So use the CTFont variation data
        // to match names to tags to create the appropriate CGFont.
        SkUniqueCFRef<CTFontRef> ct(CTFontCreateWithGraphicsFont(cg, 0, nullptr, nullptr));
        // CTFontCopyVariationAxes returns nullptr for CGFontCreateWithDataProvider fonts with
        // macOS 10.10 and iOS 9 or earlier. When this happens, there is no API to provide the tag.
        SkUniqueCFRef<CFArrayRef> ctAxes(CTFontCopyVariationAxes(ct.get()));
        if (!ctAxes) {
            return nullptr;
        }
        CFIndex axisCount = CFArrayGetCount(ctAxes.get());

        const SkFontArguments::VariationPosition position = args.getVariationDesignPosition();

        SkUniqueCFRef<CFMutableDictionaryRef> dict(
                CFDictionaryCreateMutable(kCFAllocatorDefault, axisCount,
                                          &kCFTypeDictionaryKeyCallBacks,
                                          &kCFTypeDictionaryValueCallBacks));

        for (int i = 0; i < axisCount; ++i) {
            CFTypeRef axisInfo = CFArrayGetValueAtIndex(ctAxes.get(), i);
            if (CFDictionaryGetTypeID() != CFGetTypeID(axisInfo)) {
                return nullptr;
            }
            CFDictionaryRef axisInfoDict = static_cast<CFDictionaryRef>(axisInfo);

            // The assumption is that values produced by kCTFontVariationAxisNameKey and
            // kCGFontVariationAxisName will always be equal.
            // If they are ever not, seach the project history for "get_tag_for_name".
            CFTypeRef axisName = CFDictionaryGetValue(axisInfoDict, kCTFontVariationAxisNameKey);
            if (!axisName || CFGetTypeID(axisName) != CFStringGetTypeID()) {
                return nullptr;
            }

            CFTypeRef tag = CFDictionaryGetValue(axisInfoDict, kCTFontVariationAxisIdentifierKey);
            if (!tag || CFGetTypeID(tag) != CFNumberGetTypeID()) {
                return nullptr;
            }
            CFNumberRef tagNumber = static_cast<CFNumberRef>(tag);
            int64_t tagLong;
            if (!CFNumberGetValue(tagNumber, kCFNumberSInt64Type, &tagLong)) {
                return nullptr;
            }

            // The variation axes can be set to any value, but cg will effectively pin them.
            // Pin them here to normalize.
            CFTypeRef min = CFDictionaryGetValue(axisInfoDict, kCTFontVariationAxisMinimumValueKey);
            CFTypeRef max = CFDictionaryGetValue(axisInfoDict, kCTFontVariationAxisMaximumValueKey);
            CFTypeRef def = CFDictionaryGetValue(axisInfoDict, kCTFontVariationAxisDefaultValueKey);
            if (!min || CFGetTypeID(min) != CFNumberGetTypeID() ||
                !max || CFGetTypeID(max) != CFNumberGetTypeID() ||
                !def || CFGetTypeID(def) != CFNumberGetTypeID())
            {
                return nullptr;
            }
            CFNumberRef minNumber = static_cast<CFNumberRef>(min);
            CFNumberRef maxNumber = static_cast<CFNumberRef>(max);
            CFNumberRef defNumber = static_cast<CFNumberRef>(def);
            double minDouble;
            double maxDouble;
            double defDouble;
            if (!CFNumberGetValue(minNumber, kCFNumberDoubleType, &minDouble) ||
                !CFNumberGetValue(maxNumber, kCFNumberDoubleType, &maxDouble) ||
                !CFNumberGetValue(defNumber, kCFNumberDoubleType, &defDouble))
            {
                return nullptr;
            }

            double value = defDouble;
            // The position may be over specified. If there are multiple values for a given axis,
            // use the last one since that's what css-fonts-4 requires.
            for (int j = position.coordinateCount; j --> 0;) {
                if (position.coordinates[j].axis == tagLong) {
                    value = SkTPin(SkScalarToDouble(position.coordinates[j].value),
                                   minDouble, maxDouble);
                    break;
                }
            }
            SkUniqueCFRef<CFNumberRef> valueNumber(
                CFNumberCreate(kCFAllocatorDefault, kCFNumberDoubleType, &value));
            CFDictionaryAddValue(dict.get(), axisName, valueNumber.get());
        }
        return std::move(dict);
    }
    sk_sp<SkTypeface> onMakeFromStreamArgs(std::unique_ptr<SkStreamAsset> s,
                                           const SkFontArguments& args) const override {
        if (args.getCollectionIndex() != 0) {
            return nullptr;
        }
        SkUniqueCFRef<CGDataProviderRef> provider(SkCreateDataProviderFromStream(s->duplicate()));
        if (!provider) {
            return nullptr;
        }
        SkUniqueCFRef<CGFontRef> cg(CGFontCreateWithDataProvider(provider.get()));
        if (!cg) {
            return nullptr;
        }

        SkUniqueCFRef<CFDictionaryRef> cgVariations = copy_axes(cg.get(), args);
        // The CGFontRef returned by CGFontCreateCopyWithVariations when the passed CGFontRef was
        // created from a data provider does not appear to have any ownership of the underlying
        // data. The original CGFontRef must be kept alive until the copy will no longer be used.
        SkUniqueCFRef<CGFontRef> cgVariant;
        if (cgVariations) {
            cgVariant.reset(CGFontCreateCopyWithVariations(cg.get(), cgVariations.get()));
        } else {
            cgVariant.reset(cg.release());
        }

        SkUniqueCFRef<CTFontRef> ct(
                CTFontCreateWithGraphicsFont(cgVariant.get(), 0, nullptr, nullptr));
        if (!ct) {
            return nullptr;
        }
        return create_from_CTFontRef(std::move(ct), std::move(cg), std::move(s));
    }

    /** Creates a dictionary suitable for setting the axes on a CGFont. */
    static SkUniqueCFRef<CFDictionaryRef> copy_axes(CGFontRef cg, SkFontData* fontData) {
        SkUniqueCFRef<CFArrayRef> cgAxes(CGFontCopyVariationAxes(cg));
        if (!cgAxes) {
            return nullptr;
        }

        CFIndex axisCount = CFArrayGetCount(cgAxes.get());
        if (0 == axisCount || axisCount != fontData->getAxisCount()) {
            return nullptr;
        }

        SkUniqueCFRef<CFMutableDictionaryRef> dict(
                CFDictionaryCreateMutable(kCFAllocatorDefault, axisCount,
                                          &kCFTypeDictionaryKeyCallBacks,
                                          &kCFTypeDictionaryValueCallBacks));

        for (int i = 0; i < fontData->getAxisCount(); ++i) {
            CFTypeRef axisInfo = CFArrayGetValueAtIndex(cgAxes.get(), i);
            if (CFDictionaryGetTypeID() != CFGetTypeID(axisInfo)) {
                return nullptr;
            }
            CFDictionaryRef axisInfoDict = static_cast<CFDictionaryRef>(axisInfo);

            CFTypeRef axisName = CFDictionaryGetValue(axisInfoDict, kCGFontVariationAxisName);
            if (!axisName || CFGetTypeID(axisName) != CFStringGetTypeID()) {
                return nullptr;
            }

            // The variation axes can be set to any value, but cg will effectively pin them.
            // Pin them here to normalize.
            CFTypeRef min = CFDictionaryGetValue(axisInfoDict, kCGFontVariationAxisMinValue);
            CFTypeRef max = CFDictionaryGetValue(axisInfoDict, kCGFontVariationAxisMaxValue);
            if (!min || CFGetTypeID(min) != CFNumberGetTypeID() ||
                !max || CFGetTypeID(max) != CFNumberGetTypeID())
            {
                return nullptr;
            }
            CFNumberRef minNumber = static_cast<CFNumberRef>(min);
            CFNumberRef maxNumber = static_cast<CFNumberRef>(max);
            double minDouble;
            double maxDouble;
            if (!CFNumberGetValue(minNumber, kCFNumberDoubleType, &minDouble) ||
                !CFNumberGetValue(maxNumber, kCFNumberDoubleType, &maxDouble))
            {
                return nullptr;
            }
            double value = SkTPin(SkFixedToDouble(fontData->getAxis()[i]), minDouble, maxDouble);
            SkUniqueCFRef<CFNumberRef> valueNumber(
                    CFNumberCreate(kCFAllocatorDefault, kCFNumberDoubleType, &value));
            CFDictionaryAddValue(dict.get(), axisName, valueNumber.get());
        }
        return std::move(dict);
    }
    sk_sp<SkTypeface> onMakeFromFontData(std::unique_ptr<SkFontData> fontData) const override {
        if (fontData->getIndex() != 0) {
            return nullptr;
        }
        SkUniqueCFRef<CGDataProviderRef> provider(
                SkCreateDataProviderFromStream(fontData->getStream()->duplicate()));
        if (!provider) {
            return nullptr;
        }
        SkUniqueCFRef<CGFontRef> cg(CGFontCreateWithDataProvider(provider.get()));
        if (!cg) {
            return nullptr;
        }

        SkUniqueCFRef<CFDictionaryRef> cgVariations = copy_axes(cg.get(), fontData.get());
        // The CGFontRef returned by CGFontCreateCopyWithVariations when the passed CGFontRef was
        // created from a data provider does not appear to have any ownership of the underlying
        // data. The original CGFontRef must be kept alive until the copy will no longer be used.
        SkUniqueCFRef<CGFontRef> cgVariant;
        if (cgVariations) {
            cgVariant.reset(CGFontCreateCopyWithVariations(cg.get(), cgVariations.get()));
        } else {
            cgVariant.reset(cg.release());
        }

        SkUniqueCFRef<CTFontRef> ct(
                CTFontCreateWithGraphicsFont(cgVariant.get(), 0, nullptr, nullptr));
        if (!ct) {
            return nullptr;
        }
        return create_from_CTFontRef(std::move(ct), std::move(cg), fontData->detachStream());
    }

    sk_sp<SkTypeface> onMakeFromFile(const char path[], int ttcIndex) const override {
        SkUniqueCFRef<CGDataProviderRef> pr(CGDataProviderCreateWithFilename(path));
        if (!pr) {
            return nullptr;
        }
        return create_from_dataProvider(std::move(pr), SkFILEStream::Make(path), ttcIndex);
    }

    sk_sp<SkTypeface> onLegacyMakeTypeface(const char familyName[], SkFontStyle style) const override {
        if (familyName) {
            familyName = map_css_names(familyName);
        }

        sk_sp<SkTypeface> face = create_from_name(familyName, style);
        if (face) {
            return face;
        }

        static SkTypeface* gDefaultFace;
        static SkOnce lookupDefault;
        static const char FONT_DEFAULT_NAME[] = "Lucida Sans";
        lookupDefault([]{
            gDefaultFace = create_from_name(FONT_DEFAULT_NAME, SkFontStyle()).release();
        });
        return sk_ref_sp(gDefaultFace);
    }
};

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkFontMgr> SkFontMgr::Factory() { return sk_make_sp<SkFontMgr_Mac>(); }

#endif//defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
