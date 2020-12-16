/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTextBlob.h"
#include "src/core/SkSurfacePriv.h"
#include "src/gpu/text/GrTextBlob.h"
#include "tests/Test.h"
#include "tools/ToolUtils.h"

SkBitmap rasterize_blob(SkTextBlob* blob,
                        const SkPaint& paint,
                        GrRecordingContext* rContext,
                        const SkMatrix& matrix) {
    const SkImageInfo info =
            SkImageInfo::Make(500, 500, kN32_SkColorType, kPremul_SkAlphaType);
    auto surface = SkSurface::MakeRenderTarget(rContext, SkBudgeted::kNo, info);
    auto canvas = surface->getCanvas();
    canvas->drawColor(SK_ColorWHITE);
    canvas->concat(matrix);
    canvas->drawTextBlob(blob, 10, 250, paint);
    SkBitmap bitmap;
    bitmap.allocN32Pixels(500, 500);
    surface->readPixels(bitmap, 0, 0);
    return bitmap;
}

bool check_for_black(const SkBitmap& bm) {
    for (int y = 0; y < bm.height(); y++) {
        for (int x = 0; x < bm.width(); x++) {
            if (bm.getColor(x, y) == SK_ColorBLACK) {
                return true;
            }
        }
    }
    return false;
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrTextBlobScaleAnimation, reporter, ctxInfo) {
    auto tf = ToolUtils::create_portable_typeface("Mono", SkFontStyle());
    SkFont font{tf};
    font.setHinting(SkFontHinting::kNormal);
    font.setSize(12);
    font.setEdging(SkFont::Edging::kAntiAlias);
    font.setSubpixel(true);

    SkTextBlobBuilder builder;
    const auto& runBuffer = builder.allocRunPosH(font, 30, 0, nullptr);

    for (int i = 0; i < 30; i++) {
        runBuffer.glyphs[i] = static_cast<SkGlyphID>(i);
        runBuffer.pos[i] = SkIntToScalar(i);
    }
    auto blob = builder.make();

    auto dContext = ctxInfo.directContext();
    bool anyBlack = false;
    for (int n = -13; n < 5; n++) {
        SkMatrix m = SkMatrix::Scale(std::exp2(n), std::exp2(n));
        auto bm = rasterize_blob(blob.get(), SkPaint(), dContext, m);
        anyBlack |= check_for_black(bm);
    }
    REPORTER_ASSERT(reporter, anyBlack);
}

