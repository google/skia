/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPerlinNoiseShader_DEFINED
#define SkPerlinNoiseShader_DEFINED

#include "SkShader.h"

/** \class SkPerlinNoiseShader

    SkPerlinNoiseShader creates an image using the Perlin turbulence function.

    It can produce tileable noise if asked to stitch tiles and provided a tile size.
    In order to fill a large area with repeating noise, set the stitchTiles flag to
    true, and render exactly a single tile of noise. Without this flag, the result
    will contain visible seams between tiles.

    The algorithm used is described here :
    http://www.w3.org/TR/SVG/filters.html#feTurbulenceElement
*/
class SK_API SkPerlinNoiseShader : public SkShader {
public:
    struct StitchData;
    struct PaintingData;

    /**
     *  About the noise types : the difference between the 2 is just minor tweaks to the algorithm,
     *  they're not 2 entirely different noises. The output looks different, but once the noise is
     *  generated in the [1, -1] range, the output is brought back in the [0, 1] range by doing :
     *  kFractalNoise_Type : noise * 0.5 + 0.5
     *  kTurbulence_Type   : abs(noise)
     *  Very little differences between the 2 types, although you can tell the difference visually.
     */
    enum Type {
        kFractalNoise_Type,
        kTurbulence_Type,
        kFirstType = kFractalNoise_Type,
        kLastType = kTurbulence_Type
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
    static SkShader* CreateFractalNoise(SkScalar baseFrequencyX, SkScalar baseFrequencyY,
                                        int numOctaves, SkScalar seed,
                                        const SkISize* tileSize = NULL);
    static SkShader* CreateTurbulence(SkScalar baseFrequencyX, SkScalar baseFrequencyY,
                                     int numOctaves, SkScalar seed,
                                     const SkISize* tileSize = NULL);
    /**
     * Create alias for CreateTurbulunce until all Skia users changed
     * its code to use the new naming
     */
    static SkShader* CreateTubulence(SkScalar baseFrequencyX, SkScalar baseFrequencyY,
                                     int numOctaves, SkScalar seed,
                                     const SkISize* tileSize = NULL) {
    return CreateTurbulence(baseFrequencyX, baseFrequencyY, numOctaves, seed, tileSize);
    }


    size_t contextSize() const override;

    class PerlinNoiseShaderContext : public SkShader::Context {
    public:
        PerlinNoiseShaderContext(const SkPerlinNoiseShader& shader, const ContextRec&);
        virtual ~PerlinNoiseShaderContext();

        void shadeSpan(int x, int y, SkPMColor[], int count) override;

    private:
        SkPMColor shade(const SkPoint& point, StitchData& stitchData) const;
        SkScalar calculateTurbulenceValueForPoint(
            int channel,
            StitchData& stitchData, const SkPoint& point) const;
        SkScalar noise2D(int channel,
                         const StitchData& stitchData, const SkPoint& noiseVector) const;

        SkMatrix fMatrix;
        PaintingData* fPaintingData;

        typedef SkShader::Context INHERITED;
    };

#if SK_SUPPORT_GPU
    const GrFragmentProcessor* asFragmentProcessor(GrContext* context, const SkMatrix& viewM,
                                                   const SkMatrix*, SkFilterQuality) const override;
#endif

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkPerlinNoiseShader)

protected:
    void flatten(SkWriteBuffer&) const override;
    Context* onCreateContext(const ContextRec&, void* storage) const override;

private:
    SkPerlinNoiseShader(SkPerlinNoiseShader::Type type, SkScalar baseFrequencyX,
                        SkScalar baseFrequencyY, int numOctaves, SkScalar seed,
                        const SkISize* tileSize);
    virtual ~SkPerlinNoiseShader();

    const SkPerlinNoiseShader::Type fType;
    const SkScalar                  fBaseFrequencyX;
    const SkScalar                  fBaseFrequencyY;
    const int                       fNumOctaves;
    const SkScalar                  fSeed;
    const SkISize                   fTileSize;
    const bool                      fStitchTiles;

    typedef SkShader INHERITED;
};

#endif
