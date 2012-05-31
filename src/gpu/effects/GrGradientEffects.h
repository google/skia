/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGradientEffects_DEFINED
#define GrGradientEffects_DEFINED

#include "GrCustomStage.h"
#include "GrTypes.h"
#include "GrScalar.h"

class GrGLRadialGradient;

class GrRadialGradient : public GrCustomStage {

public:

    GrRadialGradient();
    virtual ~GrRadialGradient();

    static const char* Name() { return "Radial Gradient"; }
    virtual const GrProgramStageFactory& getFactory() const SK_OVERRIDE;
    virtual bool isEqual(const GrCustomStage&) const SK_OVERRIDE;

    typedef GrGLRadialGradient GLProgramStage;

private:

    typedef GrCustomStage INHERITED;
};

class GrGLRadial2Gradient;

class GrRadial2Gradient : public GrCustomStage {

public:

    GrRadial2Gradient(GrScalar center, GrScalar radius, bool posRoot);
    virtual ~GrRadial2Gradient();

    static const char* Name() { return "Two-Point Radial Gradient"; }
    virtual const GrProgramStageFactory& getFactory() const SK_OVERRIDE;
    virtual bool isEqual(const GrCustomStage&) const SK_OVERRIDE;

    // The radial gradient parameters can collapse to a linear (instead
    // of quadratic) equation.
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

    typedef GrCustomStage INHERITED;
};

class GrGLSweepGradient;

class GrSweepGradient : public GrCustomStage {

public:

    GrSweepGradient();
    virtual ~GrSweepGradient();

    static const char* Name() { return "Sweep Gradient"; }
    virtual const GrProgramStageFactory& getFactory() const SK_OVERRIDE;
    virtual bool isEqual(const GrCustomStage&) const SK_OVERRIDE;

    typedef GrGLSweepGradient GLProgramStage;

protected:

    typedef GrCustomStage INHERITED;
};

#endif

