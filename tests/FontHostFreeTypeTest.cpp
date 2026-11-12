/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColor.h"
#include "include/core/SkData.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/private/SkFixed.h"
#include "include/private/SkTo.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkMask.h"
#include "src/core/SkMaskGamma.h"
#include "src/core/SkScalerContext.h"
#include "src/core/SkStrikeSpec.h"
#include "src/ports/SkFontHost_FreeType_common.h"
#include "tests/Test.h"
#include "tools/Resources.h"
#include "tools/fonts/TestEmptyTypeface.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

#include <ft2build.h>

#include <freetype/freetype.h>
#include <freetype/ftoutln.h>

namespace {

// Creates an SkGlyph locked to caller-specified bounds and mask format, simulating the cached
// metrics from an initial glyph load before a subsequent FreeType load diverges.
SkGlyph make_test_glyph(SkRect bounds,
                        SkMask::Format format,
                        SkPixelGeometry pixelGeometry,
                        SkPackedGlyphID packedID,
                        SkArenaAlloc* alloc) {
    class FixedBoundsTypeface final : public TestEmptyTypeface {
    public:
        FixedBoundsTypeface(SkRect bounds, SkMask::Format format)
                : fBounds(bounds), fFormat(format) {}

    protected:
        std::unique_ptr<SkScalerContext> onCreateScalerContext(
                const SkScalerContextEffects& effects, const SkDescriptor* desc) const override {
            class ScalerContext final : public SkScalerContext {
            public:
                ScalerContext(FixedBoundsTypeface& tf,
                              const SkScalerContextEffects& effects,
                              const SkDescriptor* desc)
                        : SkScalerContext(tf, effects, desc)
                        , fBounds(tf.fBounds)
                        , fFormat(tf.fFormat) {}

            protected:
                GlyphMetrics generateMetrics(const SkGlyph&, SkArenaAlloc*) override {
                    GlyphMetrics mx(fFormat);
                    mx.neverRequestPath = true;
                    mx.bounds = fBounds;
                    return mx;
                }
                void generateImage(const SkGlyph&, void*) override {}
                std::optional<GeneratedPath> generatePath(const SkGlyph&) override {
                    return std::nullopt;
                }
                void generateFontMetrics(SkFontMetrics*) override {}

            private:
                const SkRect fBounds;
                const SkMask::Format fFormat;
            };
            return std::make_unique<ScalerContext>(
                    *const_cast<FixedBoundsTypeface*>(this), effects, desc);
        }

    private:
        const SkRect fBounds;
        const SkMask::Format fFormat;
    };

    SkFont font(sk_make_sp<FixedBoundsTypeface>(bounds, format), 10.0f);
    font.setEdging(format == SkMask::kLCD16_Format ? SkFont::Edging::kSubpixelAntiAlias
                                                   : SkFont::Edging::kAntiAlias);
    font.setSubpixel(true);
    SkStrikeSpec spec = SkStrikeSpec::MakeMask(font,
                                               SkPaint(),
                                               SkSurfaceProps(0, pixelGeometry),
                                               SkScalerContextFlags::kNone,
                                               SkMatrix::I());
    return spec.createScalerContext()->makeGlyph(packedID, alloc);
}

// Surrounds an image buffer with 64-byte canary regions and pre-fills the payload with 0x5A so
// tests also verify that generateGlyphImage zeroes the destination on rejected/disjoint outlines.
struct GuardedBuffer {
    static constexpr size_t kCanaryBytes = 64;
    static constexpr uint8_t kCanary = 0x5A;

    std::vector<uint8_t> storage;

    explicit GuardedBuffer(size_t size) : storage(size + 2 * kCanaryBytes, kCanary) {}

    uint8_t* data() { return storage.data() + kCanaryBytes; }

    bool canariesIntact() const {
        auto isCanary = [](uint8_t b) { return b == kCanary; };
        return std::all_of(storage.begin(), storage.begin() + kCanaryBytes, isCanary) &&
               std::all_of(storage.end() - kCanaryBytes, storage.end(), isCanary);
    }

