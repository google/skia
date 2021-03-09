/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkGradientShader.h"
#include "include/gpu/GrDirectContext.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/core/SkConvertPixels.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrImageInfo.h"
#include "src/gpu/GrSurfaceContext.h"
#include "tests/Test.h"
#include "tests/TestUtils.h"
#include "tools/ToolUtils.h"
#include "tools/gpu/BackendSurfaceFactory.h"
#include "tools/gpu/BackendTextureImageFactory.h"
#include "tools/gpu/GrContextFactory.h"
#include "tools/gpu/ProxyUtils.h"

#include <initializer_list>

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
        case kRGB_888x_SkColorType:           return 8;
        case kBGRA_8888_SkColorType:          return 8;
        case kRGBA_1010102_SkColorType:       return 10;
        case kRGB_101010x_SkColorType:        return 10;
        case kBGRA_1010102_SkColorType:       return 10;
        case kBGR_101010x_SkColorType:        return 10;
        case kGray_8_SkColorType:             return 8;   // counting gray as "rgb"
        case kRGBA_F16Norm_SkColorType:       return 10;  // just counting the mantissa
        case kRGBA_F16_SkColorType:           return 10;  // just counting the mantissa
        case kRGBA_F32_SkColorType:           return 23;  // just counting the mantissa
        case kR16G16B16A16_unorm_SkColorType: return 16;
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
        case kRGB_888x_SkColorType:           return 0;
        case kBGRA_8888_SkColorType:          return 8;
        case kRGBA_1010102_SkColorType:       return 2;
        case kRGB_101010x_SkColorType:        return 0;
        case kBGRA_1010102_SkColorType:       return 2;
        case kBGR_101010x_SkColorType:        return 0;
        case kGray_8_SkColorType:             return 0;
        case kRGBA_F16Norm_SkColorType:       return 10;  // just counting the mantissa
        case kRGBA_F16_SkColorType:           return 10;  // just counting the mantissa
        case kRGBA_F32_SkColorType:           return 23;  // just counting the mantissa
        case kR16G16B16A16_unorm_SkColorType: return 16;
    }
    SkUNREACHABLE;
}

