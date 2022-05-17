/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTextBlob.h"
#include "src/core/SkDevice.h"
#include "src/core/SkSurfacePriv.h"
#include "src/gpu/ganesh/GrColorInfo.h"
#include "src/gpu/ganesh/text/GrTextBlob.h"
#include "tests/Test.h"
#include "tools/ToolUtils.h"

using BagOfBytes = sktext::gpu::BagOfBytes;
using SubRunAllocator = sktext::gpu::SubRunAllocator;

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

DEF_TEST(BagOfBytesBasic, r) {
    const int k4K = 1 << 12;
    {
        // GrBagOfBytes::MinimumSizeWithOverhead(-1); // This should fail
        BagOfBytes::PlatformMinimumSizeWithOverhead(0, 16);
        BagOfBytes::PlatformMinimumSizeWithOverhead(
                std::numeric_limits<int>::max() - k4K - 1, 16);
        // GrBagOfBytes::MinimumSizeWithOverhead(std::numeric_limits<int>::max() - k4K);  // Fail
        REPORTER_ASSERT(r, BagOfBytes::MinimumSizeWithOverhead(0, 1, 16, 16) == 31);
        REPORTER_ASSERT(r, BagOfBytes::MinimumSizeWithOverhead(1, 1, 16, 16) == 32);
        REPORTER_ASSERT(r, BagOfBytes::MinimumSizeWithOverhead(63, 1, 16, 16) == 94);
        REPORTER_ASSERT(r, BagOfBytes::MinimumSizeWithOverhead(0, 8, 16, 16) == 24);
        REPORTER_ASSERT(r, BagOfBytes::MinimumSizeWithOverhead(1, 8, 16, 16) == 32);
        REPORTER_ASSERT(r, BagOfBytes::MinimumSizeWithOverhead(63, 8, 16, 16) == 88);
        REPORTER_ASSERT(r, BagOfBytes::MinimumSizeWithOverhead(0, 16, 16, 16) == 16);
        REPORTER_ASSERT(r, BagOfBytes::MinimumSizeWithOverhead(1, 16, 16, 16) == 32);
        REPORTER_ASSERT(r, BagOfBytes::MinimumSizeWithOverhead(63, 16, 16, 16) == 80);

        REPORTER_ASSERT(r, BagOfBytes::MinimumSizeWithOverhead(0, 1, 8, 16) == 23);
        REPORTER_ASSERT(r, BagOfBytes::MinimumSizeWithOverhead(1, 1, 8, 16) == 24);
        REPORTER_ASSERT(r, BagOfBytes::MinimumSizeWithOverhead(63, 1, 8, 16) == 86);
        REPORTER_ASSERT(r, BagOfBytes::MinimumSizeWithOverhead(0, 8, 8, 16) == 16);
        REPORTER_ASSERT(r, BagOfBytes::MinimumSizeWithOverhead(1, 8, 8, 16) == 24);
        REPORTER_ASSERT(r, BagOfBytes::MinimumSizeWithOverhead(63, 8, 8, 16) == 80);
    }

    {
        BagOfBytes bob;
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
        BagOfBytes bob;
        const int k64K = 1 << 16;
        // By default allocation block sizes start at 1K and go up with fib. This should allocate
        // 10 individual blocks.
        for (int i = 0; i < 10; i++) {
            bob.alignedBytes(k64K, 1);
        }
    }
}

// Helper for defining allocators with inline/reserved storage.
// For argument declarations, stick to the base type (SubRunAllocator).
// Note: Inheriting from the storage first means the storage will outlive the
// SubRunAllocator, letting ~SubRunAllocator read it as it calls destructors.
// (This is mostly only relevant for strict tools like MSAN.)

template <size_t inlineSize>
class GrSTSubRunAllocator : private BagOfBytes::Storage<inlineSize>, public SubRunAllocator {
public:
    explicit GrSTSubRunAllocator(int firstHeapAllocation =
                                     BagOfBytes::PlatformMinimumSizeWithOverhead(inlineSize, 1))
            : SubRunAllocator{this->data(), SkTo<int>(this->size()), firstHeapAllocation} {}
};

DEF_TEST(SubRunAllocator, r) {
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

    auto exercise = [&](SubRunAllocator* alloc) {
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
        SubRunAllocator arena{0};
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
        SubRunAllocator arena{block.get(), 1024, 0};
        exercise(&arena);
    }

    // Exercise the singly-link list of unique_ptrs use case
    {
        created = 0;
        destroyed = 0;
        SubRunAllocator arena;

        struct Node {
            Node(std::unique_ptr<Node, SubRunAllocator::Destroyer> next)
                    : fNext{std::move(next)} { created++; }
            ~Node() { destroyed++; }
            std::unique_ptr<Node, SubRunAllocator::Destroyer> fNext;
        };

        std::unique_ptr<Node, SubRunAllocator::Destroyer> current = nullptr;
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
        SubRunAllocator arena(4096);
        void* ptr = arena.alignedBytes(4081, 8);
        REPORTER_ASSERT(r, ((intptr_t)ptr & 7) == 0);
    }
}

DEF_TEST(KeyEqualityOnPerspective, r) {
    SkTextBlobBuilder builder;
    SkFont font(SkTypeface::MakeDefault(), 16);
    auto runBuffer = builder.allocRun(font, 1, 0.0f, 0.0f);
    runBuffer.glyphs[0] = 3;
    auto blob = builder.make();
    SkGlyphRunBuilder grBuilder;
    auto glyphRunList = grBuilder.blobToGlyphRunList(*blob, {100, 100});
    SkPaint paint;

    // Build the strike device.
    SkSurfaceProps props;
    GrSDFTControl control(false, false, 1, 100);
    SkStrikeDeviceInfo strikeDevice{props, SkScalerContextFlags::kBoostContrast, &control};
    SkMatrix matrix1;
    matrix1.setAll(1, 0, 0, 0, 1, 0, 1, 1, 1);
    SkMatrix matrix2;
    matrix2.setAll(1, 0, 0, 0, 1, 0, 2, 2, 1);
    auto key1 = std::get<1>(
            GrTextBlob::Key::Make(glyphRunList, paint, matrix1, strikeDevice));
    auto key2 = std::get<1>(
            GrTextBlob::Key::Make(glyphRunList, paint, matrix1, strikeDevice));
    auto key3 = std::get<1>(
            GrTextBlob::Key::Make(glyphRunList, paint, matrix2, strikeDevice));
    REPORTER_ASSERT(r, key1 == key2);
    REPORTER_ASSERT(r, !(key1 == key3));
}
