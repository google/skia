/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "effects/GrDisableColorXP.h"
#include "GrProcessor.h"
#include "gl/GrGLXferProcessor.h"

class GrGLDisableColorXP : public GrGLXferProcessor {
public:
    GrGLDisableColorXP(const GrProcessor&) {}

    ~GrGLDisableColorXP() SK_OVERRIDE {}

    void emitCode(const EmitArgs& args) SK_OVERRIDE {}

    void setData(const GrGLProgramDataManager&, const GrXferProcessor&) SK_OVERRIDE {}

    static void GenKey(const GrProcessor&, const GrGLCaps&, GrProcessorKeyBuilder*) {}

private:
    typedef GrGLXferProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

GrDisableColorXP::GrDisableColorXP() {
    this->initClassID<GrDisableColorXP>();
}

void GrDisableColorXP::getGLProcessorKey(const GrGLCaps& caps, GrProcessorKeyBuilder* b) const {
    GrGLDisableColorXP::GenKey(*this, caps, b);
}

GrGLXferProcessor* GrDisableColorXP::createGLInstance() const {
    return SkNEW_ARGS(GrGLDisableColorXP, (*this));
}

void GrDisableColorXP::getBlendInfo(GrXferProcessor::BlendInfo* blendInfo) const {
    blendInfo->fWriteColor = false;
}

///////////////////////////////////////////////////////////////////////////////

GrDisableColorXPFactory::GrDisableColorXPFactory() {
    this->initClassID<GrDisableColorXPFactory>();
}

GrXferProcessor* GrDisableColorXPFactory::createXferProcessor(const GrProcOptInfo& colorPOI,
                                                              const GrProcOptInfo& covPOI) const {
    return GrDisableColorXP::Create();
}

GR_DEFINE_XP_FACTORY_TEST(GrDisableColorXPFactory);

GrXPFactory* GrDisableColorXPFactory::TestCreate(SkRandom* random,
                                                  GrContext*,
                                                  const GrDrawTargetCaps&,
                                                  GrTexture*[]) {
    return GrDisableColorXPFactory::Create();
}

