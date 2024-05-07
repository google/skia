/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkScalar.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkGradientShader.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/gpu/GrTypes.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/private/base/SkTArray.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/base/SkRectMemcpy.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/core/SkImageInfoPriv.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrDataUtils.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/GrImageInfo.h"
#include "src/gpu/ganesh/GrPixmap.h"
#include "src/gpu/ganesh/GrSamplerState.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/gpu/ganesh/SurfaceContext.h"
#include "src/gpu/ganesh/SurfaceFillContext.h"
#include "src/gpu/ganesh/effects/GrTextureEffect.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"
#include "tests/TestUtils.h"
#include "tools/ToolUtils.h"
#include "tools/gpu/BackendSurfaceFactory.h"
#include "tools/gpu/BackendTextureImageFactory.h"
#include "tools/gpu/ContextType.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <functional>
#include <initializer_list>
#include <memory>
#include <utility>
#include <vector>

using namespace skia_private;

struct GrContextOptions;

static constexpr int min_rgb_channel_bits(SkColorType ct) {
    switch (ct) {
        case kUnknown_SkColorType:            return 0;
        case kAlpha_8_SkColorType:            return 0;
        case kA16_unorm_SkColorType:          return 0;
        case kA16_float_SkColorType:          return 0;
        case kRGB_565_SkColorType:            return 5;
        case kARGB_4444_SkColorType:          return 4;
        case kR8G8_unorm_SkColorType:         return 8;
        case kR16G16_unorm_SkColorType:       return 16;
        case kR16G16_float_SkColorType:       return 16;
        case kRGBA_8888_SkColorType:          return 8;
        case kSRGBA_8888_SkColorType:         return 8;
        case kRGB_888x_SkColorType:           return 8;
        case kBGRA_8888_SkColorType:          return 8;
        case kRGBA_1010102_SkColorType:       return 10;
        case kRGB_101010x_SkColorType:        return 10;
        case kBGRA_1010102_SkColorType:       return 10;
        case kBGR_101010x_SkColorType:        return 10;
        case kBGR_101010x_XR_SkColorType:     return 10;
        case kRGBA_10x6_SkColorType:          return 10;
        case kBGRA_10101010_XR_SkColorType:   return 10;
        case kGray_8_SkColorType:             return 8;   // counting gray as "rgb"
        case kRGBA_F16Norm_SkColorType:       return 10;  // just counting the mantissa
        case kRGBA_F16_SkColorType:           return 10;  // just counting the mantissa
        case kRGBA_F32_SkColorType:           return 23;  // just counting the mantissa
        case kR16G16B16A16_unorm_SkColorType: return 16;
        case kR8_unorm_SkColorType:           return 8;
    }
    SkUNREACHABLE;
}

static constexpr int alpha_channel_bits(SkColorType ct) {
    switch (ct) {
        case kUnknown_SkColorType:            return 0;
        case kAlpha_8_SkColorType:            return 8;
        case kA16_unorm_SkColorType:          return 16;
        case kA16_float_SkColorType:          return 16;
        case kRGB_565_SkColorType:            return 0;
        case kARGB_4444_SkColorType:          return 4;
        case kR8G8_unorm_SkColorType:         return 0;
        case kR16G16_unorm_SkColorType:       return 0;
        case kR16G16_float_SkColorType:       return 0;
        case kRGBA_8888_SkColorType:          return 8;
        case kSRGBA_8888_SkColorType:         return 8;
        case kRGB_888x_SkColorType:           return 0;
        case kBGRA_8888_SkColorType:          return 8;
        case kRGBA_1010102_SkColorType:       return 2;
        case kRGB_101010x_SkColorType:        return 0;
        case kBGRA_1010102_SkColorType:       return 2;
        case kBGR_101010x_SkColorType:        return 0;
        case kBGR_101010x_XR_SkColorType:     return 0;
        case kRGBA_10x6_SkColorType:          return 10;
        case kBGRA_10101010_XR_SkColorType:   return 10;
        case kGray_8_SkColorType:             return 0;
        case kRGBA_F16Norm_SkColorType:       return 10;  // just counting the mantissa
        case kRGBA_F16_SkColorType:           return 10;  // just counting the mantissa
        case kRGBA_F32_SkColorType:           return 23;  // just counting the mantissa
        case kR16G16B16A16_unorm_SkColorType: return 16;
        case kR8_unorm_SkColorType:           return 0;
    }
    SkUNREACHABLE;
}

std::vector<SkIRect> make_long_rect_array(int w, int h) {
    return {
            // entire thing
            SkIRect::MakeWH(w, h),
            // larger on all sides
            SkIRect::MakeLTRB(-10, -10, w + 10, h + 10),
            // fully contained
            SkIRect::MakeLTRB(w/4, h/4, 3*w/4, 3*h/4),
            // outside top left
            SkIRect::MakeLTRB(-10, -10, -1, -1),
            // touching top left corner
            SkIRect::MakeLTRB(-10, -10, 0, 0),
            // overlapping top left corner
            SkIRect::MakeLTRB(-10, -10, w/4, h/4),
            // overlapping top left and top right corners
            SkIRect::MakeLTRB(-10, -10, w + 10, h/4),
            // touching entire top edge
            SkIRect::MakeLTRB(-10, -10, w + 10, 0),
            // overlapping top right corner
            SkIRect::MakeLTRB(3*w/4, -10, w + 10, h/4),
            // contained in x, overlapping top edge
            SkIRect::MakeLTRB(w/4, -10, 3*w/4, h/4),
            // outside top right corner
            SkIRect::MakeLTRB(w + 1, -10, w + 10, -1),
            // touching top right corner
            SkIRect::MakeLTRB(w, -10, w + 10, 0),
            // overlapping top left and bottom left corners
            SkIRect::MakeLTRB(-10, -10, w/4, h + 10),
            // touching entire left edge
            SkIRect::MakeLTRB(-10, -10, 0, h + 10),
            // overlapping bottom left corner
            SkIRect::MakeLTRB(-10, 3*h/4, w/4, h + 10),
            // contained in y, overlapping left edge
            SkIRect::MakeLTRB(-10, h/4, w/4, 3*h/4),
            // outside bottom left corner
            SkIRect::MakeLTRB(-10, h + 1, -1, h + 10),
            // touching bottom left corner
            SkIRect::MakeLTRB(-10, h, 0, h + 10),
            // overlapping bottom left and bottom right corners
            SkIRect::MakeLTRB(-10, 3*h/4, w + 10, h + 10),
            // touching entire left edge
            SkIRect::MakeLTRB(0, h, w, h + 10),
            // overlapping bottom right corner
            SkIRect::MakeLTRB(3*w/4, 3*h/4, w + 10, h + 10),
            // overlapping top right and bottom right corners
            SkIRect::MakeLTRB(3*w/4, -10, w + 10, h + 10),
    };
}

std::vector<SkIRect> make_short_rect_array(int w, int h) {
    return {
            // entire thing
            SkIRect::MakeWH(w, h),
            // fully contained
            SkIRect::MakeLTRB(w/4, h/4, 3*w/4, 3*h/4),
            // overlapping top right corner
            SkIRect::MakeLTRB(3*w/4, -10, w + 10, h/4),
    };
}

namespace {

struct GpuReadPixelTestRules {
    // Test unpremul sources? We could omit this and detect that creating the source of the read
    // failed but having it lets us skip generating reference color data.
    bool fAllowUnpremulSrc = true;
    // Are reads that are overlapping but not contained by the src bounds expected to succeed?
    bool fUncontainedRectSucceeds = true;
    // Skip SRGB src colortype?
    bool fSkipSRGBCT = false;
    // Skip 16-bit src colortypes?
    bool fSkip16BitCT = false;
};

// Makes a src populated with the pixmap. The src should get its image info (or equivalent) from
// the pixmap.
template <typename T> using GpuSrcFactory = T(SkPixmap&);

enum class Result {
    kFail,
    kSuccess,
    kExcusedFailure,
};

// Does a read from the T into the pixmap.
template <typename T>
using GpuReadSrcFn = Result(const T&, const SkIPoint& offset, const SkPixmap&);

// Makes a dst for testing writes.
template <typename T> using GpuDstFactory = T(const SkImageInfo& ii);

// Does a write from the pixmap to the T.
template <typename T>
using GpuWriteDstFn = Result(const T&, const SkIPoint& offset, const SkPixmap&);

// To test the results of the write we do a read. This reads the entire src T. It should do a non-
// converting read (i.e. the image info of the returned pixmap matches that of the T).
template <typename T>
using GpuReadDstFn = SkAutoPixmapStorage(const T&);

}  // anonymous namespace

