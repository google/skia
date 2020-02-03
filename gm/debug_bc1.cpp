/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "src/core/SkCompressedDataUtils.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrContextPriv.h"
#include "src/image/SkImage_Base.h"

constexpr int kImgWidth  = 16;
constexpr int kImgHeight = 8;
constexpr int kPad       = 4;

struct BC1Block {
    uint16_t fColor0;
    uint16_t fColor1;
    uint32_t fIndices;
};

static int num_4x4_blocks(int size) {
    return ((size + 3) & ~3) >> 2;
}

static uint16_t to565(SkColor col) {
    int r5 = SkMulDiv255Round(31, SkColorGetR(col));
    int g6 = SkMulDiv255Round(63, SkColorGetG(col));
    int b5 = SkMulDiv255Round(31, SkColorGetB(col));

    return (r5 << 11) | (g6 << 5) | b5;
}

// BC1, apparently, actually has per-block transparency. If, taken as ints,
//    fColor0 < fColor1    -> the block has transparency (& it is in color3)
//    fColor1 > fColor0    -> the block is opaque
//
// This method can create two blocks to test out BC1's behavior. If BC1
// behaves as expected (i.e., w/ per-block transparency) then, the transparent block(s) should
// appear as:
//    opaque black, medium grey, transparent black, white.
// and the opaque block(s) should appear as:
//    opaque black, dark grey, light grey, white
//
// If sanity prevails, when drawn, we should see the transparent blocks always be drawn
// with transparency and the opaque blocks always be drawn as opaque regardless of the
// type of texture they are jammed into (e.g., RGB_S3TC_DXT1_EXT vs. RGBA_S3TC_DXT1_EXT).
//
// If the texture type overrides the per-block signal then, in RGB8 textures, we would expected
// both the officially transparent and officially opaque blocks to appear as:
//    opaque block, dark grey, light grey, white
// and, in RGBA8 textures, we would expect them both to appear as:
//    opaque black, medium grey, transparent black, white.
static void create_BC1_block(BC1Block* block, bool transparent) {
    unsigned int byte;

    if (transparent) {
        block->fColor0 = to565(SK_ColorBLACK);
        block->fColor1 = to565(SK_ColorWHITE);
        SkASSERT(block->fColor0 < block->fColor1); // this signals a transparent block
        // opaque black (col0), medium grey (col2), transparent black (col3), white (col1).
        byte = (0x0 << 0) | (0x2 << 2) | (0x3 << 4) | (0x1 << 6);
    } else {
        block->fColor0 = to565(SK_ColorWHITE);
        block->fColor1 = to565(SK_ColorBLACK);
        SkASSERT(block->fColor0 > block->fColor1); // this signals an opaque block
        // opaque black (col1), dark grey (col3), light grey (col2), white (col0)
        byte = (0x1 << 0) | (0x3 << 2) | (0x2 << 4) | (0x0 << 6);
    }

    block->fIndices = (byte << 24) | (byte << 16) | (byte << 8) | byte;
}

// This makes a 16x8 BC1 texture which has the top 4 rows be officially transparent
// and the bottom 4 rows be officially opaque.
static sk_sp<SkData> make_compressed_data() {
    SkISize dim{ kImgWidth, kImgHeight };

    size_t totalSize = SkCompressedDataSize(SkImage::CompressionType::kBC1_RGB8_UNORM, dim,
                                            nullptr, false);

    sk_sp<SkData> tmp = SkData::MakeUninitialized(totalSize);
    BC1Block* dstBlocks = reinterpret_cast<BC1Block*>(tmp->writable_data());

    BC1Block transBlock, opaqueBlock;
    create_BC1_block(&transBlock, true);
    create_BC1_block(&opaqueBlock, false);

    int numXBlocks = num_4x4_blocks(kImgWidth);
    int numYBlocks = num_4x4_blocks(kImgHeight);

    for (int y = 0; y < numYBlocks; ++y) {
        for (int x = 0; x < numXBlocks; ++x) {
            dstBlocks[y*numXBlocks + x] = (y < numYBlocks/2) ? transBlock : opaqueBlock;
        }
    }

    return tmp;
}

namespace skiagm {

// This GM draws the BC1 compressed texture filled with "make_compressed_data"s data twice.
//
// It is drawn once (on the top) as a kBC1_RGB8_UNORM texture and then again (on the bottom)
// as a kBC1_RGBA8_UNORM texture.
//
// If BC1 behaves as expected we should see:
//
//   RGB8             Black MidGrey Black*   White ...
//                    Black DrkGrey LtGrey  White ...
//
//   RGBA8            Black MidGrey   Red   White ...
//                    Black DrkGrey LtGrey  White ...
//
// * We expect this to be black bc Skia will force it to be opaque. If BC1 were treating it as
//   opaque it would be LtGrey - not black.
class DebugBC1GM : public GpuGM {
public:
    DebugBC1GM() {
        this->setBGColor(SK_ColorRED);
    }

protected:

    SkString onShortName() override {
        return SkString("debug_bc1");
    }

    SkISize onISize() override {
        return SkISize::Make(kImgWidth + 2 * kPad, 2 * kImgHeight + 3 * kPad);
    }

    void onOnceBeforeDraw() override {
        fBC1Data = make_compressed_data();
    }

    void onDraw(GrContext* context, GrRenderTargetContext* renderTargetContext,
                SkCanvas* canvas) override {
        const GrCaps* caps = context->priv().caps();

        auto rgbImg = SkImage::MakeTextureFromCompressed(context,
                                                         fBC1Data,
                                                         kImgWidth,
                                                         kImgHeight,
                                                         SkImage::CompressionType::kBC1_RGB8_UNORM,
                                                         GrMipMapped::kNo);
        SkASSERT(rgbImg->isTextureBacked());

        auto rgbaImg = SkImage::MakeTextureFromCompressed(context,
                                                          fBC1Data,
                                                          kImgWidth,
                                                          kImgHeight,
                                                          SkImage::CompressionType::kBC1_RGBA8_UNORM,
                                                          GrMipMapped::kNo);
        SkASSERT(rgbaImg->isTextureBacked());

        // Manually decompressed draws will just be red-herrings
        bool isRGBCompressed, isRGBACompressed;
        isRGBCompressed = caps->isFormatCompressed(as_IB(rgbImg)->peekProxy()->backendFormat());
        isRGBACompressed = caps->isFormatCompressed(as_IB(rgbaImg)->peekProxy()->backendFormat());

        if (isRGBCompressed) {
            canvas->drawImage(rgbImg, kPad, kPad);
        }

        if (isRGBACompressed) {
            canvas->drawImage(rgbaImg, kPad, 2 * kPad + kImgHeight);
        }

    }

private:
    sk_sp<SkData> fBC1Data;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new DebugBC1GM;)
}