    bool hasNonZeroPayload() const {
        return std::any_of(storage.begin() + kCanaryBytes,
                           storage.end() - kCanaryBytes,
                           [](uint8_t b) { return b != 0; });
    }
};

}  // namespace

// Verifies SkScalerContextFTUtils::generateGlyphImage when a FreeType outline diverges from
// SkGlyph's pre-computed bounds:
//   1. kLCD16_Format subpixel-to-pixel intersection and 3x subpixel cropping.
//   2. Non-LCD kA8_Format SkFDot6 subpixel translation overflow guards.
DEF_TEST(FreeType_GenerateGlyphImageDivergentOutline, reporter) {
    // Load the 'X' outline from ahem.ttf at 10px to establish a known 10x10 pixel glyph box.
    sk_sp<SkData> fontData = GetResourceAsData("fonts/ahem.ttf");
    if (!fontData) {
        return;
    }

    FT_Library rawLib = nullptr;
    if (FT_Init_FreeType(&rawLib) != 0) {
        ERRORF(reporter, "FT_Init_FreeType failed");
        return;
    }
    std::unique_ptr<FT_LibraryRec_, decltype(&FT_Done_FreeType)> library(rawLib, FT_Done_FreeType);

    FT_Face rawFace = nullptr;
    if (FT_New_Memory_Face(
                library.get(), fontData->bytes(), SkTo<FT_Long>(fontData->size()), 0, &rawFace) !=
        0) {
        ERRORF(reporter, "FT_New_Memory_Face failed");
        return;
    }
    std::unique_ptr<FT_FaceRec_, decltype(&FT_Done_Face)> face(rawFace, FT_Done_Face);

    REPORTER_ASSERT(reporter, FT_Set_Pixel_Sizes(face.get(), 0, 10) == 0);
    REPORTER_ASSERT(reporter, FT_Load_Char(face.get(), 'X', FT_LOAD_NO_BITMAP) == 0);
    REPORTER_ASSERT(reporter, face->glyph->format == FT_GLYPH_FORMAT_OUTLINE);

    // Negate yMax/yMin to convert FreeType's Y-up 26.6 coordinates into Skia's Y-down pixel space.
    FT_BBox initBox;
    FT_Outline_Get_CBox(&face->glyph->outline, &initBox);
    const SkRect baseBounds = SkRect::MakeLTRB(SkIntToScalar(initBox.xMin >> 6),
                                               SkIntToScalar(-(initBox.yMax >> 6)),
                                               SkIntToScalar(initBox.xMax >> 6),
                                               SkIntToScalar(-(initBox.yMin >> 6)));
    REPORTER_ASSERT(reporter, baseBounds.width() == 10 && baseBounds.height() == 10);

    // --- Part 1: kLCD16_Format subpixel clipping (horizontal 3x1 and vertical 1x3) ---
    // FreeType's LCD renderer scales bitmap width (horizontal) or rows (vertical) by 3x while
    // keeping bitmap_left / bitmap_top in whole pixels. Shifting the outline before rendering
    // forces generateGlyphImage to clip the 3x subpixel bitmap against pixel-space mask.fBounds.
    struct ShiftCase {
        int dxPx;
        int dyPx;
        bool expectNonZero;
    };
    constexpr ShiftCase kShifts[] = {
            {0, 0, true},   // Unshifted baseline: full 10x10 overlap.
            {-4, 0, true},  // Partial left/right/top/bottom shifts: +-4 px is not a multiple of 3,
            {4, 0, true},   // verifying 3x subpixel stride scaling and copyFT2LCD16 alignment.
            {0, -4, true},
            {0, 4, true},
            {-15, 0, false},  // Disjoint shifts: moves the 10x10 glyph outside baseBounds while
            {0, 15, false},  // landing inside the [0..30] false-overlap window of unscaled 3x size.
    };

    for (bool doVert : {false, true}) {
        const SkPixelGeometry geom = doVert ? kRGB_V_SkPixelGeometry : kRGB_H_SkPixelGeometry;
        SkArenaAlloc alloc{256};
        SkGlyph glyph = make_test_glyph(
                baseBounds, SkMask::kLCD16_Format, geom, SkPackedGlyphID((SkGlyphID)1), &alloc);
        REPORTER_ASSERT(reporter, glyph.maskFormat() == SkMask::kLCD16_Format);

        SkScalerContextFTUtils utils{};
        const uint32_t flags = doVert ? SkScalerContext::kLCD_Vertical_Flag : 0;
        utils.init(SK_ColorBLACK, static_cast<SkScalerContext::Flags>(flags));
        SkMaskGamma::PreBlend noPreBlend;

        for (const ShiftCase& tc : kShifts) {
            REPORTER_ASSERT(reporter, FT_Load_Char(face.get(), 'X', FT_LOAD_NO_BITMAP) == 0);
            FT_Outline_Translate(&face->glyph->outline, tc.dxPx * 64, tc.dyPx * 64);

            GuardedBuffer buf(glyph.imageSize());
            utils.generateGlyphImage(face.get(), glyph, buf.data(), SkMatrix::I(), noPreBlend);

            REPORTER_ASSERT(reporter, buf.canariesIntact());
            REPORTER_ASSERT(reporter, buf.hasNonZeroPayload() == tc.expectNonZero);
        }
    }

    // --- Part 2: Non-LCD SkFDot6 subpixel translation overflow guards ---
    // With (0.5, 0.5) subpixel positioning, generateGlyphImage applies dx = +32 and dy = -32 (in
    // 26.6 fixed-point) and snaps the shifted bounding box to the pixel grid via xShift / yShift.
    // Each row places outline coordinates near INT32 limits to test a distinct overflow guard.
    struct OverflowCase {
        int32_t point0X;
        int32_t restX;
        int32_t allY;
    };
    constexpr OverflowCase kOverflows[] = {
            // xMin fits in SkFDot6, but xMin + dx (+32) > SK_MaxS32.
            {SK_MaxS32 - 16, SK_MaxS32 - 16, 0},
            // yMin fits in SkFDot6, but yMin + dy (-32) < SK_MinS32.
            {0, 0, SK_MinS32 + 16},
            // xMin + dx fits in SkFDot6, but xShift (dx - SK_MinS32) > SK_MaxS32.
            {SK_MinS32, SK_MinS32, 0},
            // xMin, xMax, and xShift each fit in SkFDot6, but xMax + xShift > SK_MaxS32.
            {-1500000000, 1500000000, 0},
    };

    SkArenaAlloc alloc{256};
    SkGlyph subpixelGlyph =
            make_test_glyph(baseBounds,
                            SkMask::kA8_Format,
                            kUnknown_SkPixelGeometry,
                            SkPackedGlyphID((SkGlyphID)1, SK_FixedHalf, SK_FixedHalf),
                            &alloc);
    REPORTER_ASSERT(reporter, subpixelGlyph.maskFormat() == SkMask::kA8_Format);

    SkScalerContextFTUtils utils{};
    utils.init(SK_ColorBLACK, SkScalerContext::kSubpixelPositioning_Flag);
    SkMaskGamma::PreBlend noPreBlend;

    for (const OverflowCase& oc : kOverflows) {
        REPORTER_ASSERT(reporter, FT_Load_Char(face.get(), 'X', FT_LOAD_NO_BITMAP) == 0);
        REPORTER_ASSERT(reporter, face->glyph->outline.n_points >= 2);
        for (int i = 0; i < face->glyph->outline.n_points; ++i) {
            face->glyph->outline.points[i].x = (i == 0) ? oc.point0X : oc.restX;
            face->glyph->outline.points[i].y = oc.allY;
        }

        GuardedBuffer buf(subpixelGlyph.imageSize());
        utils.generateGlyphImage(face.get(), subpixelGlyph, buf.data(), SkMatrix::I(), noPreBlend);

        REPORTER_ASSERT(reporter, buf.canariesIntact());
        REPORTER_ASSERT(reporter, !buf.hasNonZeroPayload());
    }
}