SkPixmap make_pixmap_have_valid_alpha_type(SkPixmap pm) {
    if (pm.alphaType() == kUnknown_SkAlphaType) {
        return {pm.info().makeAlphaType(kUnpremul_SkAlphaType), pm.addr(), pm.rowBytes()};
    }
    return pm;
}

static SkAutoPixmapStorage make_ref_data(const SkImageInfo& info, bool forceOpaque) {
    SkAutoPixmapStorage result;
    if (info.alphaType() == kUnknown_SkAlphaType) {
        result.alloc(info.makeAlphaType(kUnpremul_SkAlphaType));
    } else {
        result.alloc(info);
    }
    auto surface = SkSurfaces::WrapPixels(result);
    if (!surface) {
        return result;
    }

    SkPoint pts1[] = {{0, 0}, {float(info.width()), float(info.height())}};
    static constexpr SkColor kColors1[] = {SK_ColorGREEN, SK_ColorRED};
    SkPaint paint;
    paint.setShader(SkGradientShader::MakeLinear(pts1, kColors1, nullptr, 2, SkTileMode::kClamp));
    surface->getCanvas()->drawPaint(paint);

    SkPoint pts2[] = {{float(info.width()), 0}, {0, float(info.height())}};
    static constexpr SkColor kColors2[] = {SK_ColorBLUE, SK_ColorBLACK};
    paint.setShader(SkGradientShader::MakeLinear(pts2, kColors2, nullptr, 2, SkTileMode::kClamp));
    paint.setBlendMode(SkBlendMode::kPlus);
    surface->getCanvas()->drawPaint(paint);

    // If not opaque add some fractional alpha.
    if (info.alphaType() != kOpaque_SkAlphaType && !forceOpaque) {
        static constexpr SkColor kColors3[] = {SK_ColorWHITE,
                                               SK_ColorWHITE,
                                               0x60FFFFFF,
                                               SK_ColorWHITE,
                                               SK_ColorWHITE};
        static constexpr SkScalar kPos3[] = {0.f, 0.15f, 0.5f, 0.85f, 1.f};
        paint.setShader(SkGradientShader::MakeRadial({info.width()/2.f, info.height()/2.f},
                                                     (info.width() + info.height())/10.f,
                                                     kColors3, kPos3, 5, SkTileMode::kMirror));
        paint.setBlendMode(SkBlendMode::kDstIn);
        surface->getCanvas()->drawPaint(paint);
    }
    return result;
}

