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

#include "include/core/SkColor.h"
#include "include/core/SkData.h"
#include "include/core/SkFontArguments.h"
#include "include/core/SkFontParameters.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/ports/SkTypeface_mac.h"
#include "include/private/SkFixed.h"
#include "include/private/SkMalloc.h"
#include "include/private/SkMutex.h"
#include "include/private/SkOnce.h"
#include "include/private/SkTDArray.h"
#include "include/private/SkTPin.h"
#include "include/private/SkTemplates.h"
#include "include/private/SkTo.h"
#include "src/core/SkAdvancedTypefaceMetrics.h"
#include "src/core/SkEndian.h"
#include "src/core/SkFontDescriptor.h"
#include "src/core/SkMask.h"
#include "src/core/SkScalerContext.h"
#include "src/core/SkTypefaceCache.h"
#include "src/core/SkUtils.h"
#include "src/ports/SkScalerContext_mac_ct.h"
#include "src/ports/SkTypeface_mac_ct.h"
#include "src/sfnt/SkOTTableTypes.h"
#include "src/sfnt/SkOTTable_OS_2.h"
#include "src/sfnt/SkOTTable_OS_2_V4.h"
#include "src/sfnt/SkOTUtils.h"
#include "src/sfnt/SkSFNTHeader.h"
#include "src/utils/SkUTF.h"
#include "src/utils/mac/SkCGBase.h"
#include "src/utils/mac/SkCGGeometry.h"
#include "src/utils/mac/SkCTFont.h"
#include "src/utils/mac/SkUniqueCFRef.h"

#include <dlfcn.h>
#include <limits.h>
#include <string.h>
#include <memory>

/** Assumes src and dst are not nullptr. */
void SkStringFromCFString(CFStringRef src, SkString* dst) {
    // Reserve enough room for the worst-case string,
    // plus 1 byte for the trailing null.
    CFIndex length = CFStringGetMaximumSizeForEncoding(CFStringGetLength(src),
                                                       kCFStringEncodingUTF8) + 1;
    dst->resize(length);
    CFStringGetCString(src, dst->writable_str(), length, kCFStringEncodingUTF8);
    // Resize to the actual UTF-8 length used, stripping the null character.
    dst->resize(strlen(dst->c_str()));
}

SkString SkCFTypeIDDescription(CFTypeID id) {
    SkUniqueCFRef<CFStringRef> typeDescription(CFCopyTypeIDDescription(id));
    SkString skTypeDescription;
    SkStringFromCFString(typeDescription.get(), &skTypeDescription);
    return skTypeDescription;
}

template<typename CF> CFTypeID SkCFGetTypeID();
#define SK_GETCFTYPEID(cf) \
template<> CFTypeID SkCFGetTypeID<cf##Ref>() { return cf##GetTypeID(); }
SK_GETCFTYPEID(CFBoolean);
SK_GETCFTYPEID(CFDictionary);
SK_GETCFTYPEID(CFNumber);

/* Checked dynamic downcast of CFTypeRef.
 *
 * @param cf the ref to downcast.
 * @param cfAsCF if cf can be cast to the type CF, receives the downcast ref.
 * @param name if non-nullptr the cast is expected to succeed and failures will be logged.
 * @return true if the cast succeeds, false otherwise.
 */
template <typename CF>
static bool SkCFDynamicCast(CFTypeRef cf, CF* cfAsCF, char const* name) {
    //SkDEBUGF("SkCFDynamicCast '%s' of type %s to type %s\n", name ? name : "<annon>",
    //         SkCFTypeIDDescription(  CFGetTypeID(cf)  ).c_str()
    //         SkCFTypeIDDescription(SkCFGetTypeID<CF>()).c_str());
    if (!cf) {
        if (name) {
            SkDEBUGF("%s not present\n", name);
        }
        return false;
    }
    if (CFGetTypeID(cf) != SkCFGetTypeID<CF>()) {
        if (name) {
            SkDEBUGF("%s is a %s but expected a %s\n", name,
                     SkCFTypeIDDescription(  CFGetTypeID(cf)  ).c_str(),
                     SkCFTypeIDDescription(SkCFGetTypeID<CF>()).c_str());
        }
        return false;
    }
    *cfAsCF = static_cast<CF>(cf);
    return true;
}

