/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCoreBlitters.h"
#include "SkColorPriv.h"
#include "SkShader.h"
#include "SkUtils.h"
#include "SkXfermode.h"
#include "SkBlitMask.h"
#include "SkTemplates.h"
#include "SkPM4f.h"

template <typename State> class SkState_Blitter : public SkRasterBlitter {
    typedef SkRasterBlitter INHERITED;
    State fState;

public:
    SkState_Blitter(const SkPixmap& device, const SkPaint& paint)
        : INHERITED(device)
        , fState(device.info(), paint, nullptr)
    {}

    void blitH(int x, int y, int width) override {
        SkASSERT(x >= 0 && y >= 0 && x + width <= fDevice.width());

        fState.fProc1(fState.fXfer, State::WritableAddr(fDevice, x, y),
                      &fState.fPM4f, width, nullptr);
    }

    void blitV(int x, int y, int height, SkAlpha alpha) override {
        SkASSERT(x >= 0 && y >= 0 && y + height <= fDevice.height());

        typename State::DstType* device = State::WritableAddr(fDevice, x, y);
        size_t                 deviceRB = fDevice.rowBytes();

        for (int i = 0; i < height; ++i) {
            fState.fProc1(fState.fXfer, device, &fState.fPM4f, 1, &alpha);
            device = (typename State::DstType*)((char*)device + deviceRB);
        }
    }

    void blitRect(int x, int y, int width, int height) override {
        SkASSERT(x >= 0 && y >= 0 &&
                 x + width <= fDevice.width() && y + height <= fDevice.height());

        typename State::DstType* device = State::WritableAddr(fDevice, x, y);
        size_t        deviceRB = fDevice.rowBytes();

        do {
            fState.fProc1(fState.fXfer, device, &fState.fPM4f, width, nullptr);
            y += 1;
            device = (typename State::DstType*)((char*)device + deviceRB);
        } while (--height > 0);
    }

    void blitAntiH(int x, int y, const SkAlpha antialias[], const int16_t runs[]) override {
        typename State::DstType* device = State::WritableAddr(fDevice, x, y);

        for (;;) {
            int count = *runs;
            if (count <= 0) {
                break;
            }
            int aa = *antialias;
            if (aa) {
                if (aa == 255) {
                    fState.fProc1(fState.fXfer, device, &fState.fPM4f, count, nullptr);
                } else {
                    for (int i = 0; i < count; ++i) {
                        fState.fProc1(fState.fXfer, &device[i], &fState.fPM4f, 1, antialias);
                    }
                }
            }
            device += count;
            runs += count;
            antialias += count;
            x += count;
        }
    }

    void blitLCDMask(const SkMask& mask, const SkIRect& clip) {
        auto proc = fState.getLCDProc(SkXfermode::kSrcIsSingle_LCDFlag);

        const int x = clip.fLeft;
        const int width = clip.width();
        const int y = clip.fTop;
        const int height = clip.height();

        typename State::DstType* device = State::WritableAddr(fDevice, x, y);
        const size_t dstRB = fDevice.rowBytes();
        const uint16_t* maskRow = (const uint16_t*)mask.getAddr(x, y);
        const size_t maskRB = mask.fRowBytes;

        for (int i = 0; i < height; ++i) {
            proc(device, &fState.fPM4f, width, maskRow);
            device = (typename State::DstType*)((char*)device + dstRB);
            maskRow = (const uint16_t*)((const char*)maskRow + maskRB);
        }
    }

    void blitMask(const SkMask& mask, const SkIRect& clip) override {
        if (SkMask::kLCD16_Format == mask.fFormat) {
            this->blitLCDMask(mask, clip);
            return;
        }
        if (SkMask::kA8_Format != mask.fFormat) {
            this->INHERITED::blitMask(mask, clip);
            return;
        }

        SkASSERT(mask.fBounds.contains(clip));

        const int x = clip.fLeft;
        const int width = clip.width();
        const int y = clip.fTop;
        const int height = clip.height();

        typename State::DstType* device = State::WritableAddr(fDevice, x, y);
        const size_t dstRB = fDevice.rowBytes();
        const uint8_t* maskRow = (const uint8_t*)mask.getAddr(x, y);
        const size_t maskRB = mask.fRowBytes;

        for (int i = 0; i < height; ++i) {
            fState.fProc1(fState.fXfer, device, &fState.fPM4f, width, maskRow);
            device = (typename State::DstType*)((char*)device + dstRB);
            maskRow += maskRB;
        }
    }
};