template <typename T>
static void gpu_read_pixels_test_driver(skiatest::Reporter* reporter,
                                        const GpuReadPixelTestRules& rules,
                                        const std::function<GpuSrcFactory<T>>& srcFactory,
                                        const std::function<GpuReadSrcFn<T>>& read,
                                        SkString label) {
    if (!label.isEmpty()) {
        // Add space for printing.
        label.append(" ");
    }
    // Separate this out just to give it some line width to breathe. Note 'srcPixels' should have
    // the same image info as src. We will do a converting readPixels() on it to get the data
    // to compare with the results of 'read'.
    auto runTest = [&](const T& src,
                       const SkPixmap& srcPixels,
                       const SkImageInfo& readInfo,
                       SkIPoint offset) {
        const bool csConversion =
                !SkColorSpace::Equals(readInfo.colorSpace(), srcPixels.info().colorSpace());
        const auto readCT = readInfo.colorType();
        const auto readAT = readInfo.alphaType();
        const auto srcCT = srcPixels.info().colorType();
        const auto srcAT = srcPixels.info().alphaType();
        const auto rect = SkIRect::MakeWH(readInfo.width(), readInfo.height()).makeOffset(offset);
        const auto surfBounds = SkIRect::MakeWH(srcPixels.width(), srcPixels.height());
        const size_t readBpp = SkColorTypeBytesPerPixel(readCT);

        // Make the row bytes in the dst be loose for extra stress.
        const size_t dstRB = readBpp * readInfo.width() + 10 * readBpp;
        // This will make the last row tight.
        const size_t dstSize = readInfo.computeByteSize(dstRB);
        std::unique_ptr<char[]> dstData(new char[dstSize]);
        SkPixmap dstPixels(readInfo, dstData.get(), dstRB);
        // Initialize with an arbitrary value for each byte. Later we will check that only the
        // correct part of the destination gets overwritten by 'read'.
        static constexpr auto kInitialByte = static_cast<char>(0x1B);
        std::fill_n(static_cast<char*>(dstPixels.writable_addr()),
                    dstPixels.computeByteSize(),
                    kInitialByte);

        const Result result = read(src, offset, dstPixels);

        if (!SkIRect::Intersects(rect, surfBounds)) {
            REPORTER_ASSERT(reporter, result != Result::kSuccess);
        } else if (readCT == kUnknown_SkColorType) {
            REPORTER_ASSERT(reporter, result != Result::kSuccess);
        } else if ((readAT == kUnknown_SkAlphaType) != (srcAT == kUnknown_SkAlphaType)) {
            REPORTER_ASSERT(reporter, result != Result::kSuccess);
        } else if (!rules.fUncontainedRectSucceeds && !surfBounds.contains(rect)) {
            REPORTER_ASSERT(reporter, result != Result::kSuccess);
        } else if (result == Result::kFail) {
            // TODO: Support RGB/BGR 101010x, BGRA 1010102 on the GPU.
            if (SkColorTypeToGrColorType(readCT) != GrColorType::kUnknown) {
                ERRORF(reporter,
                       "Read failed. %sSrc CT: %s, Src AT: %s Read CT: %s, Read AT: %s, "
                       "Rect [%d, %d, %d, %d], CS conversion: %d\n",
                       label.c_str(),
                       ToolUtils::colortype_name(srcCT), ToolUtils::alphatype_name(srcAT),
                       ToolUtils::colortype_name(readCT), ToolUtils::alphatype_name(readAT),
                       rect.fLeft, rect.fTop, rect.fRight, rect.fBottom, csConversion);
            }
            return result;
        }

        bool guardOk = true;
        auto guardCheck = [](char x) { return x == kInitialByte; };

        // Considering the rect we tried to read and the surface bounds figure  out which pixels in
        // both src and dst space should actually have been read and written.
        SkIRect srcReadRect;
        if (result == Result::kSuccess && srcReadRect.intersect(surfBounds, rect)) {
            SkIRect dstWriteRect = srcReadRect.makeOffset(-rect.fLeft, -rect.fTop);

            const bool lumConversion =
                    !(SkColorTypeChannelFlags(srcCT) & kGray_SkColorChannelFlag) &&
                    (SkColorTypeChannelFlags(readCT) & kGray_SkColorChannelFlag);
            // A CS or luminance conversion allows a 3 value difference and otherwise a 2 value
            // difference. Note that sometimes read back on GPU can be lossy even when there no
            // conversion at all because GPU->CPU read may go to a lower bit depth format and then
            // be promoted back to the original type. For example, GL ES cannot read to 1010102, so
            // we go through 8888.
            float numer = (lumConversion || csConversion) ? 3.f : 2.f;
            // Allow some extra tolerance if unpremuling.
            if (srcAT == kPremul_SkAlphaType && readAT == kUnpremul_SkAlphaType) {
                numer += 1;
            }
            int rgbBits = std::min({min_rgb_channel_bits(readCT), min_rgb_channel_bits(srcCT), 8});
            float tol = numer / (1 << rgbBits);
            // Swiftshader is producing alpha errors with 16-bit UNORM. We choose to always allow
            // a small tolerance:
            float alphaTol = 1.f / (1 << 10);
            if (readAT != kOpaque_SkAlphaType && srcAT != kOpaque_SkAlphaType) {
                // Alpha can also get squashed down to 8 bits going through an intermediate
                // color format.
                const int alphaBits = std::min({alpha_channel_bits(readCT),
                                                alpha_channel_bits(srcCT),
                                                8});
                alphaTol = 2.f / (1 << alphaBits);
            }

            const float tols[4] = {tol, tol, tol, alphaTol};
            auto error = std::function<ComparePixmapsErrorReporter>([&](int x, int y,
                                                                        const float diffs[4]) {
                SkASSERT(x >= 0 && y >= 0);
                ERRORF(reporter,
                       "%sSrc CT: %s, Src AT: %s, Read CT: %s, Read AT: %s, Rect [%d, %d, %d, %d]"
                       ", CS conversion: %d\n"
                       "Error at %d, %d. Diff in floats: (%f, %f, %f, %f)",
                       label.c_str(),
                       ToolUtils::colortype_name(srcCT), ToolUtils::alphatype_name(srcAT),
                       ToolUtils::colortype_name(readCT), ToolUtils::alphatype_name(readAT),
                       rect.fLeft, rect.fTop, rect.fRight, rect.fBottom, csConversion, x, y,
                       diffs[0], diffs[1], diffs[2], diffs[3]);
            });
            SkAutoPixmapStorage ref;
            SkImageInfo refInfo = readInfo.makeDimensions(dstWriteRect.size());
            ref.alloc(refInfo);
            if (readAT == kUnknown_SkAlphaType) {
                // Do a spoofed read where src and dst alpha type are both kUnpremul. This will
                // allow SkPixmap readPixels to succeed and won't do any alpha type conversion.
                SkPixmap unpremulRef(refInfo.makeAlphaType(kUnpremul_SkAlphaType),
                                     ref.addr(),
                                     ref.rowBytes());
                SkPixmap unpremulSRc(srcPixels.info().makeAlphaType(kUnpremul_SkAlphaType),
                                     srcPixels.addr(),
                                     srcPixels.rowBytes());

                unpremulSRc.readPixels(unpremulRef, srcReadRect.x(), srcReadRect.y());
            } else {
                srcPixels.readPixels(ref, srcReadRect.x(), srcReadRect.y());
            }
            // This is the part of dstPixels that should have been updated.
            SkPixmap actual;
            SkAssertResult(dstPixels.extractSubset(&actual, dstWriteRect));
            ComparePixels(ref, actual, tols, error);

            const auto* v = dstData.get();
            const auto* end = dstData.get() + dstSize;
            guardOk = std::all_of(v, v + dstWriteRect.top() * dstPixels.rowBytes(), guardCheck);
            v += dstWriteRect.top() * dstPixels.rowBytes();
            for (int y = dstWriteRect.top(); y < dstWriteRect.bottom(); ++y) {
                guardOk |= std::all_of(v, v + dstWriteRect.left() * readBpp, guardCheck);
                auto pad = v + dstWriteRect.right() * readBpp;
                auto rowEnd = std::min(end, v + dstPixels.rowBytes());
                // min protects against reading past the end of the tight last row.
                guardOk |= std::all_of(pad, rowEnd, guardCheck);
                v = rowEnd;
            }
            guardOk |= std::all_of(v, end, guardCheck);
        } else {
            guardOk = std::all_of(dstData.get(), dstData.get() + dstSize, guardCheck);
        }
        if (!guardOk) {
            ERRORF(reporter,
                   "Result pixels modified result outside read rect [%d, %d, %d, %d]. "
                   "%sSrc CT: %s, Read CT: %s, CS conversion: %d",
                   rect.fLeft, rect.fTop, rect.fRight, rect.fBottom, label.c_str(),
                   ToolUtils::colortype_name(srcCT), ToolUtils::colortype_name(readCT),
                   csConversion);
        }
        return result;
    };

    static constexpr int kW = 16;
    static constexpr int kH = 16;

    const std::vector<SkIRect> longRectArray = make_long_rect_array(kW, kH);
    const std::vector<SkIRect> shortRectArray = make_short_rect_array(kW, kH);

    // We ensure we use the long array once per src and read color type and otherwise use the
    // short array to improve test run time.
    // Also, some color types have no alpha values and thus Opaque Premul and Unpremul are
    // equivalent. Just ensure each redundant AT is tested once with each CT (src and read).
    // Similarly, alpha-only color types behave the same for all alpha types so just test premul
    // after one iter.
    // We consider a src or read CT thoroughly tested once it has run through the long rect array
    // and full complement of alpha types with one successful read in the loop.
    std::array<bool, kLastEnum_SkColorType + 1> srcCTTestedThoroughly  = {},
                                                readCTTestedThoroughly = {};
    for (int sat = 0; sat <= kLastEnum_SkAlphaType; ++sat) {
        const auto srcAT = static_cast<SkAlphaType>(sat);
        if (srcAT == kUnpremul_SkAlphaType && !rules.fAllowUnpremulSrc) {
            continue;
        }
        for (int sct = 0; sct <= kLastEnum_SkColorType; ++sct) {
            const auto srcCT = static_cast<SkColorType>(sct);
            if (rules.fSkipSRGBCT && srcCT == kSRGBA_8888_SkColorType) {
                continue;
            }
            if (rules.fSkip16BitCT &&
                (srcCT == kR16G16_unorm_SkColorType ||
                 srcCT == kR16G16B16A16_unorm_SkColorType)) {
                continue;
            }

            // We always make our ref data as F32
            auto refInfo = SkImageInfo::Make(kW, kH,
                                             kRGBA_F32_SkColorType,
                                             srcAT,
                                             SkColorSpace::MakeSRGB());
            // 1010102 formats have an issue where it's easy to make a resulting
            // color where r, g, or b is greater than a. CPU/GPU differ in whether the stored color
            // channels are clipped to the alpha value. CPU clips but GPU does not.
            // Note that we only currently use srcCT for the 1010102 workaround. If we remove this
            // we can also put the ref data setup above the srcCT loop.
            bool forceOpaque = srcAT == kPremul_SkAlphaType &&
                    (srcCT == kRGBA_1010102_SkColorType || srcCT == kBGRA_1010102_SkColorType);

            SkAutoPixmapStorage refPixels = make_ref_data(refInfo, forceOpaque);
            // Convert the ref data to our desired src color type.
            const auto srcInfo = SkImageInfo::Make(kW, kH, srcCT, srcAT, SkColorSpace::MakeSRGB());
            SkAutoPixmapStorage srcPixels;
            srcPixels.alloc(srcInfo);
            {
                SkPixmap readPixmap = srcPixels;
                // Spoof the alpha type to kUnpremul so the read will succeed without doing any
                // conversion (because we made our surface also use kUnpremul).
                if (srcAT == kUnknown_SkAlphaType) {
                    readPixmap.reset(srcPixels.info().makeAlphaType(kUnpremul_SkAlphaType),
                                     srcPixels.addr(),
                                     srcPixels.rowBytes());
                }
                refPixels.readPixels(readPixmap, 0, 0);
            }

            auto src = srcFactory(srcPixels);
            if (!src) {
                continue;
            }
            if (SkColorTypeIsAlwaysOpaque(srcCT) && srcCTTestedThoroughly[srcCT] &&
                (kPremul_SkAlphaType == srcAT || kUnpremul_SkAlphaType == srcAT)) {
                continue;
            }
            if (SkColorTypeIsAlphaOnly(srcCT) && srcCTTestedThoroughly[srcCT] &&
                (kUnpremul_SkAlphaType == srcAT ||
                 kOpaque_SkAlphaType   == srcAT ||
                 kUnknown_SkAlphaType  == srcAT)) {
                continue;
            }
            for (int rct = 0; rct <= kLastEnum_SkColorType; ++rct) {
                const auto readCT = static_cast<SkColorType>(rct);
                for (const sk_sp<SkColorSpace>& readCS :
                     {SkColorSpace::MakeSRGB(), SkColorSpace::MakeSRGBLinear()}) {
                    for (int at = 0; at <= kLastEnum_SkAlphaType; ++at) {
                        const auto readAT = static_cast<SkAlphaType>(at);
                        if (srcAT != kOpaque_SkAlphaType && readAT == kOpaque_SkAlphaType) {
                            // This doesn't make sense.
                            continue;
                        }
                        if (SkColorTypeIsAlwaysOpaque(readCT) && readCTTestedThoroughly[readCT] &&
                            (kPremul_SkAlphaType == readAT || kUnpremul_SkAlphaType == readAT)) {
                            continue;
                        }
                        if (SkColorTypeIsAlphaOnly(readCT) && readCTTestedThoroughly[readCT] &&
                            (kUnpremul_SkAlphaType == readAT ||
                             kOpaque_SkAlphaType   == readAT ||
                             kUnknown_SkAlphaType  == readAT)) {
                            continue;
                        }
                        const auto& rects =
                                srcCTTestedThoroughly[sct] && readCTTestedThoroughly[rct]
                                        ? shortRectArray
                                        : longRectArray;
                        for (const auto& rect : rects) {
                            const auto readInfo = SkImageInfo::Make(rect.width(), rect.height(),
                                                                    readCT, readAT, readCS);
                            const SkIPoint offset = rect.topLeft();
                            Result r = runTest(src, srcPixels, readInfo, offset);
                            if (r == Result::kSuccess) {
                                srcCTTestedThoroughly[sct] = true;
                                readCTTestedThoroughly[rct] = true;
                            }
                        }
                    }
                }
            }
        }
    }
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(SurfaceContextReadPixels,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    using Surface = std::unique_ptr<skgpu::ganesh::SurfaceContext>;
    GrDirectContext* direct = ctxInfo.directContext();
    auto reader = std::function<GpuReadSrcFn<Surface>>(
            [direct](const Surface& surface, const SkIPoint& offset, const SkPixmap& pixels) {
                if (surface->readPixels(direct, pixels, offset)) {
                    return Result::kSuccess;
                } else {
                    // Reading from a non-renderable format is not guaranteed to work on GL.
                    // We'd have to be able to force a copy or draw to a renderable format.
                    const auto& caps = *direct->priv().caps();
                    if (direct->backend() == GrBackendApi::kOpenGL &&
                        !caps.isFormatRenderable(surface->asSurfaceProxy()->backendFormat(), 1)) {
                        return Result::kExcusedFailure;
                    }
                    return Result::kFail;
                }
            });
    GpuReadPixelTestRules rules;
    rules.fAllowUnpremulSrc = true;
    rules.fUncontainedRectSucceeds = true;

    for (auto renderable : {GrRenderable::kNo, GrRenderable::kYes}) {
        for (GrSurfaceOrigin origin : {kTopLeft_GrSurfaceOrigin, kBottomLeft_GrSurfaceOrigin}) {
            auto factory = std::function<GpuSrcFactory<Surface>>(
                    [direct, origin, renderable](const SkPixmap& src) {
                        auto sc = CreateSurfaceContext(
                                direct, src.info(), SkBackingFit::kExact, origin, renderable);
                        if (sc) {
                            sc->writePixels(direct, src, {0, 0});
                        }
                        return sc;
                    });
            auto label = SkStringPrintf("Renderable: %d, Origin: %d", (int)renderable, origin);
            gpu_read_pixels_test_driver(reporter, rules, factory, reader, label);
        }
    }
}

DEF_GANESH_TEST_FOR_ALL_CONTEXTS(ReadPixels_InvalidRowBytes_Gpu,
                                 reporter,
                                 ctxInfo,
                                 CtsEnforcement::kApiLevel_T) {
    auto srcII = SkImageInfo::Make({10, 10}, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    auto surf = SkSurfaces::RenderTarget(ctxInfo.directContext(), skgpu::Budgeted::kYes, srcII);
    for (int ct = 0; ct < kLastEnum_SkColorType + 1; ++ct) {
        auto colorType = static_cast<SkColorType>(ct);
        size_t bpp = SkColorTypeBytesPerPixel(colorType);
        if (bpp <= 1) {
            continue;
        }
        auto dstII = srcII.makeColorType(colorType);
        size_t badRowBytes = (surf->width() + 1)*bpp - 1;
        auto storage = std::make_unique<char[]>(badRowBytes*surf->height());
        REPORTER_ASSERT(reporter, !surf->readPixels(dstII, storage.get(), badRowBytes, 0, 0));
    }
}

DEF_GANESH_TEST_FOR_ALL_CONTEXTS(WritePixels_InvalidRowBytes_Gpu,
                                 reporter,
                                 ctxInfo,
                                 CtsEnforcement::kApiLevel_T) {
    auto dstII = SkImageInfo::Make({10, 10}, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    auto surf = SkSurfaces::RenderTarget(ctxInfo.directContext(), skgpu::Budgeted::kYes, dstII);
    for (int ct = 0; ct < kLastEnum_SkColorType + 1; ++ct) {
        auto colorType = static_cast<SkColorType>(ct);
        size_t bpp = SkColorTypeBytesPerPixel(colorType);
        if (bpp <= 1) {
            continue;
        }
        auto srcII = dstII.makeColorType(colorType);
        size_t badRowBytes = (surf->width() + 1)*bpp - 1;
        auto storage = std::make_unique<char[]>(badRowBytes*surf->height());
        memset(storage.get(), 0, badRowBytes * surf->height());
        // SkSurface::writePixels doesn't report bool, SkCanvas's does.
        REPORTER_ASSERT(reporter,
                        !surf->getCanvas()->writePixels(srcII, storage.get(), badRowBytes, 0, 0));
    }
}

namespace {
struct AsyncContext {
    bool fCalled = false;
    std::unique_ptr<const SkImage::AsyncReadResult> fResult;
};
}  // anonymous namespace

// Making this a lambda in the test functions caused:
//   "error: cannot compile this forwarded non-trivially copyable parameter yet"
// on x86/Win/Clang bot, referring to 'result'.
static void async_callback(void* c, std::unique_ptr<const SkImage::AsyncReadResult> result) {
    auto context = static_cast<AsyncContext*>(c);
    context->fResult = std::move(result);
    context->fCalled = true;
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(SurfaceAsyncReadPixels,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_V) {
    using Surface = sk_sp<SkSurface>;
    auto reader = std::function<GpuReadSrcFn<Surface>>(
            [](const Surface& surface, const SkIPoint& offset, const SkPixmap& pixels) {
                auto direct = surface->recordingContext()->asDirectContext();
                SkASSERT(direct);

                AsyncContext context;
                auto rect = SkIRect::MakeSize(pixels.dimensions()).makeOffset(offset);

                // Rescale quality and linearity don't matter since we're doing a non-scaling
                // readback.
                surface->asyncRescaleAndReadPixels(pixels.info(), rect,
                                                   SkImage::RescaleGamma::kSrc,
                                                   SkImage::RescaleMode::kNearest,
                                                   async_callback, &context);
                direct->submit();
                while (!context.fCalled) {
                    direct->checkAsyncWorkCompletion();
                }
                if (!context.fResult) {
                    return Result::kFail;
                }
                SkRectMemcpy(pixels.writable_addr(), pixels.rowBytes(), context.fResult->data(0),
                             context.fResult->rowBytes(0), pixels.info().minRowBytes(),
                             pixels.height());
                return Result::kSuccess;
            });
    GpuReadPixelTestRules rules;
    rules.fAllowUnpremulSrc = false;
    rules.fUncontainedRectSucceeds = false;
    // TODO: some mobile GPUs have issues reading back sRGB src data with GLES -- skip for now
    // b/296440036
    if (ctxInfo.type() == skgpu::ContextType::kGLES) {
        rules.fSkipSRGBCT = true;
    }

    for (GrSurfaceOrigin origin : {kTopLeft_GrSurfaceOrigin, kBottomLeft_GrSurfaceOrigin}) {
        auto factory = std::function<GpuSrcFactory<Surface>>(
                [context = ctxInfo.directContext(), origin](const SkPixmap& src) {
                    auto surf = SkSurfaces::RenderTarget(
                            context, skgpu::Budgeted::kYes, src.info(), 1, origin, nullptr);
                    if (surf) {
                        surf->writePixels(src, 0, 0);
                    }
                    return surf;
                });
        auto label = SkStringPrintf("Origin: %d", origin);
        gpu_read_pixels_test_driver(reporter, rules, factory, reader, label);
        auto backendRTFactory = std::function<GpuSrcFactory<Surface>>(
                [context = ctxInfo.directContext(), origin](const SkPixmap& src) {
                    auto surf = sk_gpu_test::MakeBackendRenderTargetSurface(context,
                                                                            src.info(),
                                                                            origin,
                                                                            1);
                    if (surf) {
                        surf->writePixels(src, 0, 0);
                    }
                    return surf;
                });
        label = SkStringPrintf("BERT Origin: %d", origin);
        gpu_read_pixels_test_driver(reporter, rules, backendRTFactory, reader, label);
    }
}

// Manually parameterized by GrRenderable and GrSurfaceOrigin to reduce per-test run time.
static void image_async_read_pixels(GrRenderable renderable,
                                    GrSurfaceOrigin origin,
                                    skiatest::Reporter* reporter,
                                    const sk_gpu_test::ContextInfo& ctxInfo) {
    using Image = sk_sp<SkImage>;
    auto context = ctxInfo.directContext();
    auto reader = std::function<GpuReadSrcFn<Image>>([context](const Image& image,
                                                               const SkIPoint& offset,
                                                               const SkPixmap& pixels) {
        AsyncContext asyncContext;
        auto rect = SkIRect::MakeSize(pixels.dimensions()).makeOffset(offset);
        // The GPU implementation is based on rendering and will fail for non-renderable color
        // types.
        auto ct = SkColorTypeToGrColorType(image->colorType());
        auto format = context->priv().caps()->getDefaultBackendFormat(ct, GrRenderable::kYes);
        if (!context->priv().caps()->isFormatAsColorTypeRenderable(ct, format)) {
            return Result::kExcusedFailure;
        }

        // Rescale quality and linearity don't matter since we're doing a non-scaling readback.
        image->asyncRescaleAndReadPixels(pixels.info(), rect,
                                         SkImage::RescaleGamma::kSrc,
                                         SkImage::RescaleMode::kNearest,
                                         async_callback, &asyncContext);
        context->submit();
        while (!asyncContext.fCalled) {
            context->checkAsyncWorkCompletion();
        }
        if (!asyncContext.fResult) {
            return Result::kFail;
        }
        SkRectMemcpy(pixels.writable_addr(), pixels.rowBytes(), asyncContext.fResult->data(0),
                     asyncContext.fResult->rowBytes(0), pixels.info().minRowBytes(),
                     pixels.height());
        return Result::kSuccess;
    });

    GpuReadPixelTestRules rules;
    rules.fAllowUnpremulSrc = true;
    rules.fUncontainedRectSucceeds = false;
    // TODO: some mobile GPUs have issues reading back sRGB src data with GLES -- skip for now
    // b/296440036
    if (ctxInfo.type() == skgpu::ContextType::kGLES) {
        rules.fSkipSRGBCT = true;
    }
    // TODO: D3D on Intel has issues reading back 16-bit src data -- skip for now
    // b/296440036
    if (ctxInfo.type() == skgpu::ContextType::kDirect3D) {
        rules.fSkip16BitCT = true;
    }

    auto factory = std::function<GpuSrcFactory<Image>>([&](const SkPixmap& src) {
        return sk_gpu_test::MakeBackendTextureImage(ctxInfo.directContext(), src,
                                                    renderable, origin,
                                                    GrProtected::kNo);
    });
    auto label = SkStringPrintf("Renderable: %d, Origin: %d", (int)renderable, origin);
    gpu_read_pixels_test_driver(reporter, rules, factory, reader, label);
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(ImageAsyncReadPixels_NonRenderable_TopLeft,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    image_async_read_pixels(GrRenderable::kNo, GrSurfaceOrigin::kTopLeft_GrSurfaceOrigin,
                            reporter, ctxInfo);
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(ImageAsyncReadPixels_NonRenderable_BottomLeft,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    image_async_read_pixels(GrRenderable::kNo, GrSurfaceOrigin::kBottomLeft_GrSurfaceOrigin,
                            reporter, ctxInfo);
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(ImageAsyncReadPixels_Renderable_TopLeft,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    image_async_read_pixels(GrRenderable::kYes, GrSurfaceOrigin::kTopLeft_GrSurfaceOrigin,
                            reporter, ctxInfo);
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(ImageAsyncReadPixels_Renderable_BottomLeft,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    image_async_read_pixels(GrRenderable::kYes, GrSurfaceOrigin::kBottomLeft_GrSurfaceOrigin,
                            reporter, ctxInfo);
}

DEF_GANESH_TEST(AsyncReadPixelsContextShutdown, reporter, options, CtsEnforcement::kApiLevel_T) {
    const auto ii = SkImageInfo::Make(10, 10, kRGBA_8888_SkColorType, kPremul_SkAlphaType,
                                      SkColorSpace::MakeSRGB());
    enum class ShutdownSequence {
        kFreeResult_DestroyContext,
        kDestroyContext_FreeResult,
        kFreeResult_ReleaseAndAbandon_DestroyContext,
        kFreeResult_Abandon_DestroyContext,
        kReleaseAndAbandon_FreeResult_DestroyContext,
        kAbandon_FreeResult_DestroyContext,
        kReleaseAndAbandon_DestroyContext_FreeResult,
        kAbandon_DestroyContext_FreeResult,
    };
    for (int t = 0; t < skgpu::kContextTypeCount; ++t) {
        auto type = static_cast<skgpu::ContextType>(t);
        for (auto sequence : {ShutdownSequence::kFreeResult_DestroyContext,
                              ShutdownSequence::kDestroyContext_FreeResult,
                              ShutdownSequence::kFreeResult_ReleaseAndAbandon_DestroyContext,
                              ShutdownSequence::kFreeResult_Abandon_DestroyContext,
                              ShutdownSequence::kReleaseAndAbandon_FreeResult_DestroyContext,
                              ShutdownSequence::kAbandon_FreeResult_DestroyContext,
                              ShutdownSequence::kReleaseAndAbandon_DestroyContext_FreeResult,
                              ShutdownSequence::kAbandon_DestroyContext_FreeResult}) {
            // Vulkan and D3D context abandoning without resource release has issues outside of the
            // scope of this test.
            if ((type == skgpu::ContextType::kVulkan || type == skgpu::ContextType::kDirect3D) &&
                (sequence == ShutdownSequence::kFreeResult_ReleaseAndAbandon_DestroyContext ||
                 sequence == ShutdownSequence::kFreeResult_Abandon_DestroyContext ||
                 sequence == ShutdownSequence::kReleaseAndAbandon_FreeResult_DestroyContext ||
                 sequence == ShutdownSequence::kReleaseAndAbandon_DestroyContext_FreeResult ||
                 sequence == ShutdownSequence::kAbandon_FreeResult_DestroyContext ||
                 sequence == ShutdownSequence::kAbandon_DestroyContext_FreeResult)) {
                continue;
            }
            enum class ReadType {
                kRGBA,
                kYUV,
                kYUVA
            };
            for (ReadType readType : {ReadType::kRGBA, ReadType::kYUV, ReadType::kYUVA}) {
                sk_gpu_test::GrContextFactory factory(options);
                auto direct = factory.get(type);
                if (!direct) {
                    continue;
                }
                // This test is only meaningful for contexts that support transfer buffers for
                // reads.
                if (!direct->priv().caps()->transferFromSurfaceToBufferSupport()) {
                    continue;
                }
                auto surf = SkSurfaces::RenderTarget(direct, skgpu::Budgeted::kYes, ii, 1, nullptr);
                if (!surf) {
                    continue;
                }
                AsyncContext cbContext;
                switch (readType) {
                    case ReadType::kRGBA:
                        surf->asyncRescaleAndReadPixels(ii, ii.bounds(),
                                                        SkImage::RescaleGamma::kSrc,
                                                        SkImage::RescaleMode::kNearest,
                                                        &async_callback, &cbContext);
                        break;
                    case ReadType::kYUV:
                        surf->asyncRescaleAndReadPixelsYUV420(
                                kIdentity_SkYUVColorSpace, SkColorSpace::MakeSRGB(), ii.bounds(),
                                ii.dimensions(), SkImage::RescaleGamma::kSrc,
                                SkImage::RescaleMode::kNearest, &async_callback, &cbContext);
                        break;
                    case ReadType::kYUVA:
                        surf->asyncRescaleAndReadPixelsYUVA420(
                                kIdentity_SkYUVColorSpace, SkColorSpace::MakeSRGB(), ii.bounds(),
                                ii.dimensions(), SkImage::RescaleGamma::kSrc,
                                SkImage::RescaleMode::kNearest, &async_callback, &cbContext);
                        break;
                }

                direct->submit();
                while (!cbContext.fCalled) {
                    direct->checkAsyncWorkCompletion();
                }
                if (!cbContext.fResult) {
                    const char* readTypeStr;
                    switch (readType) {
                        case ReadType::kRGBA: readTypeStr = "rgba"; break;
                        case ReadType::kYUV:  readTypeStr = "yuv";  break;
                        case ReadType::kYUVA: readTypeStr = "yuva"; break;
                    }
                    ERRORF(reporter, "Callback failed on %s. read type is: %s",
                           skgpu::ContextTypeName(type), readTypeStr);
                    continue;
                }
                // For vulkan we need to release all refs to the GrDirectContext before trying to
                // destroy the test context. The surface here is holding a ref.
                surf.reset();

                // The real test is that we don't crash, get Vulkan validation errors, etc, during
                // this shutdown sequence.
                switch (sequence) {
                    case ShutdownSequence::kFreeResult_DestroyContext:
                    case ShutdownSequence::kFreeResult_ReleaseAndAbandon_DestroyContext:
                    case ShutdownSequence::kFreeResult_Abandon_DestroyContext:
                        break;
                    case ShutdownSequence::kDestroyContext_FreeResult:
                        factory.destroyContexts();
                        break;
                    case ShutdownSequence::kReleaseAndAbandon_FreeResult_DestroyContext:
                        factory.releaseResourcesAndAbandonContexts();
                        break;
                    case ShutdownSequence::kAbandon_FreeResult_DestroyContext:
                        factory.abandonContexts();
                        break;
                    case ShutdownSequence::kReleaseAndAbandon_DestroyContext_FreeResult:
                        factory.releaseResourcesAndAbandonContexts();
                        factory.destroyContexts();
                        break;
                    case ShutdownSequence::kAbandon_DestroyContext_FreeResult:
                        factory.abandonContexts();
                        factory.destroyContexts();
                        break;
                }
                cbContext.fResult.reset();
                switch (sequence) {
                    case ShutdownSequence::kFreeResult_ReleaseAndAbandon_DestroyContext:
                        factory.releaseResourcesAndAbandonContexts();
                        break;
                    case ShutdownSequence::kFreeResult_Abandon_DestroyContext:
                        factory.abandonContexts();
                        break;
                    case ShutdownSequence::kFreeResult_DestroyContext:
                    case ShutdownSequence::kDestroyContext_FreeResult:
                    case ShutdownSequence::kReleaseAndAbandon_FreeResult_DestroyContext:
                    case ShutdownSequence::kAbandon_FreeResult_DestroyContext:
                    case ShutdownSequence::kReleaseAndAbandon_DestroyContext_FreeResult:
                    case ShutdownSequence::kAbandon_DestroyContext_FreeResult:
                        break;
                }
            }
        }
    }
}

template <typename T>
static void gpu_write_pixels_test_driver(skiatest::Reporter* reporter,
                                         const std::function<GpuDstFactory<T>>& dstFactory,
                                         const std::function<GpuWriteDstFn<T>>& write,
                                         const std::function<GpuReadDstFn<T>>& read) {
    // Separate this out just to give it some line width to breathe.
    auto runTest = [&](const T& dst,
                       const SkImageInfo& dstInfo,
                       const SkPixmap& srcPixels,
                       SkIPoint offset) {
        const bool csConversion =
                !SkColorSpace::Equals(dstInfo.colorSpace(), srcPixels.info().colorSpace());
        const auto writeCT = srcPixels.colorType();
        const auto writeAT = srcPixels.alphaType();
        const auto dstCT = dstInfo.colorType();
        const auto dstAT = dstInfo.alphaType();
        const auto rect = SkIRect::MakePtSize(offset, srcPixels.dimensions());
        const auto surfBounds = SkIRect::MakeSize(dstInfo.dimensions());

        // Do an initial read before the write.
        SkAutoPixmapStorage firstReadPM = read(dst);
        if (!firstReadPM.addr()) {
            // Particularly with GLES 2 we can have formats that are unreadable with our current
            // implementation of read pixels. If the format can't be attached to a FBO we don't have
            // a code path that draws it to another readable color type/format combo and reads from
            // that.
            return Result::kExcusedFailure;
        }

        const Result result = write(dst, offset, srcPixels);

        if (!SkIRect::Intersects(rect, surfBounds)) {
            REPORTER_ASSERT(reporter, result != Result::kSuccess);
        } else if (writeCT == kUnknown_SkColorType) {
            REPORTER_ASSERT(reporter, result != Result::kSuccess);
        } else if ((writeAT == kUnknown_SkAlphaType) != (dstAT == kUnknown_SkAlphaType)) {
            REPORTER_ASSERT(reporter, result != Result::kSuccess);
        } else if (result == Result::kExcusedFailure) {
            return result;
        } else if (result == Result::kFail) {
            // TODO: Support RGB/BGR 101010x, BGRA 1010102 on the GPU.
            if (SkColorTypeToGrColorType(writeCT) != GrColorType::kUnknown) {
                ERRORF(reporter,
                       "Write failed. Write CT: %s, Write AT: %s Dst CT: %s, Dst AT: %s, "
                       "Rect [%d, %d, %d, %d], CS conversion: %d\n",
                       ToolUtils::colortype_name(writeCT), ToolUtils::alphatype_name(writeAT),
                       ToolUtils::colortype_name(dstCT), ToolUtils::alphatype_name(dstAT),
                       rect.fLeft, rect.fTop, rect.fRight, rect.fBottom, csConversion);
            }
            return result;
        }

        SkIRect checkRect;
        if (result != Result::kSuccess || !checkRect.intersect(surfBounds, rect)) {
            return result;
        }

        // Do an initial read before the write. We'll use this to verify that areas outside the
        // write are unaffected.
        SkAutoPixmapStorage secondReadPM = read(dst);
        if (!secondReadPM.addr()) {
            // The first read succeeded so this one should, too.
            ERRORF(reporter,
                   "could not read from dst (CT: %s, AT: %s)\n",
                   ToolUtils::colortype_name(dstCT),
                   ToolUtils::alphatype_name(dstAT));
            return Result::kFail;
        }

        // Sometimes wider types go through 8bit unorm intermediates because of API
        // restrictions.
        int rgbBits = std::min({min_rgb_channel_bits(writeCT), min_rgb_channel_bits(dstCT), 8});
        float tol = 2.f/(1 << rgbBits);
        float alphaTol = 0;
        if (writeAT != kOpaque_SkAlphaType && dstAT != kOpaque_SkAlphaType) {
            // Alpha can also get squashed down to 8 bits going through an intermediate
            // color format.
            const int alphaBits = std::min({alpha_channel_bits(writeCT),
                                            alpha_channel_bits(dstCT),
                                            8});
            alphaTol = 2.f/(1 << alphaBits);
        }

        const float tols[4] = {tol, tol, tol, alphaTol};
        auto error = std::function<ComparePixmapsErrorReporter>([&](int x,
                                                                    int y,
                                                                    const float diffs[4]) {
            SkASSERT(x >= 0 && y >= 0);
            ERRORF(reporter,
                   "Write CT: %s, Write AT: %s, Dst CT: %s, Dst AT: %s, Rect [%d, %d, %d, %d]"
                   ", CS conversion: %d\n"
                   "Error at %d, %d. Diff in floats: (%f, %f, %f, %f)",
                   ToolUtils::colortype_name(writeCT),
                   ToolUtils::alphatype_name(writeAT),
                   ToolUtils::colortype_name(dstCT),
                   ToolUtils::alphatype_name(dstAT),
                   rect.fLeft,
                   rect.fTop,
                   rect.fRight,
                   rect.fBottom,
                   csConversion,
                   x,
                   y,
                   diffs[0],
                   diffs[1],
                   diffs[2],
                   diffs[3]);
        });

        SkAutoPixmapStorage ref;
        ref.alloc(secondReadPM.info().makeDimensions(checkRect.size()));
        // Here we use the CPU backend to do the equivalent conversion as the write we're
        // testing, using kUnpremul instead of kUnknown since CPU requires a valid alpha type.
        SkAssertResult(make_pixmap_have_valid_alpha_type(srcPixels).readPixels(
                make_pixmap_have_valid_alpha_type(ref),
                std::max(0, -offset.fX),
                std::max(0, -offset.fY)));
        // This is the part of secondReadPixels that should have been updated by the write.
        SkPixmap actual;
        SkAssertResult(secondReadPM.extractSubset(&actual, checkRect));
        ComparePixels(ref, actual, tols, error);
        // The area around written rect should be the same in the first and second read.
        SkIRect borders[]{
                {               0,                 0, secondReadPM.width(), secondReadPM.height()},
                {checkRect.fRight,                 0,      checkRect.fLeft, secondReadPM.height()},
                { checkRect.fLeft,                 0,     checkRect.fRight,        checkRect.fTop},
                { checkRect.fLeft, checkRect.fBottom,     checkRect.fRight, secondReadPM.height()}
        };
        for (const auto r : borders) {
            if (!r.isEmpty()) {
                // Make a copy because MSVC for some reason doesn't correctly capture 'r'.
                SkIPoint tl = r.topLeft();
                auto guardError = std::function<ComparePixmapsErrorReporter>(
                        [&](int x, int y, const float diffs[4]) {
                            x += tl.x();
                            y += tl.y();
                            ERRORF(reporter,
                                   "Write CT: %s, Write AT: %s, Dst CT: %s, Dst AT: %s,"
                                   "Rect [%d, %d, %d, %d], CS conversion: %d\n"
                                   "Error in guard region %d, %d. Diff in floats: (%f, %f, %f, %f)",
                                   ToolUtils::colortype_name(writeCT),
                                   ToolUtils::alphatype_name(writeAT),
                                   ToolUtils::colortype_name(dstCT),
                                   ToolUtils::alphatype_name(dstAT),
                                   rect.fLeft,
                                   rect.fTop,
                                   rect.fRight,
                                   rect.fBottom,
                                   csConversion,
                                   x,
                                   y,
                                   diffs[0],
                                   diffs[1],
                                   diffs[2],
                                   diffs[3]);
                        });
                SkPixmap a, b;
                SkAssertResult(firstReadPM.extractSubset(&a, r));
                SkAssertResult(firstReadPM.extractSubset(&b, r));
                float zeroTols[4] = {};
                ComparePixels(a, b, zeroTols, guardError);
            }
        }
        return result;
    };

    static constexpr int kW = 16;
    static constexpr int kH = 16;

    const std::vector<SkIRect> longRectArray = make_long_rect_array(kW, kH);
    const std::vector<SkIRect> shortRectArray = make_short_rect_array(kW, kH);

    // We ensure we use the long array once per src and read color type and otherwise use the
    // short array to improve test run time.
    // Also, some color types have no alpha values and thus Opaque Premul and Unpremul are
    // equivalent. Just ensure each redundant AT is tested once with each CT (dst and write).
    // Similarly, alpha-only color types behave the same for all alpha types so just test premul
    // after one iter.
    // We consider a dst or write CT thoroughly tested once it has run through the long rect array
    // and full complement of alpha types with one successful read in the loop.
    std::array<bool, kLastEnum_SkColorType + 1> dstCTTestedThoroughly   = {},
                                                writeCTTestedThoroughly = {};
    for (int dat = 0; dat < kLastEnum_SkAlphaType; ++dat) {
        const auto dstAT = static_cast<SkAlphaType>(dat);
        for (int dct = 0; dct <= kLastEnum_SkColorType; ++dct) {
            const auto dstCT = static_cast<SkColorType>(dct);
            const auto dstInfo = SkImageInfo::Make(kW, kH, dstCT, dstAT, SkColorSpace::MakeSRGB());
            auto dst = dstFactory(dstInfo);
            if (!dst) {
                continue;
            }
            if (SkColorTypeIsAlwaysOpaque(dstCT) && dstCTTestedThoroughly[dstCT] &&
                (kPremul_SkAlphaType == dstAT || kUnpremul_SkAlphaType == dstAT)) {
                continue;
            }
            if (SkColorTypeIsAlphaOnly(dstCT) && dstCTTestedThoroughly[dstCT] &&
                (kUnpremul_SkAlphaType == dstAT ||
                 kOpaque_SkAlphaType   == dstAT ||
                 kUnknown_SkAlphaType  == dstAT)) {
                continue;
            }
            for (int wct = 0; wct <= kLastEnum_SkColorType; ++wct) {
                const auto writeCT = static_cast<SkColorType>(wct);
                for (const sk_sp<SkColorSpace>& writeCS : {SkColorSpace::MakeSRGB(),
                                                           SkColorSpace::MakeSRGBLinear()}) {
                    for (int wat = 0; wat <= kLastEnum_SkAlphaType; ++wat) {
                        const auto writeAT = static_cast<SkAlphaType>(wat);
                        if (writeAT != kOpaque_SkAlphaType && dstAT == kOpaque_SkAlphaType) {
                            // This doesn't make sense.
                            continue;
                        }
                        if (SkColorTypeIsAlwaysOpaque(writeCT) &&
                            writeCTTestedThoroughly[writeCT] &&
                            (kPremul_SkAlphaType == writeAT || kUnpremul_SkAlphaType == writeAT)) {
                            continue;
                        }
                        if (SkColorTypeIsAlphaOnly(writeCT) && writeCTTestedThoroughly[writeCT] &&
                            (kUnpremul_SkAlphaType == writeAT ||
                             kOpaque_SkAlphaType   == writeAT ||
                             kUnknown_SkAlphaType  == writeAT)) {
                            continue;
                        }
                        const auto& rects =
                                dstCTTestedThoroughly[dct] && writeCTTestedThoroughly[wct]
                                        ? shortRectArray
                                        : longRectArray;
                        for (const auto& rect : rects) {
                            auto writeInfo = SkImageInfo::Make(rect.size(),
                                                               writeCT,
                                                               writeAT,
                                                               writeCS);
                            // CPU and GPU handle 1010102 differently. CPU clamps RGB to A, GPU
                            // doesn't.
                            bool forceOpaque = writeCT == kRGBA_1010102_SkColorType ||
                                               writeCT == kBGRA_1010102_SkColorType;
                            SkAutoPixmapStorage writePixels = make_ref_data(writeInfo, forceOpaque);
                            const SkIPoint offset = rect.topLeft();
                            Result r = runTest(dst, dstInfo, writePixels, offset);
                            if (r == Result::kSuccess) {
                                dstCTTestedThoroughly[dct] = true;
                                writeCTTestedThoroughly[wct] = true;
                            }
                        }
                    }
                }
            }
        }
    }
}

// Manually parameterized by GrRenderable and GrSurfaceOrigin to reduce per-test run time.
static void surface_context_write_pixels(GrRenderable renderable,
                                         GrSurfaceOrigin origin,
                                         skiatest::Reporter* reporter,
                                         const sk_gpu_test::ContextInfo& ctxInfo) {
    using Surface = std::unique_ptr<skgpu::ganesh::SurfaceContext>;
    GrDirectContext* direct = ctxInfo.directContext();
    auto writer = std::function<GpuWriteDstFn<Surface>>(
            [direct](const Surface& surface, const SkIPoint& offset, const SkPixmap& pixels) {
                if (surface->writePixels(direct, pixels, offset)) {
                    return Result::kSuccess;
                } else {
                    return Result::kFail;
                }
            });
    auto reader = std::function<GpuReadDstFn<Surface>>([direct](const Surface& s) {
        SkAutoPixmapStorage result;
        auto grInfo = s->imageInfo();
        SkColorType ct = GrColorTypeToSkColorType(grInfo.colorType());
        SkASSERT(ct != kUnknown_SkColorType);
        auto skInfo = SkImageInfo::Make(grInfo.dimensions(), ct, grInfo.alphaType(),
                                        grInfo.refColorSpace());
        result.alloc(skInfo);
        if (!s->readPixels(direct, result, {0, 0})) {
            SkAutoPixmapStorage badResult;
            return badResult;
        }
        return result;
    });

    auto factory = std::function<GpuDstFactory<Surface>>(
            [direct, origin, renderable](const SkImageInfo& info) {
                return CreateSurfaceContext(direct,
                                            info,
                                            SkBackingFit::kExact,
                                            origin,
                                            renderable);
            });

    gpu_write_pixels_test_driver(reporter, factory, writer, reader);
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(SurfaceContextWritePixels_NonRenderable_TopLeft,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    surface_context_write_pixels(GrRenderable::kNo, GrSurfaceOrigin::kTopLeft_GrSurfaceOrigin,
                                 reporter, ctxInfo);
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(SurfaceContextWritePixels_NonRenderable_BottomLeft,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    surface_context_write_pixels(GrRenderable::kNo, GrSurfaceOrigin::kBottomLeft_GrSurfaceOrigin,
                                 reporter, ctxInfo);
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(SurfaceContextWritePixels_Renderable_TopLeft,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    surface_context_write_pixels(GrRenderable::kYes, GrSurfaceOrigin::kTopLeft_GrSurfaceOrigin,
                                 reporter, ctxInfo);
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(SurfaceContextWritePixels_Renderable_BottomLeft,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    surface_context_write_pixels(GrRenderable::kYes, GrSurfaceOrigin::kBottomLeft_GrSurfaceOrigin,
                                 reporter, ctxInfo);
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(SurfaceContextWritePixelsMipped,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    auto direct = ctxInfo.directContext();
    if (!direct->priv().caps()->mipmapSupport()) {
        return;
    }
    static constexpr int kW = 25,
                         kH = 37;
    SkAutoPixmapStorage refP = make_ref_data(SkImageInfo::Make({kW, kH},
                                                               kRGBA_F32_SkColorType,
                                                               kPremul_SkAlphaType,
                                                               nullptr),
                                             false);
    SkAutoPixmapStorage refO = make_ref_data(SkImageInfo::Make({kW, kH},
                                                               kRGBA_F32_SkColorType,
                                                               kOpaque_SkAlphaType,
                                                               nullptr),
                                             true);

    for (int c = 0; c < kGrColorTypeCnt; ++c) {
        auto ct = static_cast<GrColorType>(c);
        // Below we use rendering to read the level pixels back.
        auto format = direct->priv().caps()->getDefaultBackendFormat(ct, GrRenderable::kYes);
        if (!format.isValid()) {
            continue;
        }
        SkAlphaType at = GrColorTypeHasAlpha(ct) ? kPremul_SkAlphaType : kOpaque_SkAlphaType;
        GrImageInfo info(ct, at, nullptr, kW, kH);
        TArray<GrCPixmap> levels;
        const auto& ref = at == kPremul_SkAlphaType ? refP : refO;
        for (int w = kW, h = kH; w || h; w/=2, h/=2) {
            auto level = GrPixmap::Allocate(info.makeWH(std::max(w, 1), std::max(h, 1)));
            SkPixmap src;
            SkAssertResult(ref.extractSubset(&src, SkIRect::MakeSize(level.dimensions())));
            SkAssertResult(GrConvertPixels(level, src));
            levels.push_back(level);
        }

        for (bool unowned : {false, true}) { // test a GrCPixmap that doesn't own its storage.
            for (auto renderable : {GrRenderable::kNo, GrRenderable::kYes}) {
                for (GrSurfaceOrigin origin : {kTopLeft_GrSurfaceOrigin,
                                               kBottomLeft_GrSurfaceOrigin}) {
                    auto sc = CreateSurfaceContext(direct,
                                                   info,
                                                   SkBackingFit::kExact,
                                                   origin,
                                                   renderable,
                                                   /*sample count*/ 1,
                                                   skgpu::Mipmapped::kYes);
                    if (!sc) {
                        continue;
                    }
                    // Keeps pixels in unowned case alive until after writePixels is called but no
                    // longer.
                    GrPixmap keepAlive;
                    GrCPixmap savedLevel = levels[1];
                    if (unowned) {
                        // Also test non-tight row bytes with the unowned pixmap, bump width by 1.
                        int w = levels[1].width() + 1;
                        int h = levels[1].height();
                        keepAlive = GrPixmap::Allocate(levels[1].info().makeWH(w, h));
                        SkPixmap src;
                        // These pixel values will be the same as the original level 1.
                        SkAssertResult(ref.extractSubset(&src, SkIRect::MakeWH(w, h)));
                        SkAssertResult(GrConvertPixels(keepAlive, src));
                        levels[1] = GrCPixmap(levels[1].info(),
                                              keepAlive.addr(),
                                              keepAlive.rowBytes());
                    }
                    // Going through intermediate textures is not supported for MIP levels (because
                    // we don't support rendering to non-base levels). So it's hard to have any hard
                    // rules about when we expect success.
                    if (!sc->writePixels(direct, levels.begin(), levels.size())) {
                        continue;
                    }
                    // Make sure the pixels from the unowned pixmap are released and then put the
                    // original level back in for the comparison after the read below.
                    keepAlive = {};
                    levels[1] = savedLevel;

                    // TODO: Update this when read pixels supports reading back levels to read
                    // directly rather than using minimizing draws.
                    auto dstSC = CreateSurfaceContext(direct,
                                                      info,
                                                      SkBackingFit::kExact,
                                                      kBottomLeft_GrSurfaceOrigin,
                                                      GrRenderable::kYes);
                    SkASSERT(dstSC);
                    GrSamplerState sampler(SkFilterMode::kNearest, SkMipmapMode::kNearest);
                    for (int i = 1; i <= 1; ++i) {
                        auto te = GrTextureEffect::Make(sc->readSurfaceView(),
                                                        info.alphaType(),
                                                        SkMatrix::I(),
                                                        sampler,
                                                        *direct->priv().caps());
                        dstSC->asFillContext()->fillRectToRectWithFP(
                                SkIRect::MakeSize(sc->dimensions()),
                                SkIRect::MakeSize(levels[i].dimensions()),
                                std::move(te));
                        GrImageInfo readInfo =
                                dstSC->imageInfo().makeDimensions(levels[i].dimensions());
                        GrPixmap read = GrPixmap::Allocate(readInfo);
                        if (!dstSC->readPixels(direct, read, {0, 0})) {
                            continue;
                        }

                        auto skCT = GrColorTypeToSkColorType(info.colorType());
                        int rgbBits = std::min(min_rgb_channel_bits(skCT), 8);
                        float rgbTol = (rgbBits == 0) ? 1.f : 2.f / ((1 << rgbBits) - 1);
                        int alphaBits = std::min(alpha_channel_bits(skCT), 8);
                        float alphaTol = (alphaBits == 0) ? 1.f : 2.f / ((1 << alphaBits) - 1);
                        float tol[] = {rgbTol, rgbTol, rgbTol, alphaTol};

                        GrCPixmap a = levels[i];
                        GrCPixmap b = read;
                        // The compare code will linearize when reading the srgb data. This will
                        // magnify differences at the high end. Rather than adjusting the tolerance
                        // to compensate we do the comparison without going through srgb->linear.
                        if (ct == GrColorType::kRGBA_8888_SRGB) {
                            a = GrCPixmap(a.info().makeColorType(GrColorType::kRGBA_8888),
                                          a.addr(),
                                          a.rowBytes());
                            b = GrCPixmap(b.info().makeColorType(GrColorType::kRGBA_8888),
                                          b.addr(),
                                          b.rowBytes());
                        }

                        auto error = std::function<ComparePixmapsErrorReporter>(
                                [&](int x, int y, const float diffs[4]) {
                                    SkASSERT(x >= 0 && y >= 0);
                                    ERRORF(reporter,
                                           "CT: %s, Level %d, Unowned: %d. "
                                           "Error at %d, %d. Diff in floats:"
                                           "(%f, %f, %f, %f)",
                                           GrColorTypeToStr(info.colorType()), i, unowned, x, y,
                                           diffs[0], diffs[1], diffs[2], diffs[3]);
                                });
                        ComparePixels(a, b, tol, error);
                    }
                }
            }
        }
    }
}

// Tests a bug found in OOP-R canvas2d in Chrome. The GPU backend would incorrectly not bind
// buffer 0 to GL_PIXEL_PACK_BUFFER before a glReadPixels() that was supposed to read into
// client memory if a GrDirectContext::resetContext() occurred.
DEF_GANESH_TEST_FOR_GL_CONTEXT(GLReadPixelsUnbindPBO,
                               reporter,
                               ctxInfo,
                               CtsEnforcement::kApiLevel_T) {
    // Start with a async read so that we bind to GL_PIXEL_PACK_BUFFER.
    auto info = SkImageInfo::Make(16, 16, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    SkAutoPixmapStorage pmap = make_ref_data(info, /*forceOpaque=*/false);
    auto image = SkImages::RasterFromPixmap(pmap, nullptr, nullptr);
    image = SkImages::TextureFromImage(ctxInfo.directContext(), image);
    if (!image) {
        ERRORF(reporter, "Couldn't make texture image.");
        return;
    }

    AsyncContext asyncContext;
    image->asyncRescaleAndReadPixels(info,
                                     SkIRect::MakeSize(info.dimensions()),
                                     SkImage::RescaleGamma::kSrc,
                                     SkImage::RescaleMode::kNearest,
                                     async_callback,
                                     &asyncContext);

    // This will force the async readback to finish.
    ctxInfo.directContext()->flushAndSubmit(GrSyncCpu::kYes);
    if (!asyncContext.fCalled) {
        ERRORF(reporter, "async_callback not called.");
    }
    if (!asyncContext.fResult) {
        ERRORF(reporter, "async read failed.");
    }

    SkPixmap asyncResult(info, asyncContext.fResult->data(0), asyncContext.fResult->rowBytes(0));

    // Bug was that this would cause GrGLGpu to think no buffer was left bound to
    // GL_PIXEL_PACK_BUFFER even though async transfer did leave one bound. So the sync read
    // wouldn't bind buffer 0.
    ctxInfo.directContext()->resetContext();

    SkBitmap syncResult;
    syncResult.allocPixels(info);
    syncResult.eraseARGB(0xFF, 0xFF, 0xFF, 0xFF);

    image->readPixels(ctxInfo.directContext(), syncResult.pixmap(), 0, 0);

    float tol[4] = {};  // expect exactly same pixels, no conversions.
    auto error = std::function<ComparePixmapsErrorReporter>([&](int x, int y,
                                                                const float diffs[4]) {
      SkASSERT(x >= 0 && y >= 0);
      ERRORF(reporter, "Expect sync and async read to be the same. "
             "Error at %d, %d. Diff in floats: (%f, %f, %f, %f)",
             x, y, diffs[0], diffs[1], diffs[2], diffs[3]);
    });

    ComparePixels(syncResult.pixmap(), asyncResult, tol, error);
}
