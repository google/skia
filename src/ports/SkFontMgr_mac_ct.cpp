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
#include <dlfcn.h>
#endif

#include "include/core/SkData.h"
#include "include/core/SkFontArguments.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/ports/SkFontMgr_mac_ct.h"
#include "include/private/base/SkFixed.h"
#include "include/private/base/SkOnce.h"
#include "include/private/base/SkTPin.h"
#include "include/private/base/SkTemplates.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkUTF.h"
#include "src/core/SkFontDescriptor.h"
#include "src/ports/SkTypeface_mac_ct.h"

#include <string.h>
#include <memory>

using namespace skia_private;

#if (defined(SK_BUILD_FOR_IOS) && defined(__IPHONE_14_0) &&  \
      __IPHONE_OS_VERSION_MIN_REQUIRED >= __IPHONE_14_0) ||  \
    (defined(SK_BUILD_FOR_MAC) && defined(__MAC_11_0) &&     \
      __MAC_OS_VERSION_MIN_REQUIRED >= __MAC_11_0)

static uint32_t SkGetCoreTextVersion() {
    // If compiling for iOS 14.0+ or macOS 11.0+, the CoreText version number
    // must be derived from the OS version number.
    static const uint32_t kCoreTextVersionNEWER = 0x000D0000;
    return kCoreTextVersionNEWER;
}

#else

static uint32_t SkGetCoreTextVersion() {
    // Check for CoreText availability before calling CTGetCoreTextVersion().
    static const bool kCoreTextIsAvailable = (&CTGetCoreTextVersion != nullptr);
    if (kCoreTextIsAvailable) {
        return CTGetCoreTextVersion();
    }

    // Default to a value that's smaller than any known CoreText version.
    static const uint32_t kCoreTextVersionUNKNOWN = 0;
    return kCoreTextVersionUNKNOWN;
}

#endif

static SkUniqueCFRef<CFStringRef> make_CFString(const char s[]) {
    return SkUniqueCFRef<CFStringRef>(CFStringCreateWithCString(nullptr, s, kCFStringEncodingUTF8));
}

