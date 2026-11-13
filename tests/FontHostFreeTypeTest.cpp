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
#include "include/core/SkString.h"
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
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <type_traits>
#include <vector>

#include <ft2build.h>

#include <freetype/freetype.h>
#include <freetype/ftimage.h>
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
    if (format == SkMask::kLCD16_Format) {
        font.setEdging(SkFont::Edging::kSubpixelAntiAlias);
        font.setSubpixel(true);
    } else if (format == SkMask::kBW_Format) {
        font.setEdging(SkFont::Edging::kAlias);
        font.setSubpixel(false);
    } else {
        font.setEdging(SkFont::Edging::kAntiAlias);
        font.setSubpixel(true);
    }
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
//   2. Non-LCD (kA8_Format and kBW_Format) target-space clipping and disjoint rejection.
//   3. SkFDot6 subpixel translation overflow guards across LCD and non-LCD formats.
//   4. Monochrome (kBW_Format) FT_OUTLINE_HIGH_PRECISION Bezier coordinate bounds
//      (b/576952569).
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

    // --- Part 2: Non-LCD (kA8_Format and kBW_Format) target-space clipping ---
    // Shifting the outline relative to glyph.iRect() should clip partial overlaps into the
    // destination mask and reject disjoint outlines rather than snapping shiftedXMin/YMin to 0.
    for (SkMask::Format fmt : {SkMask::kA8_Format, SkMask::kBW_Format}) {
        SkArenaAlloc alloc{256};
        SkGlyph glyph = make_test_glyph(
                baseBounds, fmt, kUnknown_SkPixelGeometry, SkPackedGlyphID((SkGlyphID)1), &alloc);
        REPORTER_ASSERT(reporter, glyph.maskFormat() == fmt);

        SkScalerContextFTUtils utils{};
        utils.init(SK_ColorBLACK, static_cast<SkScalerContext::Flags>(0));
        SkMaskGamma::PreBlend noPreBlend;

        for (const ShiftCase& tc : kShifts) {
            skiatest::ReporterContext ctx(
                    reporter,
                    SkStringPrintf(
                            "fmt %d shift (%d, %d)", static_cast<int>(fmt), tc.dxPx, tc.dyPx));
            REPORTER_ASSERT(reporter, FT_Load_Char(face.get(), 'X', FT_LOAD_NO_BITMAP) == 0);
            FT_Outline_Translate(&face->glyph->outline, tc.dxPx * 64, tc.dyPx * 64);

            GuardedBuffer buf(glyph.imageSize());
            utils.generateGlyphImage(face.get(), glyph, buf.data(), SkMatrix::I(), noPreBlend);

            REPORTER_ASSERT(reporter, buf.canariesIntact());
            REPORTER_ASSERT(reporter, buf.hasNonZeroPayload() == tc.expectNonZero);
            for (int y = 0; y < glyph.height(); ++y) {
                for (int x = 0; x < glyph.width(); ++x) {
                    // FreeType Y goes up, so +dyPx moves the square toward smaller mask rows.
                    const bool inside = x - tc.dxPx >= 0 && x - tc.dxPx < 10 && y + tc.dyPx >= 0 &&
                                        y + tc.dyPx < 10;
                    const uint8_t* row = buf.data() + y * glyph.rowBytes();
                    const int value = fmt == SkMask::kBW_Format
                                              ? ((row[x >> 3] >> (7 - (x & 7))) & 1) * 0xFF
                                              : row[x];
                    REPORTER_ASSERT(reporter,
                                    value == (inside ? 0xFF : 0),
                                    "pixel (%d, %d) got %d",
                                    x,
                                    y,
                                    value);
                }
            }
        }
    }

    // --- Part 3: SkFDot6 subpixel translation overflow guards ---
    // With (0.5, 0.5) subpixel positioning, generateGlyphImage applies dx = +32 and dy = -32 (in
    // 26.6 fixed-point). Use a [-10, -10, 0, 0] glyph so xShift = +32 - (-640) = +672, allowing a
    // distinct test case where shiftedXMax fits in SkFDot6 while translatedXMax overflows SkFDot6.
    struct OverflowCase {
        int32_t minX;
        int32_t maxX;
        int32_t minY;
        int32_t maxY;
        bool nonLcdOnly;
    };
    constexpr OverflowCase kOverflows[] = {
            // maxX fits in SkFDot6, but maxX + dx (+32) > SK_MaxS32.
            {SK_MaxS32 - 640, SK_MaxS32 - 16, 0, 640, false},
            // minY fits in SkFDot6, but minY + dy (-32) < SK_MinS32.
            {0, 640, SK_MinS32 + 16, SK_MinS32 + 640, false},
            // Degenerate zero-area outline near SK_MinS32.
            {SK_MinS32, SK_MinS32, 0, 0, false},
            // maxX + dx (SK_MaxS32 - 32) fits in SkFDot6, but maxX + xShift (+672) > SK_MaxS32.
            {SK_MaxS32 - 640, SK_MaxS32 - 64, 0, 640, true},
    };

    const SkRect negLeftBounds = SkRect::MakeLTRB(-10, -10, 0, 0);
    for (SkMask::Format fmt : {SkMask::kA8_Format, SkMask::kBW_Format, SkMask::kLCD16_Format}) {
        const SkPixelGeometry geom =
                fmt == SkMask::kLCD16_Format ? kRGB_H_SkPixelGeometry : kUnknown_SkPixelGeometry;
        SkArenaAlloc alloc{256};
        SkGlyph subpixelGlyph =
                make_test_glyph(negLeftBounds,
                                fmt,
                                geom,
                                SkPackedGlyphID((SkGlyphID)1, SK_FixedHalf, SK_FixedHalf),
                                &alloc);
        REPORTER_ASSERT(reporter, subpixelGlyph.maskFormat() == fmt);

        SkScalerContextFTUtils utils{};
        utils.init(SK_ColorBLACK, SkScalerContext::kSubpixelPositioning_Flag);
        SkMaskGamma::PreBlend noPreBlend;

        for (const OverflowCase& oc : kOverflows) {
            if (oc.nonLcdOnly && fmt == SkMask::kLCD16_Format) {
                continue;
            }
            REPORTER_ASSERT(reporter, FT_Load_Char(face.get(), 'X', FT_LOAD_NO_BITMAP) == 0);
            REPORTER_ASSERT(reporter, face->glyph->outline.n_points >= 4);
            for (int i = 0; i < face->glyph->outline.n_points; ++i) {
                face->glyph->outline.points[i].x = (i & 1) ? oc.maxX : oc.minX;
                face->glyph->outline.points[i].y = (i & 2) ? oc.maxY : oc.minY;
            }

            GuardedBuffer buf(subpixelGlyph.imageSize());
            utils.generateGlyphImage(
                    face.get(), subpixelGlyph, buf.data(), SkMatrix::I(), noPreBlend);

            REPORTER_ASSERT(reporter, buf.canariesIntact());
            REPORTER_ASSERT(reporter, !buf.hasNonZeroPayload());
        }
    }

    // --- Part 4: Monochrome FT_OUTLINE_HIGH_PRECISION Bezier bounds (b/576952569) ---
    // FreeType's raster1 (ftraster.c) enables high-precision mode (precision_scale = 64) from the
    // outline flag and the target bitmap size only, without looking at the outline bounds, and
    // subdivides conics/cubics in signed Long (32-bit on Windows LLP64 / ILP32). Verify that
    // generateGlyphImage:
    //   - preserves FT_OUTLINE_HIGH_PRECISION when a small kBW_Format outline fits in the target,
    //   - clears FT_OUTLINE_HIGH_PRECISION when a divergent outline extends outside the target but
    //     stays within +-kMaxOutlineCoord ((1 << 22) - 64), and
    //   - rejects outlines exceeding +-kMaxOutlineCoord (including the b/576952569 regression
    //     coordinates 0xD55520 / 0xEB5D8B).
    {
        using TagType = std::remove_pointer_t<decltype(FT_Outline::tags)>;
        using ContourType = std::remove_pointer_t<decltype(FT_Outline::contours)>;

        const SkRect bwBounds = SkRect::MakeLTRB(0, -10, 10, 0);
        SkArenaAlloc alloc{256};
        SkGlyph bwGlyph = make_test_glyph(bwBounds,
                                          SkMask::kBW_Format,
                                          kUnknown_SkPixelGeometry,
                                          SkPackedGlyphID((SkGlyphID)1),
                                          &alloc);
        REPORTER_ASSERT(reporter, bwGlyph.maskFormat() == SkMask::kBW_Format);
        REPORTER_ASSERT(reporter, bwGlyph.width() == 10 && bwGlyph.height() == 10);

        SkScalerContextFTUtils utils{};
        utils.init(SK_ColorBLACK, static_cast<SkScalerContext::Flags>(0));
        SkMaskGamma::PreBlend noPreBlend;

        struct BezierCase {
            int32_t startY;
            int32_t ctrlY;
            int32_t endY;
            bool expectAccepted;
            bool expectHighPrecisionKept;
        };
        constexpr int32_t kMaxCoord = (1 << 22) - 64;
        constexpr BezierCase kBezierCases[] = {
                // In-bounds conic/cubic: fits inside [0..640], keeps FT_OUTLINE_HIGH_PRECISION.
                {64, 320, 576, true, true},
                // Divergent outline at exact kMaxOutlineCoord: accepted, high precision cleared.
                {64, kMaxCoord, 320, true, false},
                // 1 unit past kMaxOutlineCoord: rejected before FT_Outline_Get_Bitmap.
                {64, kMaxCoord + 1, 320, false, false},
                // Negative coordinate past -kMaxOutlineCoord: rejected.
                {64, -kMaxCoord - 1, 320, false, false},
                // Regression coordinates from b/576952569 (241,014 px conic): rejected.
                {0xD55520, 0xEB5D8B, 0x140, false, false},
        };

        const FT_Outline savedOutline = face->glyph->outline;
        for (bool testCubic : {false, true}) {
            for (const BezierCase& bc : kBezierCases) {
                // Contour 0: 8x8 px filled square inside [64..576] x [64..576] so accepted cases
                // produce non-zero monochrome pixels. Contour 1: conic (3 pts) or cubic (4 pts).
                std::array<FT_Vector, 8> pts = {{
                        {64, 64},
                        {64, 576},
                        {576, 576},
                        {576, 64},
                        {64, bc.startY},
                        {192, bc.ctrlY},
                        {320, testCubic ? bc.ctrlY : bc.endY},
                        {448, bc.endY},
                }};
                const TagType kOn = static_cast<TagType>(FT_CURVE_TAG_ON);
                const TagType kConic = static_cast<TagType>(FT_CURVE_TAG_CONIC);
                const TagType kCubic = static_cast<TagType>(FT_CURVE_TAG_CUBIC);
                std::array<TagType, 8> tags = {{
                        kOn,
                        kOn,
                        kOn,
                        kOn,
                        kOn,
                        testCubic ? kCubic : kConic,
                        testCubic ? kCubic : kOn,
                        kOn,
                }};
                const unsigned short nPoints = testCubic ? 8 : 7;
                std::array<ContourType, 2> contours = {{
                        static_cast<ContourType>(3),
                        static_cast<ContourType>(nPoints - 1),
                }};

                face->glyph->format = FT_GLYPH_FORMAT_OUTLINE;
                face->glyph->outline.n_contours = 2;
                face->glyph->outline.n_points = nPoints;
                face->glyph->outline.points = pts.data();
                face->glyph->outline.tags = tags.data();
                face->glyph->outline.contours = contours.data();
                face->glyph->outline.flags = FT_OUTLINE_HIGH_PRECISION;

                GuardedBuffer buf(bwGlyph.imageSize());
                utils.generateGlyphImage(
                        face.get(), bwGlyph, buf.data(), SkMatrix::I(), noPreBlend);

                REPORTER_ASSERT(reporter, buf.canariesIntact());
                REPORTER_ASSERT(reporter, buf.hasNonZeroPayload() == bc.expectAccepted);
                if (bc.expectAccepted) {
                    const bool highPrecKept =
                            (face->glyph->outline.flags & FT_OUTLINE_HIGH_PRECISION) != 0;
                    REPORTER_ASSERT(reporter, highPrecKept == bc.expectHighPrecisionKept);
                }
            }
        }
        face->glyph->outline = savedOutline;
    }
}
