/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkEmbossMaskFilter.h"
#include "SkBlurMaskFilter.h"
#include "SkBlurMask.h"
#include "SkEmbossMask.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkString.h"

SkEmbossMaskFilter* SkEmbossMaskFilter::Create(SkScalar blurSigma, const Light& light) {
    return SkNEW_ARGS(SkEmbossMaskFilter, (blurSigma, light));
}

static inline int pin2byte(int n) {
    if (n < 0) {
        n = 0;
    } else if (n > 0xFF) {
        n = 0xFF;
    }
    return n;
}

SkMaskFilter* SkBlurMaskFilter::CreateEmboss(const SkScalar direction[3],
                                             SkScalar ambient, SkScalar specular,
                                             SkScalar blurRadius) {
    return SkBlurMaskFilter::CreateEmboss(SkBlurMask::ConvertRadiusToSigma(blurRadius),
                                          direction, ambient, specular);
}

SkMaskFilter* SkBlurMaskFilter::CreateEmboss(SkScalar blurSigma, const SkScalar direction[3],
                                             SkScalar ambient, SkScalar specular) {
    if (direction == NULL) {
        return NULL;
    }

    // ambient should be 0...1 as a scalar
    int am = pin2byte(SkScalarToFixed(ambient) >> 8);

    // specular should be 0..15.99 as a scalar
    int sp = pin2byte(SkScalarToFixed(specular) >> 12);

    SkEmbossMaskFilter::Light   light;

    memcpy(light.fDirection, direction, sizeof(light.fDirection));
    light.fAmbient = SkToU8(am);
    light.fSpecular = SkToU8(sp);

    return SkEmbossMaskFilter::Create(blurSigma, light);
}

///////////////////////////////////////////////////////////////////////////////

static void normalize(SkScalar v[3]) {
    SkScalar mag = SkScalarSquare(v[0]) + SkScalarSquare(v[1]) + SkScalarSquare(v[2]);
    mag = SkScalarSqrt(mag);

    for (int i = 0; i < 3; i++) {
        v[i] /= mag;
    }
}

SkEmbossMaskFilter::SkEmbossMaskFilter(SkScalar blurSigma, const Light& light)
    : fLight(light), fBlurSigma(blurSigma) {
    normalize(fLight.fDirection);
}

SkMask::Format SkEmbossMaskFilter::getFormat() const {
    return SkMask::k3D_Format;
}

bool SkEmbossMaskFilter::filterMask(SkMask* dst, const SkMask& src,
                                    const SkMatrix& matrix, SkIPoint* margin) const {
    SkScalar sigma = matrix.mapRadius(fBlurSigma);

    if (!SkBlurMask::BoxBlur(dst, src, sigma, kInner_SkBlurStyle, kLow_SkBlurQuality)) {
        return false;
    }

    dst->fFormat = SkMask::k3D_Format;
    if (margin) {
        margin->set(SkScalarCeilToInt(3*sigma), SkScalarCeilToInt(3*sigma));
    }

    if (src.fImage == NULL) {
        return true;
    }

    // create a larger buffer for the other two channels (should force fBlur to do this for us)

    {
        uint8_t* alphaPlane = dst->fImage;
        size_t   planeSize = dst->computeImageSize();
        if (0 == planeSize) {
            return false;   // too big to allocate, abort
        }
        dst->fImage = SkMask::AllocImage(planeSize * 3);
        memcpy(dst->fImage, alphaPlane, planeSize);
        SkMask::FreeImage(alphaPlane);
    }

    // run the light direction through the matrix...
    Light   light = fLight;
    matrix.mapVectors((SkVector*)(void*)light.fDirection,
                      (SkVector*)(void*)fLight.fDirection, 1);

    // now restore the length of the XY component
    // cast to SkVector so we can call setLength (this double cast silences alias warnings)
    SkVector* vec = (SkVector*)(void*)light.fDirection;
    vec->setLength(light.fDirection[0],
                   light.fDirection[1],
                   SkPoint::Length(fLight.fDirection[0], fLight.fDirection[1]));

    SkEmbossMask::Emboss(dst, light);

    // restore original alpha
    memcpy(dst->fImage, src.fImage, src.computeImageSize());

    return true;
}

SkFlattenable* SkEmbossMaskFilter::CreateProc(SkReadBuffer& buffer) {
    Light light;
    if (buffer.readByteArray(&light, sizeof(Light))) {
        light.fPad = 0; // for the font-cache lookup to be clean
        const SkScalar sigma = buffer.readScalar();
        return Create(sigma, light);
    }
    return NULL;
}

void SkEmbossMaskFilter::flatten(SkWriteBuffer& buffer) const {
    Light tmpLight = fLight;
    tmpLight.fPad = 0;    // for the font-cache lookup to be clean
    buffer.writeByteArray(&tmpLight, sizeof(tmpLight));
    buffer.writeScalar(fBlurSigma);
}

#ifndef SK_IGNORE_TO_STRING
void SkEmbossMaskFilter::toString(SkString* str) const {
    str->append("SkEmbossMaskFilter: (");

    str->append("direction: (");
    str->appendScalar(fLight.fDirection[0]);
    str->append(", ");
    str->appendScalar(fLight.fDirection[1]);
    str->append(", ");
    str->appendScalar(fLight.fDirection[2]);
    str->append(") ");

    str->appendf("ambient: %d specular: %d ",
        fLight.fAmbient, fLight.fSpecular);

    str->append("blurSigma: ");
    str->appendScalar(fBlurSigma);
    str->append(")");
}
#endif