template<typename T> struct SkCFNumberTypeFor {};
#define SK_CFNUMBERTYPE_FOR(c, cf) \
template<> struct SkCFNumberTypeFor<c> : std::integral_constant<CFNumberType, cf> {};
SK_CFNUMBERTYPE_FOR(char     , kCFNumberCharType    );
SK_CFNUMBERTYPE_FOR(short    , kCFNumberShortType   );
SK_CFNUMBERTYPE_FOR(int      , kCFNumberIntType     );
SK_CFNUMBERTYPE_FOR(long     , kCFNumberLongType    );
SK_CFNUMBERTYPE_FOR(long long, kCFNumberLongLongType);
SK_CFNUMBERTYPE_FOR(float    , kCFNumberFloatType   );
SK_CFNUMBERTYPE_FOR(double   , kCFNumberDoubleType  );

template <typename T>
static bool SkCFNumberDynamicCast(CFTypeRef cf, T* number, CFNumberRef* cfNumber, char const* name){
    CFNumberRef cfAsCFNumber;
    if (!SkCFDynamicCast(cf, &cfAsCFNumber, name)) {
        return false;
    }
    if (!CFNumberGetValue(cfAsCFNumber, SkCFNumberTypeFor<T>::value, number)) {
        if (name) {
            SkDEBUGF("%s CFNumber not extractable\n", name);
        }
        return false;
    }
    if (cfNumber) {
        *cfNumber = cfAsCFNumber;
    }
    return true;
}

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
                                                 OpszVariation opsz)
{
    SkUniqueCFRef<CFMutableDictionaryRef> attr(
    CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
                              &kCFTypeDictionaryKeyCallBacks,
                              &kCFTypeDictionaryValueCallBacks));

    if (opsz.isSet) {
        add_opsz_attr(attr.get(), opsz.value);
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

CTFontRef SkTypeface_GetCTFontRef(const SkTypeface* face) {
    return face ? (CTFontRef)face->internal_private_getCTFontRef() : nullptr;
}

static bool find_by_CTFontRef(SkTypeface* cached, void* context) {
    CTFontRef self = (CTFontRef)context;
    CTFontRef other = (CTFontRef)cached->internal_private_getCTFontRef();

    return CFEqual(self, other);
}

/** Creates a typeface, searching the cache if isLocalStream is false. */
sk_sp<SkTypeface> SkTypeface_Mac::Make(SkUniqueCFRef<CTFontRef> font,
                                       OpszVariation opszVariation,
                                       std::unique_ptr<SkStreamAsset> providedData) {
    static SkMutex gTFCacheMutex;
    static SkTypefaceCache gTFCache;

    SkASSERT(font);
    const bool isFromStream(providedData);

    if (!isFromStream) {
        SkAutoMutexExclusive ama(gTFCacheMutex);
        sk_sp<SkTypeface> face = gTFCache.findByProcAndRef(find_by_CTFontRef, (void*)font.get());
        if (face) {
            return face;
        }
    }

    SkUniqueCFRef<CTFontDescriptorRef> desc(CTFontCopyFontDescriptor(font.get()));
    SkFontStyle style = SkCTFontDescriptorGetSkFontStyle(desc.get(), isFromStream);
    CTFontSymbolicTraits traits = CTFontGetSymbolicTraits(font.get());
    bool isFixedPitch = SkToBool(traits & kCTFontMonoSpaceTrait);

    sk_sp<SkTypeface> face(new SkTypeface_Mac(std::move(font), style,
                                              isFixedPitch, opszVariation,
                                              std::move(providedData)));
    if (!isFromStream) {
        SkAutoMutexExclusive ama(gTFCacheMutex);
        gTFCache.add(face);
    }
    return face;
}

/*  This function is visible on the outside. It first searches the cache, and if
 *  not found, returns a new entry (after adding it to the cache).
 */
sk_sp<SkTypeface> SkMakeTypefaceFromCTFont(CTFontRef font) {
    CFRetain(font);
    return SkTypeface_Mac::Make(SkUniqueCFRef<CTFontRef>(font),
                                OpszVariation(),
                                nullptr);
}

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

/** Convert the [0, 1000] CSS weight to [-1, 1] CTFontDescriptor weight (for system fonts).
 *
 *  The -1 to 1 weights reported by CTFontDescriptors have different mappings depending on if the
 *  CTFont is native or created from a CGDataProvider.
 */
CGFloat SkCTFontCTWeightForCSSWeight(int fontstyleWeight) {
    using Interpolator = LinearInterpolater<int, CGFloat, CGFloatIdentity>;

    // Note that Mac supports the old OS2 version A so 0 through 10 are as if multiplied by 100.
    // However, on this end we can't tell, so this is ignored.

    static Interpolator::Mapping nativeWeightMappings[11];
    static SkOnce once;
    once([&] {
        const CGFloat(&nsFontWeights)[11] = SkCTFontGetNSFontWeightMapping();
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

    static Interpolator::Mapping nativeWeightMappings[11];
    static Interpolator::Mapping dataProviderWeightMappings[11];
    static SkOnce once;
    once([&] {
        const CGFloat(&nsFontWeights)[11] = SkCTFontGetNSFontWeightMapping();
        const CGFloat(&userFontWeights)[11] = SkCTFontGetDataFontWeightMapping();
        for (int i = 0; i < 11; ++i) {
            nativeWeightMappings[i].src_val = nsFontWeights[i];
            nativeWeightMappings[i].dst_val = i * 100;

            dataProviderWeightMappings[i].src_val = userFontWeights[i];
            dataProviderWeightMappings[i].dst_val = i * 100;
        }
    });
    static constexpr Interpolator nativeInterpolator(
            nativeWeightMappings, SK_ARRAY_COUNT(nativeWeightMappings));
    static constexpr Interpolator dataProviderInterpolator(
            dataProviderWeightMappings, SK_ARRAY_COUNT(dataProviderWeightMappings));

    return fromDataProvider ? dataProviderInterpolator.map(cgWeight)
                            : nativeInterpolator.map(cgWeight);
}

/** Convert the [0, 10] CSS weight to [-1, 1] CTFontDescriptor width. */
CGFloat SkCTFontCTWidthForCSSWidth(int fontstyleWidth) {
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

SkFontStyle SkCTFontDescriptorGetSkFontStyle(CTFontDescriptorRef desc, bool fromDataProvider) {
    SkUniqueCFRef<CFTypeRef> traits(CTFontDescriptorCopyAttribute(desc, kCTFontTraitsAttribute));
    CFDictionaryRef fontTraitsDict;
    if (!SkCFDynamicCast(traits.get(), &fontTraitsDict, "Font traits")) {
        return SkFontStyle();
    }

    CGFloat weight, width, slant;
    if (!find_dict_CGFloat(fontTraitsDict, kCTFontWeightTrait, &weight)) {
        weight = 0;
    }
    if (!find_dict_CGFloat(fontTraitsDict, kCTFontWidthTrait, &width)) {
        width = 0;
    }
    if (!find_dict_CGFloat(fontTraitsDict, kCTFontSlantTrait, &slant)) {
        slant = 0;
    }

    return SkFontStyle(ct_weight_to_fontstyle(weight, fromDataProvider),
                       ct_width_to_fontstyle(width),
                       slant ? SkFontStyle::kItalic_Slant
                             : SkFontStyle::kUpright_Slant);
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

static constexpr uint16_t kPlaneSize = 1 << 13;

static void get_plane_glyph_map(const uint8_t* bits,
                                CTFontRef ctFont,
                                CFIndex glyphCount,
                                SkUnichar* glyphToUnicode,
                                uint8_t planeIndex) {
    SkUnichar planeOrigin = (SkUnichar)planeIndex << 16; // top half of codepoint.
    for (uint16_t i = 0; i < kPlaneSize; i++) {
        uint8_t mask = bits[i];
        if (!mask) {
            continue;
        }
        for (uint8_t j = 0; j < 8; j++) {
            if (0 == (mask & ((uint8_t)1 << j))) {
                continue;
            }
            uint16_t planeOffset = (i << 3) | j;
            SkUnichar codepoint = planeOrigin | (SkUnichar)planeOffset;
            uint16_t utf16[2] = {planeOffset, 0};
            size_t count = 1;
            if (planeOrigin != 0) {
                count = SkUTF::ToUTF16(codepoint, utf16);
            }
            CGGlyph glyphs[2] = {0, 0};
            if (CTFontGetGlyphsForCharacters(ctFont, utf16, glyphs, count)) {
                SkASSERT(glyphs[1] == 0);
                SkASSERT(glyphs[0] < glyphCount);
                // CTFontCopyCharacterSet and CTFontGetGlyphsForCharacters seem to add 'support'
                // for characters 0x9, 0xA, and 0xD mapping them to the glyph for character 0x20?
                // Prefer mappings to codepoints at or above 0x20.
                if (glyphToUnicode[glyphs[0]] < 0x20) {
                    glyphToUnicode[glyphs[0]] = codepoint;
                }
            }
        }
    }
}
// Construct Glyph to Unicode table.
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
    CFIndex dataLength = CFDataGetLength(bitmap.get());
    if (!dataLength) {
        return;
    }
    SkASSERT(dataLength >= kPlaneSize);
    const UInt8* bits = CFDataGetBytePtr(bitmap.get());

    get_plane_glyph_map(bits, ctFont, glyphCount, glyphToUnicode, 0);
    /*
    A CFData object that specifies the bitmap representation of the Unicode
    character points the for the new character set. The bitmap representation could
    contain all the Unicode character range starting from BMP to Plane 16. The
    first 8KiB (8192 bytes) of the data represent the BMP range. The BMP range 8KiB
    can be followed by zero to sixteen 8KiB bitmaps, each prepended with the plane
    index byte. For example, the bitmap representing the BMP and Plane 2 has the
    size of 16385 bytes (8KiB for BMP, 1 byte index, and a 8KiB bitmap for Plane
    2). The plane index byte, in this case, contains the integer value two.
    */

    if (dataLength <= kPlaneSize) {
        return;
    }
    int extraPlaneCount = (dataLength - kPlaneSize) / (1 + kPlaneSize);
    SkASSERT(dataLength == kPlaneSize + extraPlaneCount * (1 + kPlaneSize));
    while (extraPlaneCount-- > 0) {
        bits += kPlaneSize;
        uint8_t planeIndex = *bits++;
        SkASSERT(planeIndex >= 1);
        SkASSERT(planeIndex <= 16);
        get_plane_glyph_map(bits, ctFont, glyphCount, glyphToUnicode, planeIndex);
    }
}

void SkTypeface_Mac::getGlyphToUnicodeMap(SkUnichar* dstArray) const {
    SkUniqueCFRef<CTFontRef> ctFont =
            SkCTFontCreateExactCopy(fFontRef.get(), CTFontGetUnitsPerEm(fFontRef.get()),
                                    fOpszVariation);
    CFIndex glyphCount = CTFontGetGlyphCount(ctFont.get());
    populate_glyph_to_unicode(ctFont.get(), glyphCount, dstArray);
}

std::unique_ptr<SkAdvancedTypefaceMetrics> SkTypeface_Mac::onGetAdvancedMetrics() const {

    SkUniqueCFRef<CTFontRef> ctFont =
            SkCTFontCreateExactCopy(fFontRef.get(), CTFontGetUnitsPerEm(fFontRef.get()),
                                    fOpszVariation);

    std::unique_ptr<SkAdvancedTypefaceMetrics> info(new SkAdvancedTypefaceMetrics);

    {
        SkUniqueCFRef<CFStringRef> fontName(CTFontCopyPostScriptName(ctFont.get()));
        if (fontName.get()) {
            SkStringFromCFString(fontName.get(), &info->fPostScriptName);
            info->fFontName = info->fPostScriptName;
        }
    }

    SkUniqueCFRef<CFArrayRef> ctAxes(CTFontCopyVariationAxes(ctFont.get()));
    if (ctAxes && CFArrayGetCount(ctAxes.get()) > 0) {
        info->fFlags |= SkAdvancedTypefaceMetrics::kVariable_FontFlag;
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
    if (!this->getTableSize(SkSetFourByteTag('g','l','y','f')) ||
        !this->getTableSize(SkSetFourByteTag('l','o','c','a')))
    {
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
    r.setLTRB(SkScalarFromCGFloat(SkCGRectGetMinX(bbox)),   // Left
              SkScalarFromCGFloat(SkCGRectGetMaxY(bbox)),   // Top
              SkScalarFromCGFloat(SkCGRectGetMaxX(bbox)),   // Right
              SkScalarFromCGFloat(SkCGRectGetMinY(bbox)));  // Bottom

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
    fStream = std::make_unique<SkMemoryStream>(totalSize);
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

int SkTypeface_Mac::onGetVariationDesignPosition(
        SkFontArguments::VariationPosition::Coordinate coordinates[], int coordinateCount) const
{
    SkUniqueCFRef<CFArrayRef> ctAxes(CTFontCopyVariationAxes(fFontRef.get()));
    if (!ctAxes) {
        return -1;
    }
    CFIndex axisCount = CFArrayGetCount(ctAxes.get());
    if (!coordinates || coordinateCount < axisCount) {
        return axisCount;
    }

    // On 10.12 and later, this only returns non-default variations.
    SkUniqueCFRef<CFDictionaryRef> ctVariation(CTFontCopyVariation(fFontRef.get()));
    if (!ctVariation) {
        return -1;
    }

    for (int i = 0; i < axisCount; ++i) {
        CFDictionaryRef axisInfoDict;
        if (!SkCFDynamicCast(CFArrayGetValueAtIndex(ctAxes.get(), i), &axisInfoDict, "Axis")) {
            return -1;
        }

        int64_t tagLong;
        CFNumberRef tagNumber;
        CFTypeRef tag = CFDictionaryGetValue(axisInfoDict, kCTFontVariationAxisIdentifierKey);
        if (!SkCFNumberDynamicCast(tag, &tagLong, &tagNumber, "Axis tag")) {
            return -1;
        }
        coordinates[i].axis = tagLong;

        CGFloat valueCGFloat;
        CFTypeRef value = CFDictionaryGetValue(ctVariation.get(), tagNumber);
        if (value) {
            if (!SkCFNumberDynamicCast(value, &valueCGFloat, nullptr, "Variation value")) {
                return -1;
            }
        } else {
            CFTypeRef def = CFDictionaryGetValue(axisInfoDict, kCTFontVariationAxisDefaultValueKey);
            if (!SkCFNumberDynamicCast(def, &valueCGFloat, nullptr, "Axis default value")) {
                return -1;
            }
        }
        coordinates[i].value = SkScalarFromCGFloat(valueCGFloat);
    }
    return axisCount;
}

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
            SkStringFromCFString(cfLanguage.get(), &skLanguage);
        } else {
            skLanguage = "und"; //undetermined
        }
        if (cfFamilyName) {
            SkStringFromCFString(cfFamilyName.get(), &skFamilyName);
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

sk_sp<SkData> SkTypeface_Mac::onCopyTableData(SkFontTableTag tag) const {
    SkUniqueCFRef<CFDataRef> srcData = copy_table_from_font(fFontRef.get(), tag);
    if (!srcData) {
        return nullptr;
    }
    const UInt8* data = CFDataGetBytePtr(srcData.get());
    CFIndex length = CFDataGetLength(srcData.get());
    return SkData::MakeWithProc(data, length,
                                [](const void*, void* ctx) {
                                    CFRelease((CFDataRef)ctx);
                                }, (void*)srcData.release());
}

std::unique_ptr<SkScalerContext> SkTypeface_Mac::onCreateScalerContext(
    const SkScalerContextEffects& effects, const SkDescriptor* desc) const
{
    return std::make_unique<SkScalerContext_Mac>(
            sk_ref_sp(const_cast<SkTypeface_Mac*>(this)), effects, desc);
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

    const SkCTFontSmoothBehavior smoothBehavior = SkCTFontGetSmoothBehavior();

    // Only two levels of hinting are supported.
    // kNo_Hinting means avoid CoreGraphics outline dilation (smoothing).
    // kNormal_Hinting means CoreGraphics outline dilation (smoothing) is allowed.
    if (rec->getHinting() != SkFontHinting::kNone) {
        rec->setHinting(SkFontHinting::kNormal);
    }
    // If smoothing has no effect, don't request it.
    if (smoothBehavior == SkCTFontSmoothBehavior::none) {
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
    // [LCD][no-hint]: currently unable to honor, and must pick which to respect.
    // Currently side with LCD, effectively ignoring the hinting setting.
    // [LCD][yes-hint]: generate LCD using CoreGraphic's LCD output.
    if (rec->fMaskFormat == SkMask::kLCD16_Format) {
        if (smoothBehavior == SkCTFontSmoothBehavior::subpixel) {
            //CoreGraphics creates 555 masks for smoothed text anyway.
            rec->fMaskFormat = SkMask::kLCD16_Format;
            rec->setHinting(SkFontHinting::kNormal);
        } else {
            rec->fMaskFormat = SkMask::kA8_Format;
            if (smoothBehavior != SkCTFontSmoothBehavior::none) {
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
        SkColor color = rec->getLuminanceColor();
        if (smoothBehavior == SkCTFontSmoothBehavior::some) {
            // CoreGraphics smoothed text without subpixel coverage blitting goes from a gamma of
            // 2.0 for black foreground to a gamma of 1.0 for white foreground. Emulate this
            // through the mask gamma by reducing the color values to 1/2.
            color = SkColorSetRGB(SkColorGetR(color) * 1/2,
                                  SkColorGetG(color) * 1/2,
                                  SkColorGetB(color) * 1/2);
        } else if (smoothBehavior == SkCTFontSmoothBehavior::subpixel) {
            // CoreGraphics smoothed text with subpixel coverage blitting goes from a gamma of
            // 2.0 for black foreground to a gamma of ~1.4? for white foreground. Emulate this
            // through the mask gamma by reducing the color values to 3/4.
            color = SkColorSetRGB(SkColorGetR(color) * 3/4,
                                  SkColorGetG(color) * 3/4,
                                  SkColorGetB(color) * 3/4);
        }
        rec->setLuminanceColor(color);

        // CoreGraphics dialates smoothed text to provide contrast.
        rec->setContrast(0);
    }
}

/** Takes ownership of the CFStringRef. */
static const char* get_str(CFStringRef ref, SkString* str) {
    if (nullptr == ref) {
        return nullptr;
    }
    SkStringFromCFString(ref, str);
    CFRelease(ref);
    return str->c_str();
}

void SkTypeface_Mac::onGetFamilyName(SkString* familyName) const {
    get_str(CTFontCopyFamilyName(fFontRef.get()), familyName);
}

bool SkTypeface_Mac::onGetPostScriptName(SkString* skPostScriptName) const {
    SkUniqueCFRef<CFStringRef> ctPostScriptName(CTFontCopyPostScriptName(fFontRef.get()));
    if (!ctPostScriptName) {
        return false;
    }
    if (skPostScriptName) {
        SkStringFromCFString(ctPostScriptName.get(), skPostScriptName);
    }
    return true;
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

/** Creates a dictionary suitable for setting the axes on a CTFont. */
CTFontVariation SkCTVariationFromSkFontArguments(CTFontRef ct, const SkFontArguments& args) {
    OpszVariation opsz;
    constexpr const SkFourByteTag opszTag = SkSetFourByteTag('o','p','s','z');

    SkUniqueCFRef<CFArrayRef> ctAxes(CTFontCopyVariationAxes(ct));
    if (!ctAxes) {
        return CTFontVariation();
    }
    CFIndex axisCount = CFArrayGetCount(ctAxes.get());

    // On 10.12 and later, this only returns non-default variations.
    SkUniqueCFRef<CFDictionaryRef> oldCtVariation(CTFontCopyVariation(ct));

    const SkFontArguments::VariationPosition position = args.getVariationDesignPosition();

    SkUniqueCFRef<CFMutableDictionaryRef> newCtVariation(
            CFDictionaryCreateMutable(kCFAllocatorDefault, axisCount,
                                      &kCFTypeDictionaryKeyCallBacks,
                                      &kCFTypeDictionaryValueCallBacks));
    SkUniqueCFRef<CFMutableDictionaryRef> wrongOpszVariation;

    for (int i = 0; i < axisCount; ++i) {
        CFDictionaryRef axisInfoDict;
        if (!SkCFDynamicCast(CFArrayGetValueAtIndex(ctAxes.get(), i), &axisInfoDict, "Axis")) {
            return CTFontVariation();
        }

        int64_t tagLong;
        CFNumberRef tagNumber;
        CFTypeRef tag = CFDictionaryGetValue(axisInfoDict, kCTFontVariationAxisIdentifierKey);
        if (!SkCFNumberDynamicCast(tag, &tagLong, &tagNumber, "Axis tag")) {
            return CTFontVariation();
        }

        // The variation axes can be set to any value, but cg will effectively pin them.
        // Pin them here to normalize.
        double minDouble;
        double maxDouble;
        double defDouble;
        CFTypeRef min = CFDictionaryGetValue(axisInfoDict, kCTFontVariationAxisMinimumValueKey);
        CFTypeRef max = CFDictionaryGetValue(axisInfoDict, kCTFontVariationAxisMaximumValueKey);
        CFTypeRef def = CFDictionaryGetValue(axisInfoDict, kCTFontVariationAxisDefaultValueKey);
        if (!SkCFNumberDynamicCast(min, &minDouble, nullptr, "Axis min") ||
            !SkCFNumberDynamicCast(max, &maxDouble, nullptr, "Axis max") ||
            !SkCFNumberDynamicCast(def, &defDouble, nullptr, "Axis def"))
        {
            return CTFontVariation();
        }

        // Start with the default value.
        double value = defDouble;

        // Then the current value.
        bool haveCurrentDouble = false;
        double currentDouble = 0;
        if (oldCtVariation) {
            CFTypeRef currentNumber = CFDictionaryGetValue(oldCtVariation.get(), tagNumber);
            if (currentNumber) {
                if (!SkCFNumberDynamicCast(currentNumber, &value, nullptr, "Variation value")) {
                    return CTFontVariation();
                }
                currentDouble = value;
                haveCurrentDouble = true;
            }
        }

        // Then the requested value.
        // The position may be over specified. If there are multiple values for a given axis,
        // use the last one since that's what css-fonts-4 requires.
        for (int j = position.coordinateCount; j --> 0;) {
            if (position.coordinates[j].axis == tagLong) {
                value = SkTPin<double>(position.coordinates[j].value, minDouble, maxDouble);
                if (tagLong == opszTag) {
                    opsz.isSet = true;
                }
                break;
            }
        }
        if (tagLong == opszTag) {
            opsz.value = value;
            if (haveCurrentDouble && value == currentDouble) {
                // Calculate a value strictly in range but different from currentValue.
                double wrongOpszDouble = ((maxDouble - minDouble) / 2.0) + minDouble;
                if (wrongOpszDouble == currentDouble) {
                    wrongOpszDouble = ((maxDouble - minDouble) / 4.0) + minDouble;
                }
                wrongOpszVariation.reset(
                    CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
                                              &kCFTypeDictionaryKeyCallBacks,
                                              &kCFTypeDictionaryValueCallBacks));
                SkUniqueCFRef<CFNumberRef> wrongOpszNumber(
                    CFNumberCreate(kCFAllocatorDefault, kCFNumberDoubleType, &wrongOpszDouble));
                CFDictionarySetValue(wrongOpszVariation.get(), tagNumber, wrongOpszNumber.get());
            }
        }
        SkUniqueCFRef<CFNumberRef> valueNumber(
            CFNumberCreate(kCFAllocatorDefault, kCFNumberDoubleType, &value));
        CFDictionaryAddValue(newCtVariation.get(), tagNumber, valueNumber.get());
    }
    return { SkUniqueCFRef<CFDictionaryRef>(std::move(newCtVariation)),
             SkUniqueCFRef<CFDictionaryRef>(std::move(wrongOpszVariation)),
             opsz };
}

sk_sp<SkTypeface> SkTypeface_Mac::onMakeClone(const SkFontArguments& args) const {
    CTFontVariation ctVariation = SkCTVariationFromSkFontArguments(fFontRef.get(), args);

    SkUniqueCFRef<CTFontRef> ctVariant;
    if (ctVariation.variation) {
        SkUniqueCFRef<CFMutableDictionaryRef> attributes(
                CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
                                          &kCFTypeDictionaryKeyCallBacks,
                                          &kCFTypeDictionaryValueCallBacks));

        CTFontRef ctFont = fFontRef.get();
        SkUniqueCFRef<CTFontRef> wrongOpszFont;
        if (ctVariation.wrongOpszVariation) {
            // On macOS 11 cloning a system font with an opsz axis and not changing the
            // value of the opsz axis (either by setting it to the same value or not
            // specifying it at all) when setting a variation causes the variation to
            // be set but the cloned font will still compare CFEqual to the original
            // font. Work around this by setting the opsz to something which isn't the
            // desired value before setting the entire desired variation.
            //
            // A similar issue occurs with fonts from data on macOS 10.15 and the same
            // work around seems to apply. This is less noticeable though since CFEqual
            // isn't used on these fonts.
            CFDictionarySetValue(attributes.get(),
                                 kCTFontVariationAttribute, ctVariation.wrongOpszVariation.get());
            SkUniqueCFRef<CTFontDescriptorRef> varDesc(
                CTFontDescriptorCreateWithAttributes(attributes.get()));
            wrongOpszFont.reset(CTFontCreateCopyWithAttributes(ctFont, 0, nullptr, varDesc.get()));
            ctFont = wrongOpszFont.get();
        }

        CFDictionarySetValue(attributes.get(),
                             kCTFontVariationAttribute, ctVariation.variation.get());
        SkUniqueCFRef<CTFontDescriptorRef> varDesc(
                CTFontDescriptorCreateWithAttributes(attributes.get()));
        ctVariant.reset(CTFontCreateCopyWithAttributes(ctFont, 0, nullptr, varDesc.get()));
    } else {
        ctVariant.reset((CTFontRef)CFRetain(fFontRef.get()));
    }
    if (!ctVariant) {
        return nullptr;
    }

    return SkTypeface_Mac::Make(std::move(ctVariant), ctVariation.opsz,
                                fStream ? fStream->duplicate() : nullptr);
}

int SkTypeface_Mac::onGetVariationDesignParameters(SkFontParameters::Variation::Axis parameters[],
                                                   int parameterCount) const
{
    SkUniqueCFRef<CFArrayRef> ctAxes(CTFontCopyVariationAxes(fFontRef.get()));
    if (!ctAxes) {
        return -1;
    }
    CFIndex axisCount = CFArrayGetCount(ctAxes.get());

    if (!parameters || parameterCount < axisCount) {
        return axisCount;
    }

    // Added in 10.13
    static CFStringRef* kCTFontVariationAxisHiddenKeyPtr =
            static_cast<CFStringRef*>(dlsym(RTLD_DEFAULT, "kCTFontVariationAxisHiddenKey"));

    for (int i = 0; i < axisCount; ++i) {
        CFDictionaryRef axisInfoDict;
        if (!SkCFDynamicCast(CFArrayGetValueAtIndex(ctAxes.get(), i), &axisInfoDict, "Axis")) {
            return -1;
        }

        int64_t tagLong;
        CFTypeRef tag = CFDictionaryGetValue(axisInfoDict, kCTFontVariationAxisIdentifierKey);
        if (!SkCFNumberDynamicCast(tag, &tagLong, nullptr, "Axis tag")) {
            return -1;
        }

        double minDouble;
        double maxDouble;
        double defDouble;
        CFTypeRef min = CFDictionaryGetValue(axisInfoDict, kCTFontVariationAxisMinimumValueKey);
        CFTypeRef max = CFDictionaryGetValue(axisInfoDict, kCTFontVariationAxisMaximumValueKey);
        CFTypeRef def = CFDictionaryGetValue(axisInfoDict, kCTFontVariationAxisDefaultValueKey);
        if (!SkCFNumberDynamicCast(min, &minDouble, nullptr, "Axis min") ||
            !SkCFNumberDynamicCast(max, &maxDouble, nullptr, "Axis max") ||
            !SkCFNumberDynamicCast(def, &defDouble, nullptr, "Axis def"))
        {
            return -1;
        }

        SkFontParameters::Variation::Axis& skAxis = parameters[i];
        skAxis.tag = tagLong;
        skAxis.min = minDouble;
        skAxis.max = maxDouble;
        skAxis.def = defDouble;
        skAxis.setHidden(false);
        if (kCTFontVariationAxisHiddenKeyPtr) {
            CFTypeRef hidden = CFDictionaryGetValue(axisInfoDict,*kCTFontVariationAxisHiddenKeyPtr);
            if (hidden) {
                // At least macOS 11 Big Sur Beta 4 uses CFNumberRef instead of CFBooleanRef.
                // https://crbug.com/1113444
                CFBooleanRef hiddenBoolean;
                int hiddenInt;
                if (SkCFDynamicCast(hidden, &hiddenBoolean, nullptr)) {
                    skAxis.setHidden(CFBooleanGetValue(hiddenBoolean));
                } else if (SkCFNumberDynamicCast(hidden, &hiddenInt, nullptr, "Axis hidden")) {
                    skAxis.setHidden(hiddenInt);
                } else {
                    return -1;
                }
            }
        }
    }
    return axisCount;
}

#endif
