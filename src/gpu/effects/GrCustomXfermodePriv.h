/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCustomXfermodePriv_DEFINED
#define GrCustomXfermodePriv_DEFINED

#include "GrCoordTransform.h"
#include "GrFragmentProcessor.h"
#include "GrTextureAccess.h"
#include "SkXfermode.h"

class GrGLCaps;
class GrGLFragmentProcessor;
class GrInvariantOutput;
class GrProcessorKeyBuilder;
class GrTexture;

///////////////////////////////////////////////////////////////////////////////
// Fragment Processor
///////////////////////////////////////////////////////////////////////////////

class GrCustomXferFP : public GrFragmentProcessor {
public:
    GrCustomXferFP(SkXfermode::Mode mode, GrTexture* background);

    void getGLProcessorKey(const GrGLCaps& caps, GrProcessorKeyBuilder* b) const SK_OVERRIDE; 

    GrGLFragmentProcessor* createGLInstance() const SK_OVERRIDE;

    const char* name() const SK_OVERRIDE { return "Custom Xfermode"; }

    SkXfermode::Mode mode() const { return fMode; }
    const GrTextureAccess&  backgroundAccess() const { return fBackgroundAccess; }

private:
    bool onIsEqual(const GrFragmentProcessor& other) const SK_OVERRIDE; 

    void onComputeInvariantOutput(GrInvariantOutput* inout) const SK_OVERRIDE;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    SkXfermode::Mode fMode;
    GrCoordTransform fBackgroundTransform;
    GrTextureAccess  fBackgroundAccess;

    typedef GrFragmentProcessor INHERITED;
};

#endif

