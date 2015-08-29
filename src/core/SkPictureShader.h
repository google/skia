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
    static SkShader* Create(const SkPicture*, TileMode, TileMode, const SkMatrix*,
                                   const SkRect*);
    virtual ~SkPictureShader();

    size_t contextSize() const override;

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkPictureShader)

    bool asFragmentProcessor(GrContext*, const SkPaint&, const SkMatrix& viewM, const SkMatrix*,
                             GrColor*, GrProcessorDataManager*,
                             GrFragmentProcessor**) const override;

protected:
    SkPictureShader(SkReadBuffer&);
    void flatten(SkWriteBuffer&) const override;
    Context* onCreateContext(const ContextRec&, void* storage) const override;

private:
    SkPictureShader(const SkPicture*, TileMode, TileMode, const SkMatrix*, const SkRect*);

    SkShader* refBitmapShader(const SkMatrix&, const SkMatrix* localMatrix, const int maxTextureSize = 0) const;

    const SkPicture* fPicture;
    SkRect           fTile;
    TileMode         fTmx, fTmy;

    class PictureShaderContext : public SkShader::Context {
    public:
        static Context* Create(void* storage, const SkPictureShader&, const ContextRec&,
                               SkShader* bitmapShader);

        virtual ~PictureShaderContext();

        uint32_t getFlags() const override;

        ShadeProc asAShadeProc(void** ctx) override;
        void shadeSpan(int x, int y, SkPMColor dstC[], int count) override;
        void shadeSpan16(int x, int y, uint16_t dstC[], int count) override;

    private:
        PictureShaderContext(const SkPictureShader&, const ContextRec&, SkShader* bitmapShader);

        SkAutoTUnref<SkShader>  fBitmapShader;
        SkShader::Context*      fBitmapShaderContext;
        void*                   fBitmapShaderContextStorage;

        typedef SkShader::Context INHERITED;
    };

    typedef SkShader INHERITED;
};

#endif // SkPictureShader_DEFINED
