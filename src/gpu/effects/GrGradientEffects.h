/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGradientEffects_DEFINED
#define GrGradientEffects_DEFINED

#include "GrSingleTextureEffect.h"
#include "GrTypes.h"
#include "GrScalar.h"

/*
 * The intepretation of the texture matrix depends on the sample mode. The
 * texture matrix is applied both when the texture coordinates are explicit
 * and  when vertex positions are used as texture  coordinates. In the latter
 * case the texture matrix is applied to the pre-view-matrix position 
 * values.
 *
 * Normal SampleMode
 *  The post-matrix texture coordinates are in normalize space with (0,0) at
 *  the top-left and (1,1) at the bottom right.
 * RadialGradient
 *  The matrix specifies the radial gradient parameters.
 *  (0,0) in the post-matrix space is center of the radial gradient.
 * Radial2Gradient
 *   Matrix transforms to space where first circle is centered at the
 *   origin. The second circle will be centered (x, 0) where x may be 
 *   0 and is provided by setRadial2Params. The post-matrix space is 
 *   normalized such that 1 is the second radius - first radius.
 * SweepGradient
 *  The angle from the origin of texture coordinates in post-matrix space
 *  determines the gradient value.
 */

class GrGLRadialGradient;

class GrRadialGradient : public GrSingleTextureEffect {

public:

    GrRadialGradient(GrTexture* texture);
    virtual ~GrRadialGradient();

    static const char* Name() { return "Radial Gradient"; }
    virtual const GrProgramStageFactory& getFactory() const SK_OVERRIDE;
    virtual bool isEqual(const GrCustomStage&) const SK_OVERRIDE;

    typedef GrGLRadialGradient GLProgramStage;

private:

    typedef GrSingleTextureEffect INHERITED;
};

class GrGLRadial2Gradient;

class GrRadial2Gradient : public GrSingleTextureEffect {

public:

    GrRadial2Gradient(GrTexture* texture, GrScalar center, GrScalar radius, bool posRoot);
    virtual ~GrRadial2Gradient();

    static const char* Name() { return "Two-Point Radial Gradient"; }
    virtual const GrProgramStageFactory& getFactory() const SK_OVERRIDE;
    virtual bool isEqual(const GrCustomStage&) const SK_OVERRIDE;

    // The radial gradient parameters can collapse to a linear (instead of quadratic) equation.
    bool isDegenerate() const { return GR_Scalar1 == fCenterX1; }
    GrScalar center() const { return fCenterX1; }
    GrScalar radius() const { return fRadius0; }
    bool isPosRoot() const { return SkToBool(fPosRoot); }

    typedef GrGLRadial2Gradient GLProgramStage;

private:

    // @{
    // Cache of values - these can change arbitrarily, EXCEPT
    // we shouldn't change between degenerate and non-degenerate?!

    GrScalar fCenterX1;
    GrScalar fRadius0;
    SkBool8  fPosRoot;

    // @}

    typedef GrSingleTextureEffect INHERITED;
};

class GrGLConical2Gradient;

class GrConical2Gradient : public GrSingleTextureEffect {

public:

    GrConical2Gradient(GrTexture* texture, GrScalar center, GrScalar radius, GrScalar diffRadius);
    virtual ~GrConical2Gradient();

    static const char* Name() { return "Two-Point Conical Gradient"; }
    virtual const GrProgramStageFactory& getFactory() const SK_OVERRIDE;
    virtual bool isEqual(const GrCustomStage&) const SK_OVERRIDE;

    // The radial gradient parameters can collapse to a linear (instead of quadratic) equation.
    bool isDegenerate() const { return SkScalarAbs(fDiffRadius) == SkScalarAbs(fCenterX1); }
    GrScalar center() const { return fCenterX1; }
    GrScalar diffRadius() const { return fDiffRadius; }
    GrScalar radius() const { return fRadius0; }

    typedef GrGLConical2Gradient GLProgramStage;

private:

    // @{
    // Cache of values - these can change arbitrarily, EXCEPT
    // we shouldn't change between degenerate and non-degenerate?!

    GrScalar fCenterX1;
    GrScalar fRadius0;
    GrScalar fDiffRadius;

    // @}

    typedef GrSingleTextureEffect INHERITED;
};

class GrGLSweepGradient;

class GrSweepGradient : public GrSingleTextureEffect {

public:

    GrSweepGradient(GrTexture* texture);
    virtual ~GrSweepGradient();

    static const char* Name() { return "Sweep Gradient"; }
    virtual const GrProgramStageFactory& getFactory() const SK_OVERRIDE;
    virtual bool isEqual(const GrCustomStage&) const SK_OVERRIDE;

    typedef GrGLSweepGradient GLProgramStage;

protected:

    typedef GrSingleTextureEffect INHERITED;
};

#endif

