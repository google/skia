/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPictureShader_DEFINED
#define SkPictureShader_DEFINED

#include "SkShader.h"

class SkBitmap;
class SkPicture;

/*
 * An SkPictureShader can be used to draw SkPicture-based patterns.
 *
 * The SkPicture is first rendered into a tile, which is then used to shade the area according
 * to specified tiling rules.
 */
class SkPictureShader : public SkShader {
public:
    static SkPictureShader* Create(SkPicture*, TileMode, TileMode);
    virtual ~SkPictureShader();

    virtual bool validContext(const ContextRec&, SkMatrix* totalInverse) const SK_OVERRIDE;
    virtual SkShader::Context* createContext(const ContextRec&, void* storage) const SK_OVERRIDE;
    virtual size_t contextSize() const SK_OVERRIDE;

    class PictureShaderContext : public SkShader::Context {
    public:
        PictureShaderContext(const SkPictureShader&, const ContextRec&, SkShader* bitmapShader);
        virtual ~PictureShaderContext();

        virtual uint32_t getFlags() const SK_OVERRIDE;

        virtual ShadeProc asAShadeProc(void** ctx) SK_OVERRIDE;
        virtual void shadeSpan(int x, int y, SkPMColor dstC[], int count) SK_OVERRIDE;
        virtual void shadeSpan16(int x, int y, uint16_t dstC[], int count) SK_OVERRIDE;

    private:
        SkAutoTUnref<SkShader>  fBitmapShader;
        SkShader::Context*      fBitmapShaderContext;
        void*                   fBitmapShaderContextStorage;

        typedef SkShader::Context INHERITED;
    };

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkPictureShader)

#if SK_SUPPORT_GPU
    GrEffectRef* asNewEffect(GrContext*, const SkPaint&) const SK_OVERRIDE;
#endif

protected:
    SkPictureShader(SkReadBuffer&);
    virtual void flatten(SkWriteBuffer&) const SK_OVERRIDE;

private:
    SkPictureShader(SkPicture*, TileMode, TileMode);

    SkShader* validInternal(const ContextRec&, SkMatrix* totalInverse) const;
    SkShader* refBitmapShader(const SkMatrix&) const;

    SkPicture*  fPicture;
    TileMode    fTmx, fTmy;

    mutable SkMutex                 fCachedBitmapShaderMutex;
    mutable SkAutoTUnref<SkShader>  fCachedBitmapShader;
    mutable SkSize                  fCachedTileScale;
    mutable SkMatrix                fCachedLocalMatrix;

    typedef SkShader INHERITED;
};

#endif // SkPictureShader_DEFINED
