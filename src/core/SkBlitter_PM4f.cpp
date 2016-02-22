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
        
        fState.fProc1(fState, State::WritableAddr(fDevice, x, y), fState.fPM4f, width, nullptr);
    }

    void blitV(int x, int y, int height, SkAlpha alpha) override {
        SkASSERT(x >= 0 && y >= 0 && y + height <= fDevice.height());
        
        typename State::DstType* device = State::WritableAddr(fDevice, x, y);
        size_t                 deviceRB = fDevice.rowBytes();
        
        for (int i = 0; i < height; ++i) {
            fState.fProc1(fState, device, fState.fPM4f, 1, &alpha);
            device = (typename State::DstType*)((char*)device + deviceRB);
        }
    }

    void blitRect(int x, int y, int width, int height) override {
        SkASSERT(x >= 0 && y >= 0 &&
                 x + width <= fDevice.width() && y + height <= fDevice.height());
        
        typename State::DstType* device = State::WritableAddr(fDevice, x, y);
        size_t        deviceRB = fDevice.rowBytes();
        
        do {
            fState.fProc1(fState, device, fState.fPM4f, width, nullptr);
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
                    fState.fProc1(fState, device, fState.fPM4f, count, nullptr);
                } else {
                    for (int i = 0; i < count; ++i) {
                        fState.fProc1(fState, &device[i], fState.fPM4f, 1, antialias);
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
            fState.fProc1(fState, device, fState.fPM4f, width, maskRow);
            device = (typename State::DstType*)((char*)device + dstRB);
            maskRow += maskRB;
        }
    }
};

///////////////////////////////////////////////////////////////////////////////////////////////////

