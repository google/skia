/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapProcShader.h"
#include "SkBitmapProvider.h"
#include "SkImage_Base.h"
#include "SkImageShader.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"

SkImageShader::SkImageShader(const SkImage* img, TileMode tmx, TileMode tmy, const SkMatrix* matrix)
    : INHERITED(matrix)
    , fImage(SkRef(img))
    , fTileModeX(tmx)
    , fTileModeY(tmy)
{}

SkFlattenable* SkImageShader::CreateProc(SkReadBuffer& buffer) {
    const TileMode tx = (TileMode)buffer.readUInt();
    const TileMode ty = (TileMode)buffer.readUInt();
    SkMatrix matrix;
    buffer.readMatrix(&matrix);
    SkAutoTUnref<SkImage> img(buffer.readImage());
    if (!img) {
        return nullptr;
    }
    return new SkImageShader(img, tx, ty, &matrix);
}

void SkImageShader::flatten(SkWriteBuffer& buffer) const {
    buffer.writeUInt(fTileModeX);
    buffer.writeUInt(fTileModeY);
    buffer.writeMatrix(this->getLocalMatrix());
    buffer.writeImage(fImage);
}

bool SkImageShader::isOpaque() const {
    return fImage->isOpaque();
}

size_t SkImageShader::contextSize() const {
    return SkBitmapProcShader::ContextSize();
}

SkShader::Context* SkImageShader::onCreateContext(const ContextRec& rec, void* storage) const {
    return SkBitmapProcShader::MakeContext(*this, fTileModeX, fTileModeY,
                                           SkBitmapProvider(fImage), rec, storage);
}

SkShader* SkImageShader::Create(const SkImage* image, TileMode tx, TileMode ty,
                                const SkMatrix* localMatrix) {
    if (!image) {
        return nullptr;
    }
    return new SkImageShader(image, tx, ty, localMatrix);
}

#ifndef SK_IGNORE_TO_STRING
void SkImageShader::toString(SkString* str) const {
    const char* gTileModeName[SkShader::kTileModeCount] = {
        "clamp", "repeat", "mirror"
    };

    str->appendf("ImageShader: ((%s %s) ", gTileModeName[fTileModeX], gTileModeName[fTileModeY]);
    fImage->toString(str);
    this->INHERITED::toString(str);
    str->append(")");
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU

#include "GrTextureAccess.h"
#include "SkGr.h"
#include "SkGrPriv.h"
#include "effects/GrSimpleTextureEffect.h"
#include "effects/GrBicubicEffect.h"
#include "effects/GrSimpleTextureEffect.h"

const GrFragmentProcessor* SkImageShader::asFragmentProcessor(GrContext* context,
                                                              const SkMatrix& viewM,
                                                              const SkMatrix* localMatrix,
                                                              SkFilterQuality filterQuality) const {
    SkMatrix matrix;
    matrix.setIDiv(fImage->width(), fImage->height());

    SkMatrix lmInverse;
    if (!this->getLocalMatrix().invert(&lmInverse)) {
        return nullptr;
    }
    if (localMatrix) {
        SkMatrix inv;
        if (!localMatrix->invert(&inv)) {
            return nullptr;
        }
        lmInverse.postConcat(inv);
    }
    matrix.preConcat(lmInverse);

    SkShader::TileMode tm[] = { fTileModeX, fTileModeY };

    // Must set wrap and filter on the sampler before requesting a texture. In two places below
    // we check the matrix scale factors to determine how to interpret the filter quality setting.
    // This completely ignores the complexity of the drawVertices case where explicit local coords
    // are provided by the caller.
    bool doBicubic;
    GrTextureParams::FilterMode textureFilterMode =
    GrSkFilterQualityToGrFilterMode(filterQuality, viewM, this->getLocalMatrix(), &doBicubic);
    GrTextureParams params(tm, textureFilterMode);
    SkAutoTUnref<GrTexture> texture(as_IB(fImage)->asTextureRef(context, params));
    if (!texture) {
        return nullptr;
    }

    SkAutoTUnref<const GrFragmentProcessor> inner;
    if (doBicubic) {
        inner.reset(GrBicubicEffect::Create(texture, matrix, tm));
    } else {
        inner.reset(GrSimpleTextureEffect::Create(texture, matrix, params));
    }

    if (GrPixelConfigIsAlphaOnly(texture->config())) {
        return SkRef(inner.get());
    }
    return GrFragmentProcessor::MulOutputByInputAlpha(inner);
}

#endif
