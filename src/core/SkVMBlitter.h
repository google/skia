/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkVMBlitter_DEFINED
#define SkVMBlitter_DEFINED

#include "include/core/SkPixmap.h"
#include "src/base/SkArenaAlloc.h"
#include "src/base/SkTLazy.h"
#include "src/core/SkBlitter.h"
#include "src/core/SkLRUCache.h"
#include "src/core/SkVM.h"

class SkVMBlitter final : public SkBlitter {
public:
    static SkVMBlitter* Make(const SkPixmap& dst,
                             const SkPaint&,
                             const SkMatrix& ctm,
                             SkArenaAlloc*,
                             sk_sp<SkShader> clipShader);

    static SkVMBlitter* Make(const SkPixmap& dst,
                             const SkPaint&,
                             const SkPixmap& sprite,
                             int left, int top,
                             SkArenaAlloc*,
                             sk_sp<SkShader> clipShader);

    SkVMBlitter(const SkPixmap& device,
                const SkPaint& paint,
                const SkPixmap* sprite,
                SkIPoint spriteOffset,
                const SkMatrix& ctm,
                sk_sp<SkShader> clip,
                bool* ok);

    ~SkVMBlitter() override;

private:
    enum Coverage { Full, UniformF, MaskA8, MaskLCD16, Mask3D, kCount };
    struct Key {
        uint64_t shader,
                 clip,
                 blender,
                 colorSpace;
        uint8_t  colorType,
                 alphaType,
                 coverage;
        uint8_t  padding8{0};
        uint32_t padding{0};
        // Params::{paint,quality,matrices} are only passed to {shader,clip}->program(),
        // not used here by the blitter itself.  No need to include them in the key;
        // they'll be folded into the shader key if used.

        bool operator==(const Key& that) const;
        Key withCoverage(Coverage c) const;
    };

    struct Params {
        sk_sp<SkShader>         shader;
        sk_sp<SkShader>         clip;
        sk_sp<SkBlender>        blender;    // never null
        SkColorInfo             dst;
        Coverage                coverage;
        SkColor4f               paint;
        SkMatrix                ctm;

        Params withCoverage(Coverage c) const;
    };

    static Params EffectiveParams(const SkPixmap& device,
                                  const SkPixmap* sprite,
                                  SkPaint paint,
                                  const SkMatrix& ctm,
                                  sk_sp<SkShader> clip);
    static skvm::Color DstColor(skvm::Builder* p, const Params& params);
    static void BuildProgram(skvm::Builder* p, const Params& params,
                             skvm::Uniforms* uniforms, SkArenaAlloc* alloc);
    static Key CacheKey(const Params& params,
                        skvm::Uniforms* uniforms, SkArenaAlloc* alloc, bool* ok);
    static SkLRUCache<Key, skvm::Program>* TryAcquireProgramCache();
    static SkString DebugName(const Key& key);
    static void ReleaseProgramCache();

    skvm::Program* buildProgram(Coverage coverage);
    void updateUniforms(int right, int y);
    const void* isSprite(int x, int y) const;

    void blitH(int x, int y, int w) override;
    void blitAntiH(int x, int y, const SkAlpha cov[], const int16_t runs[]) override;

private:
    void blitMask(const SkMask& mask, const SkIRect& clip) override;

    SkPixmap        fDevice;
    const SkPixmap  fSprite;                  // See isSprite().
    const SkIPoint  fSpriteOffset;
    skvm::Uniforms  fUniforms;                // Most data is copied directly into fUniforms,
    SkArenaAlloc    fAlloc{2*sizeof(void*)};  // but a few effects need to ref large content.
    const Params    fParams;
    const Key       fKey;
    bool            fStoreToCache = false;

    skvm::Program*         fProgramPtrs[Coverage::kCount] = {nullptr};
    SkTLazy<skvm::Program> fPrograms[Coverage::kCount];

    friend class Viewer;
};
#endif  // SkVMBlitter_DEFINED