template <typename State> class SkState_Shader_Blitter : public SkShaderBlitter {
public:
    SkState_Shader_Blitter(const SkPixmap& device, const SkPaint& paint,
                           SkShader::Context* shaderContext)
        : INHERITED(device, paint, shaderContext)
        , fState(device.info(), paint, shaderContext)
    {}
    
    void blitH(int x, int y, int width) override {
        SkASSERT(x >= 0 && y >= 0 && x + width <= fDevice.width());
        
        typename State::DstType* device = State::WritableAddr(fDevice, x, y);
        fShaderContext->shadeSpan4f(x, y, fState.fBuffer, width);
        fState.fProcN(fState, device, fState.fBuffer, width, nullptr);
    }

    void blitV(int x, int y, int height, SkAlpha alpha) override {
        SkASSERT(x >= 0 && y >= 0 && y + height <= fDevice.height());
        
        typename State::DstType* device = State::WritableAddr(fDevice, x, y);
        size_t      deviceRB = fDevice.rowBytes();
        const int   bottom = y + height;
        
        if (fConstInY) {
            fShaderContext->shadeSpan4f(x, y, fState.fBuffer, 1);
        }
        for (; y < bottom; ++y) {
            if (!fConstInY) {
                fShaderContext->shadeSpan4f(x, y, fState.fBuffer, 1);
            }
            fState.fProcN(fState, device, fState.fBuffer, 1, &alpha);
            device = (typename State::DstType*)((char*)device + deviceRB);
        }
    }

    void blitRect(int x, int y, int width, int height) override {
        SkASSERT(x >= 0 && y >= 0 &&
                 x + width <= fDevice.width() && y + height <= fDevice.height());
        
        typename State::DstType* device = State::WritableAddr(fDevice, x, y);
        size_t        deviceRB = fDevice.rowBytes();
        const int       bottom = y + height;
        
        if (fConstInY) {
            fShaderContext->shadeSpan4f(x, y, fState.fBuffer, width);
        }
        for (; y < bottom; ++y) {
            if (!fConstInY) {
                fShaderContext->shadeSpan4f(x, y, fState.fBuffer, width);
            }
            fState.fProcN(fState, device, fState.fBuffer, width, nullptr);
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
                fShaderContext->shadeSpan4f(x, y, fState.fBuffer, count);
                if (aa == 255) {
                    fState.fProcN(fState, device, fState.fBuffer, count, nullptr);
                } else {
                    for (int i = 0; i < count; ++i) {
                        fState.fProcN(fState, &device[i], &fState.fBuffer[i], 1, antialias);
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
        
        typename State::DstType* device = State::WritableAddr(fDevice, x, y);
        const size_t deviceRB = fDevice.rowBytes();
        const uint8_t* maskRow = (const uint8_t*)mask.getAddr(x, y);
        const size_t maskRB = mask.fRowBytes;
        
        if (fConstInY) {
            fShaderContext->shadeSpan4f(x, y, fState.fBuffer, width);
        }
        for (; y < clip.fBottom; ++y) {
            if (!fConstInY) {
                fShaderContext->shadeSpan4f(x, y, fState.fBuffer, width);
            }
            fState.fProcN(fState, device, fState.fBuffer, width, maskRow);
            device = (typename State::DstType*)((char*)device + deviceRB);
            maskRow += maskRB;
        }
    }
    
private:
    State   fState;

    typedef SkShaderBlitter INHERITED;
};

//////////////////////////////////////////////////////////////////////////////////////

static bool is_opaque(const SkPaint& paint, const SkShader::Context* shaderContext) {
    return shaderContext ? SkToBool(shaderContext->getFlags() & SkShader::kOpaqueAlpha_Flag)
    : 0xFF == paint.getAlpha();
}

struct State32 : SkXfermode::PM4fState {
    typedef uint32_t        DstType;
    
    SkXfermode::PM4fProc1   fProc1;
    SkXfermode::PM4fProcN   fProcN;
    SkPM4f                  fPM4f;
    SkPM4f*                 fBuffer;
    
    State32(const SkImageInfo& info, const SkPaint& paint, const SkShader::Context* shaderContext) {
        fXfer = SkSafeRef(paint.getXfermode());
        fFlags = 0;
        if (is_opaque(paint, shaderContext)) {
            fFlags |= SkXfermode::kSrcIsOpaque_PM4fFlag;
        }
        if (info.isSRGB()) {
            fFlags |= SkXfermode::kDstIsSRGB_PM4fFlag;
        }
        if (fXfer) {
            fProc1 = fXfer->getPM4fProc1(fFlags);
            fProcN = fXfer->getPM4fProcN(fFlags);
        } else {
            fProc1 = SkXfermode::GetPM4fProc1(SkXfermode::kSrcOver_Mode, fFlags);
            fProcN = SkXfermode::GetPM4fProcN(SkXfermode::kSrcOver_Mode, fFlags);
        }

        fBuffer = nullptr;
        if (shaderContext) {
            fBuffer = new SkPM4f[info.width()];
        } else {
            fPM4f = SkColor4f::FromColor(paint.getColor()).premul();
        }
    }
    
    ~State32() {
        SkSafeUnref(fXfer);
        delete[] fBuffer;
    }

    SkXfermode::LCD32Proc getLCDProc(uint32_t oneOrManyFlag) const {
        uint32_t flags = fFlags & 1;
        if (!(fFlags & SkXfermode::kDstIsSRGB_PM4fFlag)) {
            flags |= SkXfermode::kDstIsLinearInt_LCDFlag;
        }
        return SkXfermode::GetLCD32Proc(flags | oneOrManyFlag);
    }

    static DstType* WritableAddr(const SkPixmap& device, int x, int y) {
        return device.writable_addr32(x, y);
    }
};

struct State64 : SkXfermode::U64State {
    typedef uint64_t        DstType;
    
    SkXfermode::U64Proc1    fProc1;
    SkXfermode::U64ProcN    fProcN;
    SkPM4f                  fPM4f;
    SkPM4f*                 fBuffer;
    
    State64(const SkImageInfo& info, const SkPaint& paint, const SkShader::Context* shaderContext) {
        fXfer = SkSafeRef(paint.getXfermode());
        fFlags = 0;
        if (is_opaque(paint, shaderContext)) {
            fFlags |= SkXfermode::kSrcIsOpaque_PM4fFlag;
        }
        if (kRGBA_F16_SkColorType == info.colorType()) {
            fFlags |= SkXfermode::kDstIsFloat16_U64Flag;
        }
        
        SkXfermode::Mode mode;
        if (!SkXfermode::AsMode(fXfer, &mode)) {
            mode = SkXfermode::kSrcOver_Mode;
        }
        fProc1 = SkXfermode::GetU64Proc1(mode, fFlags);
        fProcN = SkXfermode::GetU64ProcN(mode, fFlags);
        
        fBuffer = nullptr;
        if (shaderContext) {
            fBuffer = new SkPM4f[info.width()];
        } else {
            fPM4f = SkColor4f::FromColor(paint.getColor()).premul();
        }
    }
    
    ~State64() {
        SkSafeUnref(fXfer);
        delete[] fBuffer;
    }
    
    SkXfermode::LCD64Proc getLCDProc(uint32_t oneOrManyFlag) const {
        uint32_t flags = fFlags & 1;
        if (!(fFlags & SkXfermode::kDstIsFloat16_U64Flag)) {
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
        return allocator->createT<SkState_Shader_Blitter<State>>(device, paint, shaderContext);
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
