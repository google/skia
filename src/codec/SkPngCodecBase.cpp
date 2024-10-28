/*
 * Copyright 2024 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/codec/SkPngCodecBase.h"

#include <cstddef>
#include <tuple>
#include <utility>

#include "include/codec/SkCodec.h"
#include "include/codec/SkEncodedImageFormat.h"
#include "include/core/SkAlphaType.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRect.h"
#include "include/core/SkStream.h"
#include "include/private/SkEncodedInfo.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkSpan_impl.h"
#include "modules/skcms/skcms.h"
#include "src/codec/SkCodecPriv.h"
#include "src/codec/SkColorPalette.h"
#include "src/codec/SkSwizzler.h"
#include "src/core/SkMemset.h"
#include "src/core/SkSwizzlePriv.h"

namespace {

constexpr SkColorType kXformSrcColorType = kRGBA_8888_SkColorType;

inline bool needs_premul(SkAlphaType dstAT, SkEncodedInfo::Alpha encodedAlpha) {
    return kPremul_SkAlphaType == dstAT && SkEncodedInfo::kUnpremul_Alpha == encodedAlpha;
}

skcms_PixelFormat ToPixelFormat(const SkEncodedInfo& info) {
    // We use kRGB and kRGBA formats because color PNGs are always RGB or RGBA.
    if (16 == info.bitsPerComponent()) {
        if (SkEncodedInfo::kRGBA_Color == info.color()) {
            return skcms_PixelFormat_RGBA_16161616BE;
        } else if (SkEncodedInfo::kRGB_Color == info.color()) {
            return skcms_PixelFormat_RGB_161616BE;
        }
    } else if (SkEncodedInfo::kGray_Color == info.color()) {
        return skcms_PixelFormat_G_8;
    }

    return skcms_PixelFormat_RGBA_8888;
}

}  // namespace

SkPngCodecBase::~SkPngCodecBase() = default;

// static
bool SkPngCodecBase::isCompatibleColorProfileAndType(const SkEncodedInfo::ICCProfile* profile,
                                                     SkEncodedInfo::Color color) {
    if (profile) {
        switch (profile->profile()->data_color_space) {
            case skcms_Signature_CMYK:
                return false;
            case skcms_Signature_Gray:
                if (SkEncodedInfo::kGray_Color != color &&
                    SkEncodedInfo::kGrayAlpha_Color != color) {
                    return false;
                }
                break;
            default:
                break;
        }
    }

    return true;
}

SkPngCodecBase::SkPngCodecBase(SkEncodedInfo&& encodedInfo, std::unique_ptr<SkStream> stream)
        : SkCodec(std::move(encodedInfo), ToPixelFormat(encodedInfo), std::move(stream)) {}

SkEncodedImageFormat SkPngCodecBase::onGetEncodedFormat() const {
    return SkEncodedImageFormat::kPNG;
}

SkCodec::Result SkPngCodecBase::initializeXforms(const SkImageInfo& dstInfo,
                                                 const Options& options,
                                                 int frameWidth) {
    if (frameWidth != dstInfo.width() && options.fSubset) {
        return kInvalidParameters;
    }
    fXformWidth = frameWidth;

    {
        size_t encodedBitsPerPixel = static_cast<size_t>(getEncodedInfo().bitsPerPixel());

        // We assume that `frameWidth` and `bitsPerPixel` have been already sanitized
        // earlier (and that the multiplication and addition below won't overflow).
        SkASSERT(0 < frameWidth);
        SkASSERT(frameWidth < 0xFFFFFF);
        SkASSERT(encodedBitsPerPixel < 128);

        size_t encodedBitsPerRow = static_cast<size_t>(frameWidth) * encodedBitsPerPixel;
        fEncodedRowBytes = (encodedBitsPerRow + 7) / 8;  // Round up to the next byte.

#if defined(SK_DEBUG)
        size_t dstBytesPerPixel = dstInfo.bytesPerPixel();
        fDstRowBytes = static_cast<size_t>(frameWidth) * dstBytesPerPixel;
#endif
    }

    // Reset fSwizzler and this->colorXform().  We can't do this in onRewind() because the
    // interlaced scanline decoder may need to rewind.
    fSwizzler.reset(nullptr);

    // If skcms directly supports the encoded PNG format, we should skip format
    // conversion in the swizzler (or skip swizzling altogether).
    bool skipFormatConversion = false;
    switch (this->getEncodedInfo().color()) {
        case SkEncodedInfo::kRGB_Color:
            if (this->getEncodedInfo().bitsPerComponent() != 16) {
                break;
            }
            [[fallthrough]];
        case SkEncodedInfo::kRGBA_Color:
        case SkEncodedInfo::kGray_Color:
            skipFormatConversion = this->colorXform();
            break;
        default:
            break;
    }

    if (skipFormatConversion && !options.fSubset) {
        fXformMode = kColorOnly_XformMode;
    } else {
        if (SkEncodedInfo::kPalette_Color == this->getEncodedInfo().color()) {
            if (!this->createColorTable(dstInfo)) {
                return kInvalidInput;
            }
        }

        Result result =
                this->initializeSwizzler(dstInfo, options, skipFormatConversion, frameWidth);
        if (result != kSuccess) {
            return result;
        }
    }

    this->allocateStorage(dstInfo);

    // We can't call `initializeXformParams` here, because `swizzleWidth` may
    // change *after* `onStartIncrementalDecode`
    // (`SkSampledCodec::sampledDecode` first [transitively] calls
    // `onStartIncrementalDecode` and *then* `SkSwizzler::onSetSampleX`).

    return kSuccess;
}

void SkPngCodecBase::initializeXformParams() {
    if (fXformMode == kSwizzleColor_XformMode) {
        fXformWidth = this->swizzler()->swizzleWidth();
    }
}

void SkPngCodecBase::allocateStorage(const SkImageInfo& dstInfo) {
    switch (fXformMode) {
        case kSwizzleOnly_XformMode:
            break;
        case kColorOnly_XformMode:
            // Intentional fall through.  A swizzler hasn't been created yet, but one will
            // be created later if we are sampling.  We'll go ahead and allocate
            // enough memory to swizzle if necessary.
        case kSwizzleColor_XformMode: {
            const int bitsPerPixel = this->getEncodedInfo().bitsPerPixel();

            // If we have more than 8-bits (per component) of precision, we will keep that
            // extra precision.  Otherwise, we will swizzle to RGBA_8888 before transforming.
            const size_t bytesPerPixel = (bitsPerPixel > 32) ? bitsPerPixel / 8 : 4;
            const size_t colorXformBytes = dstInfo.width() * bytesPerPixel;
            fStorage.reset(colorXformBytes);
            break;
        }
    }
}

SkCodec::Result SkPngCodecBase::initializeSwizzler(const SkImageInfo& dstInfo,
                                                   const Options& options,
                                                   bool skipFormatConversion,
                                                   int frameWidth) {
    SkImageInfo swizzlerInfo = dstInfo;
    Options swizzlerOptions = options;
    fXformMode = kSwizzleOnly_XformMode;
    if (this->colorXform() && this->xformOnDecode()) {
        if (SkEncodedInfo::kGray_Color == this->getEncodedInfo().color()) {
            swizzlerInfo = swizzlerInfo.makeColorType(kGray_8_SkColorType);
        } else {
            swizzlerInfo = swizzlerInfo.makeColorType(kXformSrcColorType);
        }
        if (kPremul_SkAlphaType == dstInfo.alphaType()) {
            swizzlerInfo = swizzlerInfo.makeAlphaType(kUnpremul_SkAlphaType);
        }

        fXformMode = kSwizzleColor_XformMode;

        // Here, we swizzle into temporary memory, which is not zero initialized.
        // FIXME (msarett):
        // Is this a problem?
        swizzlerOptions.fZeroInitialized = kNo_ZeroInitialized;
    }

    SkIRect frameRect = SkIRect::MakeWH(frameWidth, 1);
    const SkIRect* frameRectPtr = nullptr;
    if (options.fSubset) {
        SkASSERT(frameWidth == dstInfo.width());
    } else {
        frameRectPtr = &frameRect;
    }

    if (skipFormatConversion) {
        // We cannot skip format conversion when there is a color table.
        SkASSERT(!fColorTable);
        int srcBPP = 0;
        switch (this->getEncodedInfo().color()) {
            case SkEncodedInfo::kRGB_Color:
                SkASSERT(this->getEncodedInfo().bitsPerComponent() == 16);
                srcBPP = 6;
                break;
            case SkEncodedInfo::kRGBA_Color:
                srcBPP = this->getEncodedInfo().bitsPerComponent() / 2;
                break;
            case SkEncodedInfo::kGray_Color:
                srcBPP = 1;
                break;
            default:
                SkASSERT(false);
                break;
        }
        fSwizzler = SkSwizzler::MakeSimple(srcBPP, swizzlerInfo, swizzlerOptions, frameRectPtr);
    } else {
        const SkPMColor* colors = get_color_ptr(fColorTable.get());
        fSwizzler = SkSwizzler::Make(
                this->getEncodedInfo(), colors, swizzlerInfo, swizzlerOptions, frameRectPtr);
    }

    return !!fSwizzler ? kSuccess : kUnimplemented;
}

SkSampler* SkPngCodecBase::getSampler(bool createIfNecessary) {
    if (fSwizzler || !createIfNecessary) {
        return fSwizzler.get();
    }

    // Ok to ignore `initializeSwizzler`'s result, because if it fails, then
    // `fSwizzler` will be `nullptr` and we want to return `nullptr` upon
    // failure.
    std::ignore = this->initializeSwizzler(
            this->dstInfo(), this->options(), true, this->dstInfo().width());

    return fSwizzler.get();
}

void SkPngCodecBase::applyXformRow(SkSpan<uint8_t> dstRow, SkSpan<const uint8_t> srcRow) {
    SkASSERT(dstRow.size() >= fDstRowBytes);
    SkASSERT(srcRow.size() >= fEncodedRowBytes);
    applyXformRow(dstRow.data(), srcRow.data());
}

void SkPngCodecBase::applyXformRow(void* dstRow, const uint8_t* srcRow) {
    switch (fXformMode) {
        case kSwizzleOnly_XformMode:
            fSwizzler->swizzle(dstRow, srcRow);
            break;
        case kColorOnly_XformMode:
            this->applyColorXform(dstRow, srcRow, fXformWidth);
            break;
        case kSwizzleColor_XformMode:
            fSwizzler->swizzle(fStorage.get(), srcRow);
            this->applyColorXform(dstRow, fStorage.get(), fXformWidth);
            break;
    }
}

// Note: SkColorPalette claims to store SkPMColors, which is not necessarily the case here.
bool SkPngCodecBase::createColorTable(const SkImageInfo& dstInfo) {
    std::optional<SkSpan<const PaletteColorEntry>> maybePlteChunk = this->onTryGetPlteChunk();
    if (!maybePlteChunk.has_value()) {
        return false;
    }
    const PaletteColorEntry* palette = maybePlteChunk->data();
    size_t numColors = maybePlteChunk->size();

    // Contents depend on tableColorType and our choice of if/when to premultiply:
    // { kPremul, kUnpremul, kOpaque } x { RGBA, BGRA }
    SkPMColor colorTable[256];
    SkColorType tableColorType = this->colorXform() ? kXformSrcColorType : dstInfo.colorType();

    std::optional<SkSpan<const uint8_t>> maybeTrnsChunk = this->onTryGetTrnsChunk();
    const uint8_t* alphas = nullptr;
    size_t numColorsWithAlpha = 0;
    if (maybeTrnsChunk.has_value()) {
        alphas = maybeTrnsChunk->data();
        numColorsWithAlpha = maybeTrnsChunk->size();
    }

    if (alphas) {
        bool premultiply = needs_premul(dstInfo.alphaType(), this->getEncodedInfo().alpha());

        // Choose which function to use to create the color table. If the final destination's
        // colortype is unpremultiplied, the color table will store unpremultiplied colors.
        PackColorProc proc = choose_pack_color_proc(premultiply, tableColorType);

        for (size_t i = 0; i < numColorsWithAlpha; i++) {
            // We don't have a function in SkOpts that combines a set of alphas with a set
            // of RGBs.  We could write one, but it's hardly worth it, given that this
            // is such a small fraction of the total decode time.
            colorTable[i] = proc(alphas[i], palette->red, palette->green, palette->blue);
            palette++;
        }
    }

    if (numColorsWithAlpha < numColors) {
        // The optimized code depends on a 3-byte png_color struct with the colors
        // in RGB order.  These checks make sure it is safe to use.
        static_assert(3 == sizeof(PaletteColorEntry));
        static_assert(offsetof(PaletteColorEntry, red) == 0);
        static_assert(offsetof(PaletteColorEntry, green) == 1);
        static_assert(offsetof(PaletteColorEntry, blue) == 2);

        if (is_rgba(tableColorType)) {
            SkOpts::RGB_to_RGB1(colorTable + numColorsWithAlpha,
                                (const uint8_t*)palette,
                                numColors - numColorsWithAlpha);
        } else {
            SkOpts::RGB_to_BGR1(colorTable + numColorsWithAlpha,
                                (const uint8_t*)palette,
                                numColors - numColorsWithAlpha);
        }
    }

    if (this->colorXform() && !this->xformOnDecode()) {
        this->applyColorXform(colorTable, colorTable, numColors);
    }

    // Pad the color table with the last color in the table (or black) in the case that
    // invalid pixel indices exceed the number of colors in the table.
    const size_t maxColors = static_cast<size_t>(1) << this->getEncodedInfo().bitsPerComponent();
    if (numColors < maxColors) {
        SkPMColor lastColor = numColors > 0 ? colorTable[numColors - 1] : SK_ColorBLACK;
        SkOpts::memset32(colorTable + numColors, lastColor, maxColors - numColors);
    }

    fColorTable.reset(new SkColorPalette(colorTable, maxColors));
    return true;
}
