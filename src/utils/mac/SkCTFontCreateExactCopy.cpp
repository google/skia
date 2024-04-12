/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)

#include "src/utils/mac/SkCTFontCreateExactCopy.h"

#include "src/ports/SkTypeface_mac_ct.h"
#include "src/utils/mac/SkUniqueCFRef.h"

// In macOS 10.12 and later any variation on the CGFont which has default axis value will be
// dropped when creating the CTFont. Unfortunately, in macOS 10.15 the priority of setting
// the optical size (and opsz variation) is
// 1. the value of kCTFontOpticalSizeAttribute in the CTFontDescriptor (undocumented)
// 2. the opsz axis default value if kCTFontOpticalSizeAttribute is 'none' (undocumented)
// 3. the opsz variation on the nascent CTFont from the CGFont (was dropped if default)
// 4. the opsz variation in kCTFontVariationAttribute in CTFontDescriptor (crashes 10.10)
// 5. the size requested (can fudge in SkTypeface but not SkScalerContext)
// The first one which is found will be used to set the opsz variation (after clamping).
static void add_opsz_attr(CFMutableDictionaryRef attr, double opsz) {
    SkUniqueCFRef<CFNumberRef> opszValueNumber(
        CFNumberCreate(kCFAllocatorDefault, kCFNumberDoubleType, &opsz));
    // Avoid using kCTFontOpticalSizeAttribute directly
    CFStringRef SkCTFontOpticalSizeAttribute = CFSTR("NSCTFontOpticalSizeAttribute");
    CFDictionarySetValue(attr, SkCTFontOpticalSizeAttribute, opszValueNumber.get());
}

// This turns off application of the 'trak' table to advances, but also all other tracking.
static void add_notrak_attr(CFMutableDictionaryRef attr) {
    int zero = 0;
    SkUniqueCFRef<CFNumberRef> unscaledTrackingNumber(
        CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &zero));
    CFStringRef SkCTFontUnscaledTrackingAttribute = CFSTR("NSCTFontUnscaledTrackingAttribute");
    CFDictionarySetValue(attr, SkCTFontUnscaledTrackingAttribute, unscaledTrackingNumber.get());
}

SkUniqueCFRef<CTFontRef> SkCTFontCreateExactCopy(CTFontRef baseFont, CGFloat textSize,
                                                 OpszVariation opszVariation)
{
    SkUniqueCFRef<CFMutableDictionaryRef> attr(
    CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
                              &kCFTypeDictionaryKeyCallBacks,
                              &kCFTypeDictionaryValueCallBacks));

    if (opszVariation.isSet) {
        add_opsz_attr(attr.get(), opszVariation.value);
    } else {
        // On (at least) 10.10 though 10.14 the default system font was SFNSText/SFNSDisplay.
        // The CTFont is backed by both; optical size < 20 means SFNSText else SFNSDisplay.
        // On at least 10.11 the glyph ids in these fonts became non-interchangable.
        // To keep glyph ids stable over size changes, preserve the optical size.
        // In 10.15 this was replaced with use of variable fonts with an opsz axis.
        // A CTFont backed by multiple fonts picked by opsz where the multiple backing fonts are
        // variable fonts with opsz axis and non-interchangeable glyph ids would break the
        // opsz.isSet branch above, but hopefully that never happens.
        // See https://crbug.com/524646 .
        CFStringRef SkCTFontOpticalSizeAttribute = CFSTR("NSCTFontOpticalSizeAttribute");
        SkUniqueCFRef<CFTypeRef> opsz(CTFontCopyAttribute(baseFont, SkCTFontOpticalSizeAttribute));
        double opsz_val;
        if (!opsz ||
            CFGetTypeID(opsz.get()) != CFNumberGetTypeID() ||
            !CFNumberGetValue(static_cast<CFNumberRef>(opsz.get()),kCFNumberDoubleType,&opsz_val) ||
            opsz_val <= 0)
        {
            opsz_val = CTFontGetSize(baseFont);
        }
        add_opsz_attr(attr.get(), opsz_val);
    }
    add_notrak_attr(attr.get());

    SkUniqueCFRef<CTFontDescriptorRef> desc(CTFontDescriptorCreateWithAttributes(attr.get()));

    return SkUniqueCFRef<CTFontRef>(
            CTFontCreateCopyWithAttributes(baseFont, textSize, nullptr, desc.get()));
}

#endif

