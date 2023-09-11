/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkTextureCompressionType.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "src/core/SkCompressedDataUtils.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrImageContextPriv.h"
#include "src/gpu/ganesh/image/SkImage_GaneshBase.h"
#include "src/image/SkImage_Base.h"
#include "tools/gpu/ProxyUtils.h"

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

// BC1 has per-block transparency. If, taken as ints,
//    fColor0 < fColor1    -> the block has transparency (& it is in color3)
//    fColor1 > fColor0    -> the block is opaque
//
// This method can create two blocks to test out BC1's behavior. If BC1
// behaves as expected (i.e., w/ per-block transparency) then, for RGBA textures,
// the transparent block(s) should appear as:
//    opaque black, medium grey, transparent black, white.
// and the opaque block(s) should appear as:
//    opaque black, dark grey, light grey, white
//
// For RGB textures, however, the transparent block(s) should appear as:
//    opaque black, medium grey, _opaque_ black, white
// and the opaque block(s) should appear as:
//    opaque black, dark grey, light grey, white.
static void create_BC1_block(BC1Block* block, bool transparent) {
    unsigned int byte;

    if (transparent) {
        block->fColor0 = to565(SK_ColorBLACK);
        block->fColor1 = to565(SK_ColorWHITE);
        SkASSERT(block->fColor0 <= block->fColor1); // this signals a transparent block
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

    size_t totalSize = SkCompressedDataSize(SkTextureCompressionType::kBC1_RGB8_UNORM, dim,
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

static sk_sp<SkImage> data_to_img(GrDirectContext *direct, sk_sp<SkData> data,
                                  SkTextureCompressionType compression) {
    if (direct) {
        return SkImages::TextureFromCompressedTextureData(
                direct, std::move(data), kImgWidth, kImgHeight, compression, skgpu::Mipmapped::kNo);
    } else {
        return SkImages::RasterFromCompressedTextureData(
                std::move(data), kImgWidth, kImgHeight, compression);
    }
}

static void draw_image(SkCanvas* canvas, sk_sp<SkImage> image, int x, int y) {

    bool isCompressed = false;
    if (image && image->isTextureBacked()) {
        const GrCaps* caps = as_IB(image)->context()->priv().caps();
        GrTextureProxy* proxy = sk_gpu_test::GetTextureImageProxy(image.get(),
                                                                  canvas->recordingContext());
        isCompressed = caps->isFormatCompressed(proxy->backendFormat());
    }

    canvas->drawImage(image, x, y);

    if (!isCompressed) {
        SkRect r = SkRect::MakeXYWH(x, y, kImgWidth, kImgHeight);
        r.outset(1.0f, 1.0f);

        SkPaint redStroke;
        redStroke.setColor(SK_ColorRED);
        redStroke.setStyle(SkPaint::kStroke_Style);
        redStroke.setStrokeWidth(2.0f);

        canvas->drawRect(r, redStroke);
    }
}

namespace skiagm {

// This GM draws the BC1 compressed texture filled with "make_compressed_data"s data twice.
//
// It is drawn once (on the top) as a kBC1_RGB8_UNORM texture and then again (on the bottom)
// as a kBC1_RGBA8_UNORM texture.
//
// If BC1 behaves as expected we should see:
//
//   RGB8             Black MidGrey Black*  White ...
//                    Black DrkGrey LtGrey  White ...
//
//   RGBA8            Black MidGrey Green+  White ...
//                    Black DrkGrey LtGrey  White ...
//
// * We expect this to be black bc the transparent black will be forced to opaque. If BC1 were
//   treating it as an opaque block then it would be LtGrey - not black.
// + This is just the background showing through the transparent black
class BC1TransparencyGM : public GM {
public:
    BC1TransparencyGM() {
        this->setBGColor(SK_ColorGREEN);
    }

protected:
    SkString getName() const override { return SkString("bc1_transparency"); }

    SkISize getISize() override {
        return SkISize::Make(kImgWidth + 2 * kPad, 2 * kImgHeight + 3 * kPad);
    }

    DrawResult onGpuSetup(SkCanvas* canvas, SkString* errorMsg) override {
        auto dContext = GrAsDirectContext(canvas->recordingContext());
        if (dContext && dContext->abandoned()) {
            // This isn't a GpuGM so a null 'context' is okay but an abandoned context
            // if forbidden.
            return DrawResult::kSkip;
        }

        sk_sp<SkData> bc1Data = make_compressed_data();

        fRGBImage = data_to_img(dContext, bc1Data, SkTextureCompressionType::kBC1_RGB8_UNORM);
        fRGBAImage = data_to_img(dContext, std::move(bc1Data),
                                 SkTextureCompressionType::kBC1_RGBA8_UNORM);
        if (!fRGBImage || !fRGBAImage) {
            *errorMsg = "Failed to create BC1 images.";
            return DrawResult::kFail;
        }

        return DrawResult::kOk;
    }

    void onGpuTeardown() override {
        fRGBImage = nullptr;
        fRGBAImage = nullptr;
    }

    void onDraw(SkCanvas* canvas) override {
        draw_image(canvas, fRGBImage, kPad, kPad);
        draw_image(canvas, fRGBAImage, kPad, 2 * kPad + kImgHeight);
    }

private:
    sk_sp<SkImage> fRGBImage;
    sk_sp<SkImage> fRGBAImage;

    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new BC1TransparencyGM;)
}  // namespace skiagm
