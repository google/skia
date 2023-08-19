/*
 * Copyright 2022 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkGradientShader.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Image.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "include/gpu/graphite/Surface.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "src/base/SkRectMemcpy.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/core/SkImageInfoPriv.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/ResourceTypes.h"
#include "tests/Test.h"
#include "tests/TestUtils.h"
#include "tools/ToolUtils.h"

using Mipmapped = skgpu::Mipmapped;

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
        case kGray_8_SkColorType:             return 0;
        case kRGBA_F16Norm_SkColorType:       return 10;  // just counting the mantissa
        case kRGBA_F16_SkColorType:           return 10;  // just counting the mantissa
        case kRGBA_F32_SkColorType:           return 23;  // just counting the mantissa
        case kR16G16B16A16_unorm_SkColorType: return 16;
        case kR8_unorm_SkColorType:           return 0;
    }
    SkUNREACHABLE;
}

namespace {
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

struct GraphiteReadPixelTestRules {
    // Test unpremul sources? We could omit this and detect that creating the source of the read
    // failed but having it lets us skip generating reference color data.
    bool fAllowUnpremulSrc = true;
    // Are reads that are overlapping but not contained by the src bounds expected to succeed?
    bool fUncontainedRectSucceeds = true;
};

// Makes a src populated with the pixmap. The src should get its image info (or equivalent) from
// the pixmap.
template <typename T> using GraphiteSrcFactory = T(skgpu::graphite::Recorder*, SkPixmap&);

enum class Result {
    kFail,
    kSuccess,
    kExcusedFailure,
};

// Does a read from the T into the pixmap.
template <typename T>
using GraphiteReadSrcFn = Result(const T&, const SkIPoint& offset, const SkPixmap&);

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
};
}  // anonymous namespace

template <typename T>
static void graphite_read_pixels_test_driver(skiatest::Reporter* reporter,
                                             skgpu::graphite::Context* context,
                                             const GraphiteReadPixelTestRules& rules,
                                             const std::function<GraphiteSrcFactory<T>>& srcFactory,
                                             const std::function<GraphiteReadSrcFn<T>>& read,
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
        } else if (readAT == kUnknown_SkAlphaType) {
            REPORTER_ASSERT(reporter, result != Result::kSuccess);
        } else if (!rules.fUncontainedRectSucceeds && !surfBounds.contains(rect)) {
            REPORTER_ASSERT(reporter, result != Result::kSuccess);
        } else if (result == Result::kFail) {
            // TODO: Support RGB/BGR 101010x, BGRA 1010102 on the GPU.
            ERRORF(reporter,
                   "Read failed. %sSrc CT: %s, Src AT: %s Read CT: %s, Read AT: %s, "
                   "Rect [%d, %d, %d, %d], CS conversion: %d\n",
                   label.c_str(),
                   ToolUtils::colortype_name(srcCT), ToolUtils::alphatype_name(srcAT),
                   ToolUtils::colortype_name(readCT), ToolUtils::alphatype_name(readAT),
                   rect.fLeft, rect.fTop, rect.fRight, rect.fBottom, csConversion);
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
            float alphaTol = 0;
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
                SkPixmap unpremulSrc(srcPixels.info().makeAlphaType(kUnpremul_SkAlphaType),
                                     srcPixels.addr(),
                                     srcPixels.rowBytes());

                unpremulSrc.readPixels(unpremulRef, srcReadRect.x(), srcReadRect.y());
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

            std::unique_ptr<skgpu::graphite::Recorder> recorder = context->makeRecorder();

            auto src = srcFactory(recorder.get(), srcPixels);
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
                // ComparePixels will end up converting these types to kUnknown
                // because there's no corresponding GrColorType, and hence it will fail
                if (readCT == kRGB_101010x_SkColorType ||
                    readCT == kBGR_101010x_XR_SkColorType ||
                    readCT == kBGR_101010x_SkColorType) {
                    continue;
                }
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
};

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(ImageAsyncReadPixelsGraphite,
                                         reporter,
                                         context,
                                         CtsEnforcement::kNextRelease) {
    using Image = sk_sp<SkImage>;
    using Renderable = skgpu::Renderable;
    using TextureInfo = skgpu::graphite::TextureInfo;

    auto reader = std::function<GraphiteReadSrcFn<Image>>([context](const Image& image,
                                                                    const SkIPoint& offset,
                                                                    const SkPixmap& pixels) {
        AsyncContext asyncContext;
        auto rect = SkIRect::MakeSize(pixels.dimensions()).makeOffset(offset);
        // The GPU implementation is based on rendering and will fail for non-renderable color
        // types.
        TextureInfo texInfo = context->priv().caps()->getDefaultSampledTextureInfo(
                image->colorType(),
                Mipmapped::kNo,
                skgpu::Protected::kNo,
                Renderable::kYes);
        if (!context->priv().caps()->isRenderable(texInfo)) {
            return Result::kExcusedFailure;
        }

        context->asyncRescaleAndReadPixels(image.get(),
                                           pixels.info(),
                                           rect,
                                           SkImage::RescaleGamma::kSrc,
                                           SkImage::RescaleMode::kRepeatedLinear,
                                           async_callback,
                                           &asyncContext);
        if (!asyncContext.fCalled) {
            context->submit();
        }
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

    GraphiteReadPixelTestRules rules;
    rules.fAllowUnpremulSrc = true;
    rules.fUncontainedRectSucceeds = false;

    for (auto renderable : {Renderable::kNo, Renderable::kYes}) {
        auto factory = std::function<GraphiteSrcFactory<Image>>([&](
                skgpu::graphite::Recorder* recorder,
                const SkPixmap& src) {
            // TODO: put this in the equivalent of sk_gpu_test::MakeBackendTextureImage
            TextureInfo info = recorder->priv().caps()->getDefaultSampledTextureInfo(
                    src.colorType(),
                    Mipmapped::kNo,
                    skgpu::Protected::kNo,
                    renderable);
            auto texture = recorder->createBackendTexture(src.dimensions(), info);
            if (!recorder->updateBackendTexture(texture, &src, 1)) {
                return (Image)(nullptr);
            }

            Image image = SkImages::AdoptTextureFrom(recorder,
                                                     texture,
                                                     src.colorType(),
                                                     src.alphaType(),
                                                     /*colorSpace=*/nullptr);

            std::unique_ptr<skgpu::graphite::Recording> recording = recorder->snap();
            skgpu::graphite::InsertRecordingInfo recordingInfo;
            recordingInfo.fRecording = recording.get();
            context->insertRecording(recordingInfo);

            return image;
        });
        auto label = SkStringPrintf("Renderable: %d", (int)renderable);
        graphite_read_pixels_test_driver(reporter, context, rules, factory, reader, label);
    }

    // It's possible that we've created an Image using the factory, but then don't try to do
    // readPixels on it, leaving a hanging command buffer. So we submit here to clean up.
    context->submit();
}

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(SurfaceAsyncReadPixelsGraphite,
                                         reporter,
                                         context,
                                         CtsEnforcement::kNextRelease) {
    using Surface = sk_sp<SkSurface>;

    auto reader = std::function<GraphiteReadSrcFn<Surface>>([context](const Surface& surface,
                                                                      const SkIPoint& offset,
                                                                      const SkPixmap& pixels) {
        AsyncContext asyncContext;
        auto rect = SkIRect::MakeSize(pixels.dimensions()).makeOffset(offset);

        context->asyncRescaleAndReadPixels(surface.get(),
                                           pixels.info(),
                                           rect,
                                           SkImage::RescaleGamma::kSrc,
                                           SkImage::RescaleMode::kRepeatedLinear,
                                           async_callback,
                                           &asyncContext);
        if (!asyncContext.fCalled) {
            context->submit();
        }
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

    GraphiteReadPixelTestRules rules;
    rules.fAllowUnpremulSrc = true;
    rules.fUncontainedRectSucceeds = false;

    auto factory = std::function<GraphiteSrcFactory<Surface>>(
            [&](skgpu::graphite::Recorder* recorder, const SkPixmap& src) {
                Surface surface = SkSurfaces::RenderTarget(recorder,
                                                           src.info(),
                                                           Mipmapped::kNo,
                                                           /*surfaceProps=*/nullptr);
                if (surface) {
                    surface->writePixels(src, 0, 0);

                    std::unique_ptr<skgpu::graphite::Recording> recording = recorder->snap();
                    skgpu::graphite::InsertRecordingInfo recordingInfo;
                    recordingInfo.fRecording = recording.get();
                    context->insertRecording(recordingInfo);
                }

                return surface;
            });
    graphite_read_pixels_test_driver(reporter, context, rules, factory, reader, {});

    // It's possible that we've created an Image using the factory, but then don't try to do
    // readPixels on it, leaving a hanging command buffer. So we submit here to clean up.
    context->submit();
}