// Test extreme positions for all combinations of positions, origins, and translation matrices.
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrTextBlobMoveAround, reporter, ctxInfo) {
    auto tf = ToolUtils::create_portable_typeface("Mono", SkFontStyle());
    SkFont font{tf};
    font.setHinting(SkFontHinting::kNormal);
    font.setSize(12);
    font.setEdging(SkFont::Edging::kAntiAlias);
    font.setSubpixel(true);

    auto makeBlob = [&](SkPoint delta) {
        SkTextBlobBuilder builder;
        const auto& runBuffer = builder.allocRunPos(font, 30, nullptr);

        for (int i = 0; i < 30; i++) {
            runBuffer.glyphs[i] = static_cast<SkGlyphID>(i);
            runBuffer.points()[i] = SkPoint::Make(SkIntToScalar(i*10) + delta.x(), 50 + delta.y());
        }
        return builder.make();
    };

    auto dContext = ctxInfo.directContext();
    auto rasterizeBlob = [&](SkTextBlob* blob, SkPoint origin, const SkMatrix& matrix) {
        SkPaint paint;
        const SkImageInfo info =
                SkImageInfo::Make(350, 80, kN32_SkColorType, kPremul_SkAlphaType);
        auto surface = SkSurface::MakeRenderTarget(dContext, SkBudgeted::kNo, info);
        auto canvas = surface->getCanvas();
        canvas->drawColor(SK_ColorWHITE);
        canvas->concat(matrix);
        canvas->drawTextBlob(blob, 10 + origin.x(), 40 + origin.y(), paint);
        SkBitmap bitmap;
        bitmap.allocN32Pixels(350, 80);
        surface->readPixels(bitmap, 0, 0);
        return bitmap;
    };

    SkBitmap benchMark;
    {
        auto blob = makeBlob({0, 0});
        benchMark = rasterizeBlob(blob.get(), {0,0}, SkMatrix::I());
    }

    auto checkBitmap = [&](const SkBitmap& bitmap) {
        REPORTER_ASSERT(reporter, benchMark.width() == bitmap.width());
        REPORTER_ASSERT(reporter, benchMark.width() == bitmap.width());

        for (int y = 0; y < benchMark.height(); y++) {
            for (int x = 0; x < benchMark.width(); x++) {
                if (benchMark.getColor(x, y) != bitmap.getColor(x, y)) {
                    return false;
                }
            }
        }
        return true;
    };

    SkScalar interestingNumbers[] = {-10'000'000, -1'000'000, -1, 0, +1, +1'000'000, +10'000'000};
    for (auto originX : interestingNumbers) {
        for (auto originY : interestingNumbers) {
            for (auto translateX : interestingNumbers) {
                for (auto translateY : interestingNumbers) {
                    // Make sure everything adds to zero.
                    SkScalar deltaPosX = -(originX + translateX);
                    SkScalar deltaPosY = -(originY + translateY);
                    auto blob = makeBlob({deltaPosX, deltaPosY});
                    SkMatrix t = SkMatrix::Translate(translateX, translateY);
                    auto bitmap = rasterizeBlob(blob.get(), {originX, originY}, t);
                    REPORTER_ASSERT(reporter, checkBitmap(bitmap));
                }
            }
        }
    }
}

// Helper for defining allocators with inline/reserved storage.
// For argument declarations, stick to the base type (GrSubRunAllocator).
// Note: Inheriting from the storage first means the storage will outlive the
// GrSubRunAllocator, letting ~GrSubRunAllocator read it as it calls destructors.
// (This is mostly only relevant for strict tools like MSAN.)

template <size_t inlineSize>
class GrSTSubRunAllocator : private GrBagOfBytes::Storage<inlineSize>, public GrSubRunAllocator {
public:
    explicit GrSTSubRunAllocator(int firstHeapAllocation =
                                    GrBagOfBytes::MinimumSizeWithOverhead(inlineSize))
            : GrSubRunAllocator{this->data(), SkTo<int>(this->size()), firstHeapAllocation} {}
};