/** Creates a typeface from a descriptor, searching the cache. */
static sk_sp<SkTypeface> create_from_desc(CTFontDescriptorRef desc) {
    SkUniqueCFRef<CTFontRef> ctFont(CTFontCreateWithFontDescriptor(desc, 0, nullptr));
    if (!ctFont) {
        return nullptr;
    }

    return SkTypeface_Mac::Make(std::move(ctFont), OpszVariation(), nullptr);
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

    // TODO(crbug.com/1018581) Some CoreText versions have errant behavior when
    // certain traits set.  Temporary workaround to omit specifying trait for those
    // versions.
    // Long term solution will involve serializing typefaces instead of relying upon
    // this to match between processes.
    //
    // Compare CoreText.h in an up to date SDK for where these values come from.
    static const uint32_t kSkiaLocalCTVersionNumber10_14 = 0x000B0000;
    static const uint32_t kSkiaLocalCTVersionNumber10_15 = 0x000C0000;

    // CTFontTraits (symbolic)
    // macOS 14 and iOS 12 seem to behave badly when kCTFontSymbolicTrait is set.
    // macOS 15 yields LastResort font instead of a good default font when
    // kCTFontSymbolicTrait is set.
    if (SkGetCoreTextVersion() < kSkiaLocalCTVersionNumber10_14) {
        CTFontSymbolicTraits ctFontTraits = 0;
        if (style.weight() >= SkFontStyle::kBold_Weight) {
            ctFontTraits |= kCTFontBoldTrait;
        }
        if (style.slant() != SkFontStyle::kUpright_Slant) {
            ctFontTraits |= kCTFontItalicTrait;
        }
        SkUniqueCFRef<CFNumberRef> cfFontTraits(
                CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &ctFontTraits));
        if (cfFontTraits) {
            CFDictionaryAddValue(cfTraits.get(), kCTFontSymbolicTrait, cfFontTraits.get());
        }
    }

    // CTFontTraits (weight)
    CGFloat ctWeight = SkCTFontCTWeightForCSSWeight(style.weight());
    SkUniqueCFRef<CFNumberRef> cfFontWeight(
            CFNumberCreate(kCFAllocatorDefault, kCFNumberCGFloatType, &ctWeight));
    if (cfFontWeight) {
        CFDictionaryAddValue(cfTraits.get(), kCTFontWeightTrait, cfFontWeight.get());
    }
    // CTFontTraits (width)
    CGFloat ctWidth = SkCTFontCTWidthForCSSWidth(style.width());
    SkUniqueCFRef<CFNumberRef> cfFontWidth(
            CFNumberCreate(kCFAllocatorDefault, kCFNumberCGFloatType, &ctWidth));
    if (cfFontWidth) {
        CFDictionaryAddValue(cfTraits.get(), kCTFontWidthTrait, cfFontWidth.get());
    }
    // CTFontTraits (slant)
    // macOS 15 behaves badly when kCTFontSlantTrait is set.
    if (SkGetCoreTextVersion() != kSkiaLocalCTVersionNumber10_15) {
        CGFloat ctSlant = style.slant() == SkFontStyle::kUpright_Slant ? 0 : 1;
        SkUniqueCFRef<CFNumberRef> cfFontSlant(
                CFNumberCreate(kCFAllocatorDefault, kCFNumberCGFloatType, &ctSlant));
        if (cfFontSlant) {
            CFDictionaryAddValue(cfTraits.get(), kCTFontSlantTrait, cfFontSlant.get());
        }
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

// Same as the above function except style is included so we can
// compare whether the created font conforms to the style. If not, we need
// to recreate the font with symbolic traits. This is needed due to MacOS 10.11
// font creation problem https://bugs.chromium.org/p/skia/issues/detail?id=8447.
static sk_sp<SkTypeface> create_from_desc_and_style(CTFontDescriptorRef desc,
                                                    const SkFontStyle& style) {
    SkUniqueCFRef<CTFontRef> ctFont(CTFontCreateWithFontDescriptor(desc, 0, nullptr));
    if (!ctFont) {
        return nullptr;
    }

    const CTFontSymbolicTraits traits = CTFontGetSymbolicTraits(ctFont.get());
    CTFontSymbolicTraits expected_traits = traits;
    if (style.slant() != SkFontStyle::kUpright_Slant) {
        expected_traits |= kCTFontItalicTrait;
    }
    if (style.weight() >= SkFontStyle::kBold_Weight) {
        expected_traits |= kCTFontBoldTrait;
    }

    if (expected_traits != traits) {
        SkUniqueCFRef<CTFontRef> ctNewFont(CTFontCreateCopyWithSymbolicTraits(
                    ctFont.get(), 0, nullptr, expected_traits, expected_traits));
        if (ctNewFont) {
            ctFont = std::move(ctNewFont);
        }
    }

    return SkTypeface_Mac::Make(std::move(ctFont), OpszVariation(), nullptr);
}

/** Creates a typeface from a name, searching the cache. */
static sk_sp<SkTypeface> create_from_name(const char familyName[], const SkFontStyle& style) {
    SkUniqueCFRef<CTFontDescriptorRef> desc = create_descriptor(familyName, style);
    if (!desc) {
        return nullptr;
    }
    return create_from_desc_and_style(desc.get(), style);
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

    for (size_t i = 0; i < std::size(gPairs); i++) {
        if (strcmp(name, gPairs[i].fFrom) == 0) {
            return gPairs[i].fTo;
        }
    }
    return name;    // no change
}

namespace {

static bool find_desc_str(CTFontDescriptorRef desc, CFStringRef name, SkString* value) {
    SkUniqueCFRef<CFStringRef> ref((CFStringRef)CTFontDescriptorCopyAttribute(desc, name));
    if (!ref) {
        return false;
    }
    SkStringFromCFString(ref.get(), value);
    return true;
}

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

static SkUniqueCFRef<CFSetRef> name_required() {
    CFStringRef set_values[] = {kCTFontFamilyNameAttribute};
    return SkUniqueCFRef<CFSetRef>(CFSetCreate(kCFAllocatorDefault,
        reinterpret_cast<const void**>(set_values), std::size(set_values),
        &kCFTypeSetCallBacks));
}

class SkFontStyleSet_Mac : public SkFontStyleSet {
public:
    SkFontStyleSet_Mac(CTFontDescriptorRef desc)
        : fArray(CTFontDescriptorCreateMatchingFontDescriptors(desc, name_required().get()))
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
            *style = SkCTFontDescriptorGetSkFontStyle(desc, false);
        }
        if (name) {
            if (!find_desc_str(desc, kCTFontStyleNameAttribute, name)) {
                name->reset();
            }
        }
    }

    sk_sp<SkTypeface> createTypeface(int index) override {
        SkASSERT((unsigned)index < (unsigned)CFArrayGetCount(fArray.get()));
        CTFontDescriptorRef desc = (CTFontDescriptorRef)CFArrayGetValueAtIndex(fArray.get(), index);

        return create_from_desc(desc);
    }

    sk_sp<SkTypeface> matchStyle(const SkFontStyle& pattern) override {
        if (0 == fCount) {
            return nullptr;
        }
        return create_from_desc(findMatchingDesc(pattern));
    }

private:
    SkUniqueCFRef<CFArrayRef> fArray;
    int fCount;

    CTFontDescriptorRef findMatchingDesc(const SkFontStyle& pattern) const {
        int bestMetric = SK_MaxS32;
        CTFontDescriptorRef bestDesc = nullptr;

        for (int i = 0; i < fCount; ++i) {
            CTFontDescriptorRef desc = (CTFontDescriptorRef)CFArrayGetValueAtIndex(fArray.get(), i);
            int metric = compute_metric(pattern, SkCTFontDescriptorGetSkFontStyle(desc, false));
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

SkUniqueCFRef<CFArrayRef> SkCopyAvailableFontFamilyNames(CTFontCollectionRef collection) {
    // Create a CFArray of all available font descriptors.
    SkUniqueCFRef<CFArrayRef> descriptors(
        CTFontCollectionCreateMatchingFontDescriptors(collection));

    // Copy the font family names of the font descriptors into a CFSet.
    auto addDescriptorFamilyNameToSet = [](const void* value, void* context) -> void {
        CTFontDescriptorRef descriptor = static_cast<CTFontDescriptorRef>(value);
        CFMutableSetRef familyNameSet = static_cast<CFMutableSetRef>(context);
        SkUniqueCFRef<CFTypeRef> familyName(
            CTFontDescriptorCopyAttribute(descriptor, kCTFontFamilyNameAttribute));
        if (familyName) {
            CFSetAddValue(familyNameSet, familyName.get());
        }
    };
    SkUniqueCFRef<CFMutableSetRef> familyNameSet(
        CFSetCreateMutable(kCFAllocatorDefault, 0, &kCFTypeSetCallBacks));
    CFArrayApplyFunction(descriptors.get(), CFRangeMake(0, CFArrayGetCount(descriptors.get())),
                         addDescriptorFamilyNameToSet, familyNameSet.get());

    // Get the set of family names into an array; this does not retain.
    CFIndex count = CFSetGetCount(familyNameSet.get());
    std::unique_ptr<const void*[]> familyNames(new const void*[count]);
    CFSetGetValues(familyNameSet.get(), familyNames.get());

    // Sort the array of family names (to match CTFontManagerCopyAvailableFontFamilyNames).
    std::sort(familyNames.get(), familyNames.get() + count, [](const void* a, const void* b){
        return CFStringCompare((CFStringRef)a, (CFStringRef)b, 0) == kCFCompareLessThan;
    });

    // Copy family names into a CFArray; this does retain.
    return SkUniqueCFRef<CFArrayRef>(
        CFArrayCreate(kCFAllocatorDefault, familyNames.get(), count, &kCFTypeArrayCallBacks));
}

/** Use CTFontManagerCopyAvailableFontFamilyNames if available, simulate if not. */
SkUniqueCFRef<CFArrayRef> SkCTFontManagerCopyAvailableFontFamilyNames() {
#ifdef SK_BUILD_FOR_IOS
    using CTFontManagerCopyAvailableFontFamilyNamesProc = CFArrayRef (*)(void);
    CTFontManagerCopyAvailableFontFamilyNamesProc ctFontManagerCopyAvailableFontFamilyNames;
    *(void**)(&ctFontManagerCopyAvailableFontFamilyNames) =
        dlsym(RTLD_DEFAULT, "CTFontManagerCopyAvailableFontFamilyNames");
    if (ctFontManagerCopyAvailableFontFamilyNames) {
        return SkUniqueCFRef<CFArrayRef>(ctFontManagerCopyAvailableFontFamilyNames());
    }
    SkUniqueCFRef<CTFontCollectionRef> collection(
        CTFontCollectionCreateFromAvailableFonts(nullptr));
    return SkUniqueCFRef<CFArrayRef>(SkCopyAvailableFontFamilyNames(collection.get()));
#else
    return SkUniqueCFRef<CFArrayRef>(CTFontManagerCopyAvailableFontFamilyNames());
#endif
}

} // namespace

class SkFontMgr_Mac : public SkFontMgr {
    SkUniqueCFRef<CFArrayRef> fNames;
    int fCount;

    CFStringRef getFamilyNameAt(int index) const {
        SkASSERT((unsigned)index < (unsigned)fCount);
        return (CFStringRef)CFArrayGetValueAtIndex(fNames.get(), index);
    }

    static sk_sp<SkFontStyleSet> CreateSet(CFStringRef cfFamilyName) {
        SkUniqueCFRef<CFMutableDictionaryRef> cfAttr(
                 CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
                                           &kCFTypeDictionaryKeyCallBacks,
                                           &kCFTypeDictionaryValueCallBacks));

        CFDictionaryAddValue(cfAttr.get(), kCTFontFamilyNameAttribute, cfFamilyName);

        SkUniqueCFRef<CTFontDescriptorRef> desc(
                CTFontDescriptorCreateWithAttributes(cfAttr.get()));
        return sk_sp<SkFontStyleSet>(new SkFontStyleSet_Mac(desc.get()));
    }

public:
    SkUniqueCFRef<CTFontCollectionRef> fFontCollection;
    SkFontMgr_Mac(CTFontCollectionRef fontCollection)
        : fNames(fontCollection ? SkCopyAvailableFontFamilyNames(fontCollection)
                                : SkCTFontManagerCopyAvailableFontFamilyNames())
        , fCount(fNames ? SkToInt(CFArrayGetCount(fNames.get())) : 0)
        , fFontCollection(fontCollection ? (CTFontCollectionRef)CFRetain(fontCollection)
                                         : CTFontCollectionCreateFromAvailableFonts(nullptr))
    {}

protected:
    int onCountFamilies() const override {
        return fCount;
    }

    void onGetFamilyName(int index, SkString* familyName) const override {
        if ((unsigned)index < (unsigned)fCount) {
            SkStringFromCFString(this->getFamilyNameAt(index), familyName);
        } else {
            familyName->reset();
        }
    }

    sk_sp<SkFontStyleSet> onCreateStyleSet(int index) const override {
        if ((unsigned)index >= (unsigned)fCount) {
            return nullptr;
        }
        return CreateSet(this->getFamilyNameAt(index));
    }

    sk_sp<SkFontStyleSet> onMatchFamily(const char familyName[]) const override {
        if (!familyName) {
            return nullptr;
        }
        SkUniqueCFRef<CFStringRef> cfName = make_CFString(familyName);
        return CreateSet(cfName.get());
    }

    sk_sp<SkTypeface> onMatchFamilyStyle(const char familyName[],
                                   const SkFontStyle& style) const override {
        SkUniqueCFRef<CTFontDescriptorRef> reqDesc = create_descriptor(familyName, style);
        if (!familyName) {
            return create_from_desc(reqDesc.get());
        }
        SkUniqueCFRef<CTFontDescriptorRef> resolvedDesc(
            CTFontDescriptorCreateMatchingFontDescriptor(reqDesc.get(), name_required().get()));
        if (!resolvedDesc) {
            return nullptr;
        }
        return create_from_desc(resolvedDesc.get());
    }

    sk_sp<SkTypeface> onMatchFamilyStyleCharacter(const char familyName[],
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
        // If 0xD800 <= codepoint <= 0xDFFF || 0x10FFFF < codepoint 'string' may be nullptr.
        // No font should be covering such codepoints (even the magic fallback font).
        if (!string) {
            return nullptr;
        }
        CFRange range = CFRangeMake(0, CFStringGetLength(string.get()));  // in UniChar units.
        SkUniqueCFRef<CTFontRef> fallbackFont(
                CTFontCreateForString(familyFont.get(), string.get(), range));
        return SkTypeface_Mac::Make(std::move(fallbackFont), OpszVariation(), nullptr);
    }

    sk_sp<SkTypeface> onMakeFromData(sk_sp<SkData> data, int ttcIndex) const override {
        return this->makeFromStream(
                std::unique_ptr<SkStreamAsset>(new SkMemoryStream(std::move(data))), ttcIndex);
    }

    sk_sp<SkTypeface> onMakeFromStreamIndex(std::unique_ptr<SkStreamAsset> stream,
                                            int ttcIndex) const override {
        return this->makeFromStream(std::move(stream),
                                    SkFontArguments().setCollectionIndex(ttcIndex));
    }

    sk_sp<SkTypeface> onMakeFromStreamArgs(std::unique_ptr<SkStreamAsset> stream,
                                           const SkFontArguments& args) const override {
        return SkTypeface_Mac::MakeFromStream(std::move(stream), args);
    }

    sk_sp<SkTypeface> onMakeFromFile(const char path[], int ttcIndex) const override {
        sk_sp<SkData> data = SkData::MakeFromFileName(path);
        if (!data) {
            return nullptr;
        }

        return this->onMakeFromData(std::move(data), ttcIndex);
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

sk_sp<SkFontMgr> SkFontMgr_New_CoreText(CTFontCollectionRef fontCollection) {
    return sk_make_sp<SkFontMgr_Mac>(fontCollection);
}

#endif//defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
