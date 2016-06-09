/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPerlinNoiseShader2_DEFINED
#define SkPerlinNoiseShader2_DEFINED

#include "SkShader.h"

/** \class SkPerlinNoiseShader2

    SkPerlinNoiseShader2 creates an image using the Perlin turbulence function.

    It can produce tileable noise if asked to stitch tiles and provided a tile size.
    In order to fill a large area with repeating noise, set the stitchTiles flag to
    true, and render exactly a single tile of noise. Without this flag, the result
    will contain visible seams between tiles.

    The algorithm used is described here :
    http://www.w3.org/TR/SVG/filters.html#feTurbulenceElement
*/
class SK_API SkPerlinNoiseShader2 : public SkShader {
public:
    struct StitchData;
    struct PaintingData;

    /**
     *  About the noise types : the difference between the first 2 is just minor tweaks to the 
     *  algorithm, they're not 2 entirely different noises. The output looks different, but once the 
     *  noise is generated in the [1, -1] range, the output is brought back in the [0, 1] range by 
     *  doing :
     *  kFractalNoise_Type : noise * 0.5 + 0.5
     *  kTurbulence_Type   : abs(noise)
     *  Very little differences between the 2 types, although you can tell the difference visually.
     *  "Improved" is based on the Improved Perlin Noise algorithm described at
     *  http://mrl.nyu.edu/~perlin/noise/. It is quite distinct from the other two, and the noise is
     *  a 2D slice of a 3D noise texture. Minor changes to the Z coordinate will result in minor
     *  changes to the noise, making it suitable for animated noise.
     */
    enum Type {
        kFractalNoise_Type,
        kTurbulence_Type,
        kImprovedNoise_Type,
        kFirstType = kFractalNoise_Type,
        kLastType = kImprovedNoise_Type
    };
    /**
     *  This will construct Perlin noise of the given type (Fractal Noise or Turbulence).
     *
     *  Both base frequencies (X and Y) have a usual range of (0..1).
     *
     *  The number of octaves provided should be fairly small, although no limit is enforced.
     *  Each octave doubles the frequency, so 10 octaves would produce noise from
     *  baseFrequency * 1, * 2, * 4, ..., * 512, which quickly yields insignificantly small
     *  periods and resembles regular unstructured noise rather than Perlin noise.
     *
     *  If tileSize isn't NULL or an empty size, the tileSize parameter will be used to modify
     *  the frequencies so that the noise will be tileable for the given tile size. If tileSize
     *  is NULL or an empty size, the frequencies will be used as is without modification.
     */
    static sk_sp<SkShader> MakeFractalNoise(SkScalar baseFrequencyX, SkScalar baseFrequencyY,
                                            int numOctaves, SkScalar seed,
                                            const SkISize* tileSize = NULL);
    static sk_sp<SkShader> MakeTurbulence(SkScalar baseFrequencyX, SkScalar baseFrequencyY,
                                          int numOctaves, SkScalar seed,
                                          const SkISize* tileSize = NULL);
    /**
     * Creates an Improved Perlin Noise shader. The z value is roughly equivalent to the seed of the
     * other two types, but minor variations to z will only slightly change the noise.
     */
    static sk_sp<SkShader> MakeImprovedNoise(SkScalar baseFrequencyX, SkScalar baseFrequencyY,
                                             int numOctaves, SkScalar z);
    /**
     * Create alias for CreateTurbulunce until all Skia users changed
     * its code to use the new naming
     */
    static sk_sp<SkShader> MakeTubulence(SkScalar baseFrequencyX, SkScalar baseFrequencyY,
                                         int numOctaves, SkScalar seed,
                                         const SkISize* tileSize = NULL) {
        return MakeTurbulence(baseFrequencyX, baseFrequencyY, numOctaves, seed, tileSize);
    }

    class PerlinNoiseShaderContext : public SkShader::Context {
    public:
        PerlinNoiseShaderContext(const SkPerlinNoiseShader2& shader, const ContextRec&);
        virtual ~PerlinNoiseShaderContext();

        void shadeSpan(int x, int y, SkPMColor[], int count) override;

    private:
        SkPMColor shade(const SkPoint& point, StitchData& stitchData) const;
        SkScalar calculateTurbulenceValueForPoint(
            int channel,
            StitchData& stitchData, const SkPoint& point) const;
        SkScalar calculateImprovedNoiseValueForPoint(int channel, const SkPoint& point) const;
        SkScalar noise2D(int channel,
                         const StitchData& stitchData, const SkPoint& noiseVector) const;

        SkMatrix fMatrix;
        PaintingData* fPaintingData;

        typedef SkShader::Context INHERITED;
    };

#if SK_SUPPORT_GPU
    sk_sp<GrFragmentProcessor> asFragmentProcessor(GrContext* context, const SkMatrix& viewM,
                                                   const SkMatrix*, SkFilterQuality,
                                                   SkSourceGammaTreatment) const override;
#endif

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkPerlinNoiseShader2)

protected:
    void flatten(SkWriteBuffer&) const override;
    size_t onContextSize(const ContextRec&) const override;
    Context* onCreateContext(const ContextRec&, void* storage) const override;

private:
    SkPerlinNoiseShader2(SkPerlinNoiseShader2::Type type, SkScalar baseFrequencyX,
                        SkScalar baseFrequencyY, int numOctaves, SkScalar seed,
                        const SkISize* tileSize);
    virtual ~SkPerlinNoiseShader2();

    const SkPerlinNoiseShader2::Type fType;
    const SkScalar                  fBaseFrequencyX;
    const SkScalar                  fBaseFrequencyY;
    const int                       fNumOctaves;
    const SkScalar                  fSeed;
    const SkISize                   fTileSize;
    const bool                      fStitchTiles;

    typedef SkShader INHERITED;
};

#endif