DEF_TEST(GrSubRunAllocator, r) {
    static int created = 0;
    static int destroyed = 0;
    struct Foo {
        Foo() : fI{-2}, fX{-3} { created++; }
        Foo(int i, float x) : fI{i}, fX{x} { created++; }
        ~Foo() { destroyed++; }
        int fI;
        float fX;
    };

    struct alignas(8) OddAlignment {
        char buf[10];
    };

    {
        created = 0;
        destroyed = 0;

        GrSubRunAllocator arena{0};
        int* p = arena.makePOD<int>(3);
        REPORTER_ASSERT(r, *p == 3);
        int* q = arena.makePOD<int>(7);
        REPORTER_ASSERT(r, *q == 7);

        REPORTER_ASSERT(r, *arena.makePOD<int>(3) == 3);
        auto foo = arena.makeUnique<Foo>(3, 4.0f);
        REPORTER_ASSERT(r, foo->fI == 3);
        REPORTER_ASSERT(r, foo->fX == 4.0f);
        REPORTER_ASSERT(r, created == 1);
        REPORTER_ASSERT(r, destroyed == 0);

        arena.makePODArray<int>(10);

        auto fooArray = arena.makeUniqueArray<Foo>(10);
        REPORTER_ASSERT(r, fooArray[3].fI == -2);
        REPORTER_ASSERT(r, fooArray[4].fX == -3.0f);
        REPORTER_ASSERT(r, created == 11);
        REPORTER_ASSERT(r, destroyed == 0);
        arena.makePOD<OddAlignment>();
    }
    REPORTER_ASSERT(r, created == 11);
    REPORTER_ASSERT(r, destroyed == 11);

    {
        created = 0;
        destroyed = 0;
        GrSTSubRunAllocator<64> arena;
        int* p = arena.makePOD<int>(3);
        REPORTER_ASSERT(r, *p == 3);
        int* q = arena.makePOD<int>(7);
        REPORTER_ASSERT(r, *q == 7);

        REPORTER_ASSERT(r, *arena.makePOD<int>(3) == 3);
        auto foo = arena.makeUnique<Foo>(3, 4.0f);
        REPORTER_ASSERT(r, foo->fI == 3);
        REPORTER_ASSERT(r, foo->fX == 4.0f);
        REPORTER_ASSERT(r, created == 1);
        REPORTER_ASSERT(r, destroyed == 0);

        arena.makePODArray<int>(10);

        auto fooArray = arena.makeUniqueArray<Foo>(10);
        REPORTER_ASSERT(r, fooArray[3].fI == -2);
        REPORTER_ASSERT(r, fooArray[4].fX == -3.0f);
        REPORTER_ASSERT(r, created == 11);
        REPORTER_ASSERT(r, destroyed == 0);
        arena.makePOD<OddAlignment>();
    }
    REPORTER_ASSERT(r, created == 11);
    REPORTER_ASSERT(r, destroyed == 11);

    {
        created = 0;
        destroyed = 0;
        std::unique_ptr<char[]> block{new char[1024]};
        GrSubRunAllocator arena{block.get(), 1024, 0};

        REPORTER_ASSERT(r, *arena.makePOD<int>(3) == 3);
        auto foo = arena.makeUnique<Foo>(3, 4.0f);
        REPORTER_ASSERT(r, foo->fI == 3);
        REPORTER_ASSERT(r, foo->fX == 4.0f);
        REPORTER_ASSERT(r, created == 1);
        REPORTER_ASSERT(r, destroyed == 0);

        auto fooArray = arena.makeUniqueArray<Foo>(10);
        REPORTER_ASSERT(r, fooArray[3].fI == -2);
        REPORTER_ASSERT(r, fooArray[4].fX == -3.0f);
        REPORTER_ASSERT(r, created == 11);
        REPORTER_ASSERT(r, destroyed == 0);
        arena.makePOD<OddAlignment>();
    }
    REPORTER_ASSERT(r, created == 11);
    REPORTER_ASSERT(r, destroyed == 11);

    {
        created = 0;
        destroyed = 0;
        GrSubRunAllocator arena;

        struct Node {
            Node(std::unique_ptr<Node, GrSubRunAllocator::Destroyer> next)
                    : fNext{std::move(next)} { created++; }
            ~Node() { destroyed++; }
            std::unique_ptr<Node, GrSubRunAllocator::Destroyer> fNext;
        };

        std::unique_ptr<Node, GrSubRunAllocator::Destroyer> current = nullptr;
        for (int i = 0; i < 128; i++) {
            current = arena.makeUnique<Node>(std::move(current));
        }
        REPORTER_ASSERT(r, created == 128);
        REPORTER_ASSERT(r, destroyed == 0);
    }
    REPORTER_ASSERT(r, created == 128);
    REPORTER_ASSERT(r, destroyed == 128);

    {
        struct I {
            I(int v) : i{v} {}
            ~I() {}
            int i;
        };
        GrSTSubRunAllocator<64> arena;
        auto a = arena.makeUniqueArray<I>(8, [](size_t i) { return i; });
        for (size_t i = 0; i < 8; i++) {
            REPORTER_ASSERT(r, a[i].i == (int)i);
        }
    }

    {
        GrSubRunAllocator arena(4096);
        char* ptr = arena.alignedBytes(4081, 8);
        REPORTER_ASSERT(r, ((intptr_t)ptr & 7) == 0);
    }
}