///////////////////////////////////////////////////////////////////////////////////////////////////

template <typename State> class SkState_Shader_Blitter : public SkShaderBlitter {
public:
    SkState_Shader_Blitter(const SkPixmap& device, const SkPaint& paint,
                           const SkShader::Context::BlitState& bstate)
        : INHERITED(device, paint, bstate.fCtx)
        , fState(device.info(), paint, bstate.fCtx)
        , fBState(bstate)
        , fBlitBW(bstate.fBlitBW)
        , fBlitAA(bstate.fBlitAA)
    {}

    void blitH(int x, int y, int width) override {
        SkASSERT(x >= 0 && y >= 0 && x + width <= fDevice.width());

        if (fBlitBW) {
            fBlitBW(&fBState, x, y, fDevice, width);
            return;
        }

        typename State::DstType* device = State::WritableAddr(fDevice, x, y);
        fShaderContext->shadeSpan4f(x, y, fState.fBuffer, width);
        fState.fProcN(fState.fXfer, device, fState.fBuffer, width, nullptr);
    }

    void blitV(int x, int y, int height, SkAlpha alpha) override {
        SkASSERT(x >= 0 && y >= 0 && y + height <= fDevice.height());

        if (fBlitAA) {
            for (const int bottom = y + height; y < bottom; ++y) {
                fBlitAA(&fBState, x, y, fDevice, 1, &alpha);
            }
            return;
        }

        typename State::DstType* device = State::WritableAddr(fDevice, x, y);
        size_t                   deviceRB = fDevice.rowBytes();

        if (fConstInY) {
            fShaderContext->shadeSpan4f(x, y, fState.fBuffer, 1);
        }
        for (const int bottom = y + height; y < bottom; ++y) {
            if (!fConstInY) {
                fShaderContext->shadeSpan4f(x, y, fState.fBuffer, 1);
            }
            fState.fProcN(fState.fXfer, device, fState.fBuffer, 1, &alpha);
            device = (typename State::DstType*)((char*)device + deviceRB);
        }
    }

    void blitRect(int x, int y, int width, int height) override {
        SkASSERT(x >= 0 && y >= 0 &&
                 x + width <= fDevice.width() && y + height <= fDevice.height());

        if (fBlitBW) {
            for (const int bottom = y + height; y < bottom; ++y) {
                fBlitBW(&fBState, x, y, fDevice, width);
            }
            return;
        }

        typename State::DstType* device = State::WritableAddr(fDevice, x, y);
        size_t                   deviceRB = fDevice.rowBytes();

        if (fConstInY) {
            fShaderContext->shadeSpan4f(x, y, fState.fBuffer, width);
        }
        for (const int bottom = y + height; y < bottom; ++y) {
            if (!fConstInY) {
                fShaderContext->shadeSpan4f(x, y, fState.fBuffer, width);
            }
            fState.fProcN(fState.fXfer, device, fState.fBuffer, width, nullptr);
            device = (typename State::DstType*)((char*)device + deviceRB);
        }
    }

    void blitAntiH(int x, int y, const SkAlpha antialias[], const int16_t runs[]) override {
        typename State::DstType* device = State::WritableAddr(fDevice, x, y);

        for (;;) {
            int count = *runs;
            if (count <= 0) {
                break;
            }
            int aa = *antialias;
            if (aa) {
                if (fBlitBW && (aa == 255)) {
                    fBlitBW(&fBState, x, y, fDevice, count);
                } else {
                    fShaderContext->shadeSpan4f(x, y, fState.fBuffer, count);
                    if (aa == 255) {
                        fState.fProcN(fState.fXfer, device, fState.fBuffer, count, nullptr);
                    } else {
                        for (int i = 0; i < count; ++i) {
                            fState.fProcN(fState.fXfer, &device[i], &fState.fBuffer[i], 1, antialias);
                        }
                    }
                }
            }
            device += count;
            runs += count;
            antialias += count;
            x += count;
        }
    }

    void blitLCDMask(const SkMask& mask, const SkIRect& clip) {
        auto proc = fState.getLCDProc(0);

        const int x = clip.fLeft;
        const int width = clip.width();
        int y = clip.fTop;

        typename State::DstType* device = State::WritableAddr(fDevice, x, y);
        const size_t deviceRB = fDevice.rowBytes();
        const uint16_t* maskRow = (const uint16_t*)mask.getAddr(x, y);
        const size_t maskRB = mask.fRowBytes;

        if (fConstInY) {
            fShaderContext->shadeSpan4f(x, y, fState.fBuffer, width);
        }
        for (; y < clip.fBottom; ++y) {
            if (!fConstInY) {
                fShaderContext->shadeSpan4f(x, y, fState.fBuffer, width);
            }
            proc(device, fState.fBuffer, width, maskRow);
            device = (typename State::DstType*)((char*)device + deviceRB);
            maskRow = (const uint16_t*)((const char*)maskRow + maskRB);
        }
    }

    void blitMask(const SkMask& mask, const SkIRect& clip) override {
        if (SkMask::kLCD16_Format == mask.fFormat) {
            this->blitLCDMask(mask, clip);
            return;
        }
        if (SkMask::kA8_Format != mask.fFormat) {
            this->INHERITED::blitMask(mask, clip);
            return;
        }

        SkASSERT(mask.fBounds.contains(clip));

        const int x = clip.fLeft;
        const int width = clip.width();
        int y = clip.fTop;
        const uint8_t* maskRow = (const uint8_t*)mask.getAddr(x, y);
        const size_t maskRB = mask.fRowBytes;

        if (fBlitAA) {
            for (; y < clip.fBottom; ++y) {
                fBlitAA(&fBState, x, y, fDevice, width, maskRow);
                maskRow += maskRB;
            }
            return;
        }

        typename State::DstType* device = State::WritableAddr(fDevice, x, y);
        const size_t deviceRB = fDevice.rowBytes();

        if (fConstInY) {
            fShaderContext->shadeSpan4f(x, y, fState.fBuffer, width);
        }
        for (; y < clip.fBottom; ++y) {
            if (!fConstInY) {
                fShaderContext->shadeSpan4f(x, y, fState.fBuffer, width);
            }
            fState.fProcN(fState.fXfer, device, fState.fBuffer, width, maskRow);
            device = (typename State::DstType*)((char*)device + deviceRB);
            maskRow += maskRB;
        }
    }

protected:
    State                        fState;
    SkShader::Context::BlitState fBState;
    SkShader::Context::BlitBW    fBlitBW;
    SkShader::Context::BlitAA    fBlitAA;

    typedef SkShaderBlitter INHERITED;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

static bool is_opaque(const SkPaint& paint, const SkShader::Context* shaderContext) {
    return shaderContext ? SkToBool(shaderContext->getFlags() & SkShader::kOpaqueAlpha_Flag)
    : 0xFF == paint.getAlpha();
}

struct State4f {
    State4f(const SkImageInfo& info, const SkPaint& paint, const SkShader::Context* shaderContext) {
        fXfer = paint.getXfermode();
        if (shaderContext) {
            fBuffer.reset(info.width());
        } else {
            fPM4f = SkColor4f::FromColor(paint.getColor()).premul();
        }
        fFlags = 0;
    }

    SkXfermode*             fXfer;
    SkPM4f                  fPM4f;
    SkAutoTMalloc<SkPM4f>   fBuffer;
    uint32_t                fFlags;

    SkShader::Context::BlitState fBState;
};

struct State32 : State4f {
    typedef uint32_t    DstType;

    SkXfermode::D32Proc fProc1;
    SkXfermode::D32Proc fProcN;

    State32(const SkImageInfo& info, const SkPaint& paint, const SkShader::Context* shaderContext)
        : State4f(info, paint, shaderContext)
    {
        if (is_opaque(paint, shaderContext)) {
            fFlags |= SkXfermode::kSrcIsOpaque_D32Flag;
        }
        if (info.isSRGB()) {
            fFlags |= SkXfermode::kDstIsSRGB_D32Flag;
        }
        fProc1 = SkXfermode::GetD32Proc(fXfer, fFlags | SkXfermode::kSrcIsSingle_D32Flag);
        fProcN = SkXfermode::GetD32Proc(fXfer, fFlags);
    }

    SkXfermode::LCD32Proc getLCDProc(uint32_t oneOrManyFlag) const {
        uint32_t flags = fFlags & 1;
        if (!(fFlags & SkXfermode::kDstIsSRGB_D32Flag)) {
            flags |= SkXfermode::kDstIsLinearInt_LCDFlag;
        }
        return SkXfermode::GetLCD32Proc(flags | oneOrManyFlag);
    }

    static DstType* WritableAddr(const SkPixmap& device, int x, int y) {
        return device.writable_addr32(x, y);
    }
};

struct State64 : State4f {
    typedef uint64_t    DstType;

    SkXfermode::D64Proc fProc1;
    SkXfermode::D64Proc fProcN;

    State64(const SkImageInfo& info, const SkPaint& paint, const SkShader::Context* shaderContext)
        : State4f(info, paint, shaderContext)
    {
        if (is_opaque(paint, shaderContext)) {
            fFlags |= SkXfermode::kSrcIsOpaque_D64Flag;
        }
        if (kRGBA_F16_SkColorType == info.colorType()) {
            fFlags |= SkXfermode::kDstIsFloat16_D64Flag;
        }
        fProc1 = SkXfermode::GetD64Proc(fXfer, fFlags | SkXfermode::kSrcIsSingle_D64Flag);
        fProcN = SkXfermode::GetD64Proc(fXfer, fFlags);
    }

    SkXfermode::LCD64Proc getLCDProc(uint32_t oneOrManyFlag) const {
        uint32_t flags = fFlags & 1;
        if (!(fFlags & SkXfermode::kDstIsFloat16_D64Flag)) {
            flags |= SkXfermode::kDstIsLinearInt_LCDFlag;
        }
        return SkXfermode::GetLCD64Proc(flags | oneOrManyFlag);
    }

    static DstType* WritableAddr(const SkPixmap& device, int x, int y) {
        return device.writable_addr64(x, y);
    }
};

template <typename State> SkBlitter* create(const SkPixmap& device, const SkPaint& paint,
                                            SkShader::Context* shaderContext,
                                            SkTBlitterAllocator* allocator) {
    SkASSERT(allocator != nullptr);

    if (shaderContext) {
        SkShader::Context::BlitState bstate;
        sk_bzero(&bstate, sizeof(bstate));
        bstate.fCtx = shaderContext;
        bstate.fXfer = paint.getXfermode();

        (void)shaderContext->chooseBlitProcs(device.info(), &bstate);
        return allocator->createT<SkState_Shader_Blitter<State>>(device, paint, bstate);
    } else {
        SkColor color = paint.getColor();
        if (0 == SkColorGetA(color)) {
            return nullptr;
        }
        return allocator->createT<SkState_Blitter<State>>(device, paint);
    }
}

SkBlitter* SkBlitter_ARGB32_Create(const SkPixmap& device, const SkPaint& paint,
                                   SkShader::Context* shaderContext,
                                   SkTBlitterAllocator* allocator) {
    return create<State32>(device, paint, shaderContext, allocator);
}

SkBlitter* SkBlitter_ARGB64_Create(const SkPixmap& device, const SkPaint& paint,
                                   SkShader::Context* shaderContext,
                                   SkTBlitterAllocator* allocator) {
    return create<State64>(device, paint, shaderContext, allocator);
}
