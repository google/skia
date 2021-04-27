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
#include "include/private/SkFixed.h"
#include "include/private/SkOnce.h"
#include "include/private/SkTPin.h"
#include "include/private/SkTemplates.h"
#include "include/private/SkTo.h"
#include "src/core/SkFontDescriptor.h"
#include "src/ports/SkTypeface_mac_ct.h"
#include "src/utils/SkUTF.h"

#include <string.h>
#include <memory>

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
    // certain traits set.  Temporary workaround to omit specifying trait for
    // those versions.
    // Long term solution will involve serializing typefaces instead of relying
    // upon this to match between processes.
    //
    // Compare CoreText.h in an up to date SDK for where these values come from.
    static const uint32_t kSkiaLocalCTVersionNumber10_14 = 0x000B0000;
    static const uint32_t kSkiaLocalCTVersionNumber10_15 = 0x000C0000;

    // CTFontTraits (symbolic)
    // macOS 14 and iOS 12 seem to behave badly when kCTFontSymbolicTrait is set.
    // macOS 15 yields LastResort font instead of a good default font when
    // kCTFontSymbolicTrait is set.
    if (!(&CTGetCoreTextVersion && CTGetCoreTextVersion() >= kSkiaLocalCTVersionNumber10_14)) {
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
    if (!(&CTGetCoreTextVersion && CTGetCoreTextVersion() == kSkiaLocalCTVersionNumber10_15)) {
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

    for (size_t i = 0; i < SK_ARRAY_COUNT(gPairs); i++) {
        if (strcmp(name, gPairs[i].fFrom) == 0) {
            return gPairs[i].fTo;
        }
    }
    return name;    // no change
}

namespace {

/** Creates a dictionary suitable for setting the axes on a CTFont. */
static CTFontVariation ctvariation_from_skfontdata(CTFontRef ct, SkFontData* fontData) {
    // In macOS 10.15 CTFontCreate* overrides any 'opsz' variation with the 'size'.
    // Track the 'opsz' and return it, since it is an out of band axis.
    OpszVariation opsz;
    constexpr const SkFourByteTag opszTag = SkSetFourByteTag('o','p','s','z');

    SkUniqueCFRef<CFArrayRef> ctAxes(CTFontCopyVariationAxes(ct));
    if (!ctAxes) {
        return CTFontVariation();
    }

    CFIndex axisCount = CFArrayGetCount(ctAxes.get());
    if (0 == axisCount || axisCount != fontData->getAxisCount()) {
        return CTFontVariation();
    }

    SkUniqueCFRef<CFMutableDictionaryRef> dict(
            CFDictionaryCreateMutable(kCFAllocatorDefault, axisCount,
                                      &kCFTypeDictionaryKeyCallBacks,
                                      &kCFTypeDictionaryValueCallBacks));

    for (int i = 0; i < fontData->getAxisCount(); ++i) {
        CFTypeRef axisInfo = CFArrayGetValueAtIndex(ctAxes.get(), i);
        if (CFDictionaryGetTypeID() != CFGetTypeID(axisInfo)) {
            return CTFontVariation();
        }
        CFDictionaryRef axisInfoDict = static_cast<CFDictionaryRef>(axisInfo);

        CFTypeRef tag = CFDictionaryGetValue(axisInfoDict,
                                             kCTFontVariationAxisIdentifierKey);
        if (!tag || CFGetTypeID(tag) != CFNumberGetTypeID()) {
            return CTFontVariation();
        }
        CFNumberRef tagNumber = static_cast<CFNumberRef>(tag);
        int64_t tagLong;
        if (!CFNumberGetValue(tagNumber, kCFNumberSInt64Type, &tagLong)) {
            return CTFontVariation();
        }

        // The variation axes can be set to any value, but cg will effectively pin them.
        // Pin them here to normalize.
        CFTypeRef min = CFDictionaryGetValue(axisInfoDict, kCTFontVariationAxisMinimumValueKey);
        CFTypeRef max = CFDictionaryGetValue(axisInfoDict, kCTFontVariationAxisMaximumValueKey);
        if (!min || CFGetTypeID(min) != CFNumberGetTypeID() ||
            !max || CFGetTypeID(max) != CFNumberGetTypeID())
        {
            return CTFontVariation();
        }
        CFNumberRef minNumber = static_cast<CFNumberRef>(min);
        CFNumberRef maxNumber = static_cast<CFNumberRef>(max);
        double minDouble;
        double maxDouble;
        if (!CFNumberGetValue(minNumber, kCFNumberDoubleType, &minDouble) ||
            !CFNumberGetValue(maxNumber, kCFNumberDoubleType, &maxDouble))
        {
            return CTFontVariation();
        }
        double value = SkTPin(SkFixedToDouble(fontData->getAxis()[i]), minDouble, maxDouble);

        if (tagLong == opszTag) {
            opsz.isSet = true;
            opsz.value = value;
        }

        SkUniqueCFRef<CFNumberRef> valueNumber(
                CFNumberCreate(kCFAllocatorDefault, kCFNumberDoubleType, &value));
        CFDictionaryAddValue(dict.get(), tagNumber, valueNumber.get());
    }
    return { SkUniqueCFRef<CFDictionaryRef>(std::move(dict)), nullptr, opsz };
}

static sk_sp<SkData> skdata_from_skstreamasset(std::unique_ptr<SkStreamAsset> stream) {
    size_t size = stream->getLength();
    if (const void* base = stream->getMemoryBase()) {
        return SkData::MakeWithProc(base, size,
                                    [](const void*, void* ctx) -> void {
                                        delete (SkStreamAsset*)ctx;
                                    }, stream.release());
    }
    return SkData::MakeFromStream(stream.get(), size);
}

static SkUniqueCFRef<CFDataRef> cfdata_from_skdata(sk_sp<SkData> data) {
    void const * const addr = data->data();
    size_t const size = data->size();

    CFAllocatorContext ctx = {
        0, // CFIndex version
        data.release(), // void* info
        nullptr, // const void *(*retain)(const void *info);
        nullptr, // void (*release)(const void *info);
        nullptr, // CFStringRef (*copyDescription)(const void *info);
        nullptr, // void * (*allocate)(CFIndex size, CFOptionFlags hint, void *info);
        nullptr, // void*(*reallocate)(void* ptr,CFIndex newsize,CFOptionFlags hint,void* info);
        [](void*,void* info) -> void { // void (*deallocate)(void *ptr, void *info);
            SkASSERT(info);
            ((SkData*)info)->unref();
        },
        nullptr, // CFIndex (*preferredSize)(CFIndex size, CFOptionFlags hint, void *info);
    };
    SkUniqueCFRef<CFAllocatorRef> alloc(CFAllocatorCreate(kCFAllocatorDefault, &ctx));
    return SkUniqueCFRef<CFDataRef>(CFDataCreateWithBytesNoCopy(
            kCFAllocatorDefault, (const UInt8 *)addr, size, alloc.get()));
}

static SkUniqueCFRef<CTFontRef> ctfont_from_skdata(sk_sp<SkData> data, int ttcIndex) {
    // TODO: Use CTFontManagerCreateFontDescriptorsFromData when available.
    if (ttcIndex != 0) {
        return nullptr;
    }

    SkUniqueCFRef<CFDataRef> cfData(cfdata_from_skdata(std::move(data)));

    SkUniqueCFRef<CTFontDescriptorRef> desc(
            CTFontManagerCreateFontDescriptorFromData(cfData.get()));
    if (!desc) {
        return nullptr;
    }
    return SkUniqueCFRef<CTFontRef>(CTFontCreateWithFontDescriptor(desc.get(), 0, nullptr));
}

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
            *style = SkCTFontDescriptorGetSkFontStyle(desc, false);
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
        // If 0xD800 <= codepoint <= 0xDFFF || 0x10FFFF < codepoint 'string' may be nullptr.
        // No font should be covering such codepoints (even the magic fallback font).
        if (!string) {
            return nullptr;
        }
        CFRange range = CFRangeMake(0, CFStringGetLength(string.get()));  // in UniChar units.
        SkUniqueCFRef<CTFontRef> fallbackFont(
                CTFontCreateForString(familyFont.get(), string.get(), range));
        return SkTypeface_Mac::Make(std::move(fallbackFont), OpszVariation(), nullptr).release();
    }

    SkTypeface* onMatchFaceStyle(const SkTypeface* familyMember,
                                 const SkFontStyle&) const override {
        return nullptr;
    }

    sk_sp<SkTypeface> onMakeFromData(sk_sp<SkData> data, int ttcIndex) const override {
        if (ttcIndex != 0) {
            return nullptr;
        }

        SkUniqueCFRef<CTFontRef> ct = ctfont_from_skdata(data, ttcIndex);
        if (!ct) {
            return nullptr;
        }

        return SkTypeface_Mac::Make(std::move(ct), OpszVariation(),
                                    SkMemoryStream::Make(std::move(data)));
    }

    sk_sp<SkTypeface> onMakeFromStreamIndex(std::unique_ptr<SkStreamAsset> stream,
                                            int ttcIndex) const override {
        if (ttcIndex != 0) {
            return nullptr;
        }

        sk_sp<SkData> data = skdata_from_skstreamasset(stream->duplicate());
        if (!data) {
            return nullptr;
        }
        SkUniqueCFRef<CTFontRef> ct = ctfont_from_skdata(std::move(data), ttcIndex);
        if (!ct) {
            return nullptr;
        }

        return SkTypeface_Mac::Make(std::move(ct), OpszVariation(), std::move(stream));
    }

    sk_sp<SkTypeface> onMakeFromStreamArgs(std::unique_ptr<SkStreamAsset> stream,
                                           const SkFontArguments& args) const override
    {
        // TODO: Use CTFontManagerCreateFontDescriptorsFromData when available.
        int ttcIndex = args.getCollectionIndex();
        if (ttcIndex != 0) {
            return nullptr;
        }

        sk_sp<SkData> data = skdata_from_skstreamasset(stream->duplicate());
        if (!data) {
            return nullptr;
        }
        SkUniqueCFRef<CTFontRef> ct = ctfont_from_skdata(std::move(data), ttcIndex);
        if (!ct) {
            return nullptr;
        }

        CTFontVariation ctVariation = SkCTVariationFromSkFontArguments(ct.get(), args);

        SkUniqueCFRef<CTFontRef> ctVariant;
        if (ctVariation.variation) {
            SkUniqueCFRef<CFMutableDictionaryRef> attributes(
                    CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
                                              &kCFTypeDictionaryKeyCallBacks,
                                              &kCFTypeDictionaryValueCallBacks));
            CFDictionaryAddValue(attributes.get(),
                                 kCTFontVariationAttribute, ctVariation.variation.get());
            SkUniqueCFRef<CTFontDescriptorRef> varDesc(
                    CTFontDescriptorCreateWithAttributes(attributes.get()));
            ctVariant.reset(CTFontCreateCopyWithAttributes(ct.get(), 0, nullptr, varDesc.get()));
        } else {
            ctVariant.reset(ct.release());
        }
        if (!ctVariant) {
            return nullptr;
        }

        return SkTypeface_Mac::Make(std::move(ctVariant), ctVariation.opsz, std::move(stream));
    }

    sk_sp<SkTypeface> onMakeFromFontData(std::unique_ptr<SkFontData> fontData) const override {
        // TODO: Use CTFontManagerCreateFontDescriptorsFromData when available.
        if (fontData->getIndex() != 0) {
            return nullptr;
        }

        sk_sp<SkData> data = skdata_from_skstreamasset(fontData->getStream()->duplicate());
        if (!data) {
            return nullptr;
        }
        SkUniqueCFRef<CTFontRef> ct = ctfont_from_skdata(std::move(data), fontData->getIndex());
        if (!ct) {
            return nullptr;
        }

        CTFontVariation ctVariation = ctvariation_from_skfontdata(ct.get(), fontData.get());

        SkUniqueCFRef<CTFontRef> ctVariant;
        if (ctVariation.variation) {
            SkUniqueCFRef<CFMutableDictionaryRef> attributes(
                    CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
                                              &kCFTypeDictionaryKeyCallBacks,
                                              &kCFTypeDictionaryValueCallBacks));
            CFDictionaryAddValue(attributes.get(),
                                 kCTFontVariationAttribute, ctVariation.variation.get());
            SkUniqueCFRef<CTFontDescriptorRef> varDesc(
                    CTFontDescriptorCreateWithAttributes(attributes.get()));
            ctVariant.reset(CTFontCreateCopyWithAttributes(ct.get(), 0, nullptr, varDesc.get()));
        } else {
            ctVariant.reset(ct.release());
        }
        if (!ctVariant) {
            return nullptr;
        }

        return SkTypeface_Mac::Make(std::move(ctVariant), ctVariation.opsz,
                                    fontData->detachStream());
    }

    sk_sp<SkTypeface> onMakeFromFile(const char path[], int ttcIndex) const override {
        if (ttcIndex != 0) {
            return nullptr;
        }

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
