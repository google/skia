/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBitmapTextGeoProc_DEFINED
#define GrBitmapTextGeoProc_DEFINED

#include "GrProcessor.h"
#include "GrGeometryProcessor.h"

class GrGLBitmapTextGeoProc;
class GrInvariantOutput;

/**
 * The output color of this effect is a modulation of the input color and a sample from a texture.
 * It allows explicit specification of the filtering and wrap modes (GrSamplerState). The input
 * coords are a custom attribute.
 */
class GrBitmapTextGeoProc : public GrGeometryProcessor {
public:

    static sk_sp<GrGeometryProcessor> Make(GrColor color,
                                           const sk_sp<GrTextureProxy>* proxies,
                                           int numActiveProxies,
                                           const GrSamplerState& p, GrMaskFormat format,
                                           const SkMatrix& localMatrix, bool usesW) {
        return sk_sp<GrGeometryProcessor>(
            new GrBitmapTextGeoProc(color, proxies, numActiveProxies, p, format,
                                    localMatrix, usesW));
    }

    ~GrBitmapTextGeoProc() override {}

    const char* name() const override { return "Texture"; }

    const Attribute& inPosition() const { return fInPosition; }
    const Attribute& inColor() const { return fInColor; }
    const Attribute& inTextureCoords() const { return fInTextureCoords; }
    GrMaskFormat maskFormat() const { return fMaskFormat; }
    GrColor color() const { return fColor; }
    bool hasVertexColor() const { return fInColor.isInitialized(); }
    const SkMatrix& localMatrix() const { return fLocalMatrix; }
    bool usesW() const { return fUsesW; }

    void addNewProxies(const sk_sp<GrTextureProxy>*, int numActiveProxies, const GrSamplerState&);

    void getGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const override;

    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps& caps) const override;

private:
    static constexpr int kMaxTextures = 4;

    GrBitmapTextGeoProc(GrColor, const sk_sp<GrTextureProxy>* proxies, int numProxies,
                        const GrSamplerState& params, GrMaskFormat format,
                        const SkMatrix& localMatrix, bool usesW);

    const Attribute& onVertexAttribute(int i) const override;

    GrColor          fColor;
    SkMatrix         fLocalMatrix;
    bool             fUsesW;
    TextureSampler   fTextureSamplers[kMaxTextures];
    Attribute        fInPosition;
    Attribute        fInColor;
    Attribute        fInTextureCoords;
    GrMaskFormat     fMaskFormat;

    GR_DECLARE_GEOMETRY_PROCESSOR_TEST

    typedef GrGeometryProcessor INHERITED;
};

#endif
