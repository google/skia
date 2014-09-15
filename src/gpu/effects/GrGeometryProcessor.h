/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGeometryProcessor_DEFINED
#define GrGeometryProcessor_DEFINED

#include "GrEffect.h"

/**
 * If an effect needs specialized vertex shader code, then it must inherit from this class.
 * Otherwise it won't be able to add vertex attribs, and it will be given a vertexless shader
 * program in emitCode.
 */
class GrGeometryProcessor : public GrEffect {
public:
    GrGeometryProcessor() { fRequiresVertexShader = true; }

protected:
    /**
     * Subclasses call this from their constructor to register vertex attributes (at most
     * kMaxVertexAttribs). This must only be called from the constructor because GrEffects are
     * immutable.
     *
     * We return a reference to the added var so that derived classes can name it nicely and use it
     * in shader code.
     */
    const GrShaderVar& addVertexAttrib(const GrShaderVar& var) {
        SkASSERT(GrShaderVar::kAttribute_TypeModifier == var.getTypeModifier());
        SkASSERT(fVertexAttribs.count() < kMaxVertexAttribs);
        return fVertexAttribs.push_back(var);
    }

private:
    typedef GrEffect INHERITED;
};

#endif