namespace {

struct GpuReadPixelTestRules {
    // Test unpremul sources? We could omit this and detect that creating the source of the read
    // failed but having it lets us skip generating reference color data.
    bool fAllowUnpremulSrc = true;
    // Are reads that are overlapping but not contained by the src bounds expected to succeed?
    bool fUncontainedRectSucceeds = true;
};

// Makes a src populated with the pixmap. The src should get its image info (or equivalent) from
// the pixmap.
template <typename T> using GpuSrcFactory = T(SkPixmap&);

enum class GpuReadResult {
    kFail,
    kSuccess,
    kExcusedFailure,
};

// Does a read from the T into the pixmap.
template <typename T>
using GpuReadSrcFn = GpuReadResult(const T&, const SkIVector& offset, const SkPixmap&);

}  // anonymous namespace

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
                       const SkIVector& offset) {
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

        const GpuReadResult result = read(src, offset, dstPixels);

        if (!SkIRect::Intersects(rect, surfBounds)) {
            REPORTER_ASSERT(reporter, result != GpuReadResult::kSuccess);
        } else if (readCT == kUnknown_SkColorType) {
            REPORTER_ASSERT(reporter, result != GpuReadResult::kSuccess);
        } else if ((readAT == kUnknown_SkAlphaType) != (srcAT == kUnknown_SkAlphaType)) {
            REPORTER_ASSERT(reporter, result != GpuReadResult::kSuccess);
        } else if (!rules.fUncontainedRectSucceeds && !surfBounds.contains(rect)) {
            REPORTER_ASSERT(reporter, result != GpuReadResult::kSuccess);
        } else if (result == GpuReadResult::kFail) {
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
        if (result == GpuReadResult::kSuccess && srcReadRect.intersect(surfBounds, rect)) {
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
                       "Error at %d, %d. Diff in floats: (%f, %f, %f %f)",
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

    // Makes the reference data that is used to populate the src. Always F32 regardless of srcCT.
    auto make_ref_f32_data = [](SkAlphaType srcAT, SkColorType srcCT) -> SkAutoPixmapStorage {
        // Make src data in F32 with srcAT. We will convert it to each color type we test to
        // initialize the src.
        auto surfInfo = SkImageInfo::Make(kW, kH,
                                          kRGBA_F32_SkColorType,
                                          srcAT,
                                          SkColorSpace::MakeSRGB());
        // Can't make a kUnknown_SkAlphaType surface.
        if (srcAT == kUnknown_SkAlphaType) {
            surfInfo = surfInfo.makeAlphaType(kUnpremul_SkAlphaType);
        }
        auto refSurf = SkSurface::MakeRaster(surfInfo);
        static constexpr SkPoint kPts1[] = {{0, 0}, {kW, kH}};
        static constexpr SkColor kColors1[] = {SK_ColorGREEN, SK_ColorRED};
        SkPaint paint;
        paint.setShader(
                SkGradientShader::MakeLinear(kPts1, kColors1, nullptr, 2, SkTileMode::kClamp));
        refSurf->getCanvas()->drawPaint(paint);
        static constexpr SkPoint kPts2[] = {{kW, 0}, {0, kH}};
        static constexpr SkColor kColors2[] = {SK_ColorBLUE, SK_ColorBLACK};
        paint.setShader(
                SkGradientShader::MakeLinear(kPts2, kColors2, nullptr, 2, SkTileMode::kClamp));
        paint.setBlendMode(SkBlendMode::kPlus);
        refSurf->getCanvas()->drawPaint(paint);
        // Keep everything opaque if the src alpha type is opaque. Also, there is an issue with
        // 1010102 (the only color type where the number of alpha bits is non-zero and not the
        // same as r, g, and b). Because of the different precisions the draw below can create
        // data that isn't strictly premul (e.g. alpha is 1/3 but green is .4). SW will clamp
        // r, g, b to a if the dst is premul and a different color type. GPU doesn't do this.
        // We could but 1010102 premul is kind of dubious anyway. So for now just keep the data
        // opaque.
        if (srcAT != kOpaque_SkAlphaType &&
            (srcAT == kPremul_SkAlphaType && srcCT != kRGBA_1010102_SkColorType &&
             srcCT != kBGRA_1010102_SkColorType)) {
            static constexpr SkColor kColors3[] = {SK_ColorWHITE,
                                                   SK_ColorWHITE,
                                                   0x60FFFFFF,
                                                   SK_ColorWHITE,
                                                   SK_ColorWHITE};
            static constexpr SkScalar kPos3[] = {0.f, 0.15f, 0.5f, 0.85f, 1.f};
            paint.setShader(SkGradientShader::MakeRadial({kW / 2.f, kH / 2.f}, (kW + kH) / 10.f,
                                                         kColors3, kPos3, 5, SkTileMode::kMirror));
            paint.setBlendMode(SkBlendMode::kDstIn);
            refSurf->getCanvas()->drawPaint(paint);
        }

        const auto srcInfo = SkImageInfo::Make(kW, kH, srcCT, srcAT, SkColorSpace::MakeSRGB());
        SkAutoPixmapStorage srcPixels;
        srcPixels.alloc(srcInfo);
        SkPixmap readPixmap = srcPixels;
        // Spoof the alpha type to kUnpremul so the read will succeed without doing any conversion
        // (because we made our surface also be kUnpremul).
        if (srcAT == kUnknown_SkAlphaType) {
            readPixmap.reset(srcPixels.info().makeAlphaType(kUnpremul_SkAlphaType),
                             srcPixels.addr(),
                             srcPixels.rowBytes());
        }
        refSurf->readPixels(readPixmap, 0, 0);
        return srcPixels;
    };
    const std::vector<SkIRect> longRectArray = {
            // entire thing
            SkIRect::MakeWH(kW, kH),
            // larger on all sides
            SkIRect::MakeLTRB(-10, -10, kW + 10, kH + 10),
            // fully contained
            SkIRect::MakeLTRB(kW / 4, kH / 4, 3 * kW / 4, 3 * kH / 4),
            // outside top left
            SkIRect::MakeLTRB(-10, -10, -1, -1),
            // touching top left corner
            SkIRect::MakeLTRB(-10, -10, 0, 0),
            // overlapping top left corner
            SkIRect::MakeLTRB(-10, -10, kW / 4, kH / 4),
            // overlapping top left and top right corners
            SkIRect::MakeLTRB(-10, -10, kW + 10, kH / 4),
            // touching entire top edge
            SkIRect::MakeLTRB(-10, -10, kW + 10, 0),
            // overlapping top right corner
            SkIRect::MakeLTRB(3 * kW / 4, -10, kW + 10, kH / 4),
            // contained in x, overlapping top edge
            SkIRect::MakeLTRB(kW / 4, -10, 3 * kW / 4, kH / 4),
            // outside top right corner
            SkIRect::MakeLTRB(kW + 1, -10, kW + 10, -1),
            // touching top right corner
            SkIRect::MakeLTRB(kW, -10, kW + 10, 0),
            // overlapping top left and bottom left corners
            SkIRect::MakeLTRB(-10, -10, kW / 4, kH + 10),
            // touching entire left edge
            SkIRect::MakeLTRB(-10, -10, 0, kH + 10),
            // overlapping bottom left corner
            SkIRect::MakeLTRB(-10, 3 * kH / 4, kW / 4, kH + 10),
            // contained in y, overlapping left edge
            SkIRect::MakeLTRB(-10, kH / 4, kW / 4, 3 * kH / 4),
            // outside bottom left corner
            SkIRect::MakeLTRB(-10, kH + 1, -1, kH + 10),
            // touching bottom left corner
            SkIRect::MakeLTRB(-10, kH, 0, kH + 10),
            // overlapping bottom left and bottom right corners
            SkIRect::MakeLTRB(-10, 3 * kH / 4, kW + 10, kH + 10),
            // touching entire left edge
            SkIRect::MakeLTRB(0, kH, kW, kH + 10),
            // overlapping bottom right corner
            SkIRect::MakeLTRB(3 * kW / 4, 3 * kH / 4, kW + 10, kH + 10),
            // overlapping top right and bottom right corners
            SkIRect::MakeLTRB(3 * kW / 4, -10, kW + 10, kH + 10),
    };
    const std::vector<SkIRect> shortRectArray = {
            // entire thing
            SkIRect::MakeWH(kW, kH),
            // fully contained
            SkIRect::MakeLTRB(kW / 4, kH / 4, 3 * kW / 4, 3 * kH / 4),
            // overlapping top right corner
            SkIRect::MakeLTRB(3 * kW / 4, -10, kW + 10, kH / 4),
    };
    // We ensure we use the long array once per src and read color type and otherwise use the
    // short array to improve test run time.
    // Also, some color types have no alpha values and thus Opaque Premul and Unpremul are
    // equivalent. Just ensure each redundant AT is tested once with each CT (src and read).
    // Similarly, alpha-only color types behave the same for all alpha types so just test premul
    // after one iter.
    // We consider a src or read CT thoroughly tested once it has run through the short rect array
    // and full complement of alpha types with one successful read in the loop.
    std::array<bool, kLastEnum_SkColorType + 1> srcCTTestedThoroughly  = {},
                                                readCTTestedThoroughly = {};
    for (int sat = 0; sat < kLastEnum_SkAlphaType; ++sat) {
        const auto srcAT = static_cast<SkAlphaType>(sat);
        if (srcAT == kUnpremul_SkAlphaType && !rules.fAllowUnpremulSrc) {
            continue;
        }
        for (int sct = 0; sct <= kLastEnum_SkColorType; ++sct) {
            const auto srcCT = static_cast<SkColorType>(sct);
            // Note that we only currently use srcCT for a 1010102 workaround. If we remove this we
            // can also put the ref data setup above the srcCT loop.
            SkAutoPixmapStorage srcPixels = make_ref_f32_data(srcAT, srcCT);
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
                            const SkIVector offset = rect.topLeft();
                            GpuReadResult r = runTest(src, srcPixels, readInfo, offset);
                            if (r == GpuReadResult::kSuccess) {
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

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SurfaceContextReadPixels, reporter, ctxInfo) {
    using Surface = std::unique_ptr<GrSurfaceContext>;
    GrDirectContext* direct = ctxInfo.directContext();
    auto reader = std::function<GpuReadSrcFn<Surface>>(
            [direct](const Surface& surface, const SkIVector& offset, const SkPixmap& pixels) {
                if (surface->readPixels(direct, pixels, {offset.fX, offset.fY})) {
                    return GpuReadResult::kSuccess;
                } else {
                    // Reading from a non-renderable format is not guaranteed to work on GL.
                    // We'd have to be able to force a copy or draw draw to a renderable format.
                    const auto& caps = *direct->priv().caps();
                    if (direct->backend() == GrBackendApi::kOpenGL &&
                        !caps.isFormatRenderable(surface->asSurfaceProxy()->backendFormat(), 1)) {
                        return GpuReadResult::kExcusedFailure;
                    }
                    return GpuReadResult::kFail;
                }
            });
    GpuReadPixelTestRules rules;
    rules.fAllowUnpremulSrc = true;
    rules.fUncontainedRectSucceeds = true;

    for (auto renderable : {GrRenderable::kNo, GrRenderable::kYes}) {
        for (GrSurfaceOrigin origin : {kTopLeft_GrSurfaceOrigin, kBottomLeft_GrSurfaceOrigin}) {
            auto factory = std::function<GpuSrcFactory<Surface>>(
                    [direct, origin, renderable](const SkPixmap& src) {
                        if (src.colorType() == kRGB_888x_SkColorType) {
                            return Surface();
                        }
                        auto surfContext = GrSurfaceContext::Make(
                                direct, src.info(), SkBackingFit::kExact, origin, renderable);
                        if (surfContext) {
                            surfContext->writePixels(direct, src, {0, 0});
                        }
                        return surfContext;
                    });
            auto label = SkStringPrintf("Renderable: %d, Origin: %d", (int)renderable, origin);
            gpu_read_pixels_test_driver(reporter, rules, factory, reader, label);
        }
    }
}

DEF_GPUTEST_FOR_ALL_CONTEXTS(ReadPixels_InvalidRowBytes_Gpu, reporter, ctxInfo) {
    auto srcII = SkImageInfo::Make({10, 10}, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    auto surf = SkSurface::MakeRenderTarget(ctxInfo.directContext(), SkBudgeted::kYes, srcII);
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

DEF_GPUTEST_FOR_ALL_CONTEXTS(WritePixels_InvalidRowBytes_Gpu, reporter, ctxInfo) {
    auto dstII = SkImageInfo::Make({10, 10}, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    auto surf = SkSurface::MakeRenderTarget(ctxInfo.directContext(), SkBudgeted::kYes, dstII);
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
};

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SurfaceAsyncReadPixels, reporter, ctxInfo) {
    using Surface = sk_sp<SkSurface>;
    auto reader = std::function<GpuReadSrcFn<Surface>>(
            [](const Surface& surface, const SkIVector& offset, const SkPixmap& pixels) {
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
                    return GpuReadResult::kFail;
                }
                SkRectMemcpy(pixels.writable_addr(), pixels.rowBytes(), context.fResult->data(0),
                             context.fResult->rowBytes(0), pixels.info().minRowBytes(),
                             pixels.height());
                return GpuReadResult::kSuccess;
            });
    GpuReadPixelTestRules rules;
    rules.fAllowUnpremulSrc = false;
    rules.fUncontainedRectSucceeds = false;

    for (GrSurfaceOrigin origin : {kTopLeft_GrSurfaceOrigin, kBottomLeft_GrSurfaceOrigin}) {
        auto factory = std::function<GpuSrcFactory<Surface>>(
                [context = ctxInfo.directContext(), origin](const SkPixmap& src) {
                    if (src.colorType() == kRGB_888x_SkColorType) {
                        return Surface();
                    }
                    auto surf = SkSurface::MakeRenderTarget(context,
                                                            SkBudgeted::kYes,
                                                            src.info(),
                                                            1,
                                                            origin,
                                                            nullptr);
                    if (surf) {
                        surf->writePixels(src, 0, 0);
                    }
                    return surf;
                });
        auto label = SkStringPrintf("Origin: %d", origin);
        gpu_read_pixels_test_driver(reporter, rules, factory, reader, label);
        auto backendRTFactory = std::function<GpuSrcFactory<Surface>>(
                [context = ctxInfo.directContext(), origin](const SkPixmap& src) {
                  if (src.colorType() == kRGB_888x_SkColorType) {
                      return Surface();
                  }
                  // Dawn backend implementation of backend render targets doesn't support reading.
                  if (context->backend() == GrBackendApi::kDawn) {
                      return Surface();
                  }
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

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ImageAsyncReadPixels, reporter, ctxInfo) {
    using Image = sk_sp<SkImage>;
    auto context = ctxInfo.directContext();
    auto reader = std::function<GpuReadSrcFn<Image>>([context](const Image& image,
                                                               const SkIVector& offset,
                                                               const SkPixmap& pixels) {
        AsyncContext asyncContext;
        auto rect = SkIRect::MakeSize(pixels.dimensions()).makeOffset(offset);
        // The GPU implementation is based on rendering and will fail for non-renderable color
        // types.
        auto ct = SkColorTypeToGrColorType(image->colorType());
        auto format = context->priv().caps()->getDefaultBackendFormat(ct, GrRenderable::kYes);
        if (!context->priv().caps()->isFormatAsColorTypeRenderable(ct, format)) {
            return GpuReadResult::kExcusedFailure;
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
            return GpuReadResult::kFail;
        }
        SkRectMemcpy(pixels.writable_addr(), pixels.rowBytes(), asyncContext.fResult->data(0),
                     asyncContext.fResult->rowBytes(0), pixels.info().minRowBytes(),
                     pixels.height());
        return GpuReadResult::kSuccess;
    });

    GpuReadPixelTestRules rules;
    rules.fAllowUnpremulSrc = true;
    rules.fUncontainedRectSucceeds = false;

    for (auto origin : {kTopLeft_GrSurfaceOrigin, kBottomLeft_GrSurfaceOrigin}) {
        for (auto renderable : {GrRenderable::kNo, GrRenderable::kYes}) {
            auto factory = std::function<GpuSrcFactory<Image>>([&](const SkPixmap& src) {
                if (src.colorType() == kRGB_888x_SkColorType) {
                    return Image();
                }
                return sk_gpu_test::MakeBackendTextureImage(ctxInfo.directContext(), src,
                                                            renderable, origin);
            });
            auto label = SkStringPrintf("Renderable: %d, Origin: %d", (int)renderable, origin);
            gpu_read_pixels_test_driver(reporter, rules, factory, reader, label);
        }
    }
}

DEF_GPUTEST(AsyncReadPixelsContextShutdown, reporter, options) {
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
    for (int t = 0; t < sk_gpu_test::GrContextFactory::kContextTypeCnt; ++t) {
        auto type = static_cast<sk_gpu_test::GrContextFactory::ContextType>(t);
        for (auto sequence : {ShutdownSequence::kFreeResult_DestroyContext,
                              ShutdownSequence::kDestroyContext_FreeResult,
                              ShutdownSequence::kFreeResult_ReleaseAndAbandon_DestroyContext,
                              ShutdownSequence::kFreeResult_Abandon_DestroyContext,
                              ShutdownSequence::kReleaseAndAbandon_FreeResult_DestroyContext,
                              ShutdownSequence::kAbandon_FreeResult_DestroyContext,
                              ShutdownSequence::kReleaseAndAbandon_DestroyContext_FreeResult,
                              ShutdownSequence::kAbandon_DestroyContext_FreeResult}) {
            // Vulkan context abandoning without resource release has issues outside of the scope of
            // this test.
            if (type == sk_gpu_test::GrContextFactory::kVulkan_ContextType &&
                (sequence == ShutdownSequence::kFreeResult_ReleaseAndAbandon_DestroyContext ||
                 sequence == ShutdownSequence::kFreeResult_Abandon_DestroyContext ||
                 sequence == ShutdownSequence::kReleaseAndAbandon_FreeResult_DestroyContext ||
                 sequence == ShutdownSequence::kReleaseAndAbandon_DestroyContext_FreeResult ||
                 sequence == ShutdownSequence::kAbandon_FreeResult_DestroyContext ||
                 sequence == ShutdownSequence::kAbandon_DestroyContext_FreeResult)) {
                continue;
            }
            for (bool yuv : {false, true}) {
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
                auto surf = SkSurface::MakeRenderTarget(direct, SkBudgeted::kYes, ii, 1, nullptr);
                if (!surf) {
                    continue;
                }
                AsyncContext cbContext;
                if (yuv) {
                    surf->asyncRescaleAndReadPixelsYUV420(
                            kIdentity_SkYUVColorSpace, SkColorSpace::MakeSRGB(), ii.bounds(),
                            ii.dimensions(), SkImage::RescaleGamma::kSrc,
                            SkImage::RescaleMode::kNearest, &async_callback, &cbContext);
                } else {
                    surf->asyncRescaleAndReadPixels(ii, ii.bounds(), SkImage::RescaleGamma::kSrc,
                                                    SkImage::RescaleMode::kNearest, &async_callback,
                                                    &cbContext);
                }
                direct->submit();
                while (!cbContext.fCalled) {
                    direct->checkAsyncWorkCompletion();
                }
                if (!cbContext.fResult) {
                    ERRORF(reporter, "Callback failed on %s. is YUV: %d",
                           sk_gpu_test::GrContextFactory::ContextTypeName(type), yuv);
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
