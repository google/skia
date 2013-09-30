/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVertexEffect_DEFINED
#define GrVertexEffect_DEFINED

#include "GrEffect.h"

/**
 * If an effect needs specialized vertex shader code, then it must inherit from this class.
 * Otherwise it won't be able to add vertex attribs, and it might be given a vertexless shader
 * program in emitCode.
 */
class GrVertexEffect : public GrEffect {
public:
    GrVertexEffect() { fHasVertexCode = true; }

protected:
    /**
     * Subclasses call this from their constructor to register vertex attributes (at most
     * kMaxVertexAttribs). This must only be called from the constructor because GrEffects are
     * immutable.
     */
    void addVertexAttrib(GrSLType type) {
        SkASSERT(fVertexAttribTypes.count() < kMaxVertexAttribs);
        fVertexAttribTypes.push_back(type);
    }

private:
    typedef GrEffect INHERITED;
};

#endif
