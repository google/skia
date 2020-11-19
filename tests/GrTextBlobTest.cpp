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

DEF_TEST(GrBagOfBytesBasic, r) {
    const int k4K = 1 << 12;
    {
        // GrBagOfBytes::MinimumSizeWithOverhead(-1); // This should fail
        GrBagOfBytes::PlatformMinimumSizeWithOverhead(0, 16);
        GrBagOfBytes::PlatformMinimumSizeWithOverhead(
                std::numeric_limits<int>::max() - k4K - 1, 16);
        // GrBagOfBytes::MinimumSizeWithOverhead(std::numeric_limits<int>::max() - k4K);  // Fail
        REPORTER_ASSERT(r, GrBagOfBytes::MinimumSizeWithOverhead(0, 1, 16, 16) == 31);
        REPORTER_ASSERT(r, GrBagOfBytes::MinimumSizeWithOverhead(1, 1, 16, 16) == 32);
        REPORTER_ASSERT(r, GrBagOfBytes::MinimumSizeWithOverhead(63, 1, 16, 16) == 94);
        REPORTER_ASSERT(r, GrBagOfBytes::MinimumSizeWithOverhead(0, 8, 16, 16) == 24);
        REPORTER_ASSERT(r, GrBagOfBytes::MinimumSizeWithOverhead(1, 8, 16, 16) == 32);
        REPORTER_ASSERT(r, GrBagOfBytes::MinimumSizeWithOverhead(63, 8, 16, 16) == 88);
        REPORTER_ASSERT(r, GrBagOfBytes::MinimumSizeWithOverhead(0, 16, 16, 16) == 16);
        REPORTER_ASSERT(r, GrBagOfBytes::MinimumSizeWithOverhead(1, 16, 16, 16) == 32);
        REPORTER_ASSERT(r, GrBagOfBytes::MinimumSizeWithOverhead(63, 16, 16, 16) == 80);

        REPORTER_ASSERT(r, GrBagOfBytes::MinimumSizeWithOverhead(0, 1, 8, 16) == 23);
        REPORTER_ASSERT(r, GrBagOfBytes::MinimumSizeWithOverhead(1, 1, 8, 16) == 24);
        REPORTER_ASSERT(r, GrBagOfBytes::MinimumSizeWithOverhead(63, 1, 8, 16) == 86);
        REPORTER_ASSERT(r, GrBagOfBytes::MinimumSizeWithOverhead(0, 8, 8, 16) == 16);
        REPORTER_ASSERT(r, GrBagOfBytes::MinimumSizeWithOverhead(1, 8, 8, 16) == 24);
        REPORTER_ASSERT(r, GrBagOfBytes::MinimumSizeWithOverhead(63, 8, 8, 16) == 80);
    }

    {
        GrBagOfBytes bob;
        // bob.alignedBytes(0, 1);  // This should fail
        // bob.alignedBytes(1, 0);  // This should fail
        // bob.alignedBytes(1, 3);  // This should fail

        struct Big {
            char stuff[std::numeric_limits<int>::max()];
        };
        // bob.alignedBytes(sizeof(Big), 1);  // this should fail
        // bob.allocateBytesFor<Big>();  // this should not compile
        // The following should run, but should not be regularly tested.
        // bob.allocateBytesFor<int>((std::numeric_limits<int>::max() - (1<<12)) / sizeof(int) - 1);
        // The following should fail
        // bob.allocateBytesFor<int>((std::numeric_limits<int>::max() - (1<<12)) / sizeof(int));
        bob.alignedBytes(1, 1);  // To avoid unused variable problems.
    }

    // Force multiple block allocation
    {
        GrBagOfBytes bob;
        const int k64K = 1 << 16;
        // By default allocation block sizes start at 1K and go up with fib. This should allocate
        // 10 individual blocks.
        for (int i = 0; i < 10; i++) {
            bob.alignedBytes(k64K, 1);
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
                                    GrBagOfBytes::PlatformMinimumSizeWithOverhead(inlineSize, 1))
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

    auto exercise = [&](GrSubRunAllocator* alloc) {
        created = 0;
        destroyed = 0;
        {
            int* p = alloc->makePOD<int>(3);
            REPORTER_ASSERT(r, *p == 3);
            int* q = alloc->makePOD<int>(7);
            REPORTER_ASSERT(r, *q == 7);

            REPORTER_ASSERT(r, *alloc->makePOD<int>(3) == 3);
            auto foo = alloc->makeUnique<Foo>(3, 4.0f);
            REPORTER_ASSERT(r, foo->fI == 3);
            REPORTER_ASSERT(r, foo->fX == 4.0f);
            REPORTER_ASSERT(r, created == 1);
            REPORTER_ASSERT(r, destroyed == 0);

            alloc->makePODArray<int>(10);

            auto fooArray = alloc->makeUniqueArray<Foo>(10);
            REPORTER_ASSERT(r, fooArray[3].fI == -2);
            REPORTER_ASSERT(r, fooArray[4].fX == -3.0f);
            REPORTER_ASSERT(r, created == 11);
            REPORTER_ASSERT(r, destroyed == 0);
            alloc->makePOD<OddAlignment>();
        }

        REPORTER_ASSERT(r, created == 11);
        REPORTER_ASSERT(r, destroyed == 11);
    };

    // Exercise default arena
    {
        GrSubRunAllocator arena{0};
        exercise(&arena);
    }

    // Exercise on stack arena
    {
        GrSTSubRunAllocator<64> arena;
        exercise(&arena);
    }

    // Exercise arena with a heap allocated starting block
    {
        std::unique_ptr<char[]> block{new char[1024]};
        GrSubRunAllocator arena{block.get(), 1024, 0};
        exercise(&arena);
    }

    // Exercise the singly-link list of unique_ptrs use case
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

    // Exercise the array ctor w/ a mapping function
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
        void* ptr = arena.alignedBytes(4081, 8);
        REPORTER_ASSERT(r, ((intptr_t)ptr & 7) == 0);
    }
}
