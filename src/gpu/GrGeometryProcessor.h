/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGeometryProcessor_DEFINED
#define GrGeometryProcessor_DEFINED

#include "src/gpu/GrPrimitiveProcessor.h"

/**
 * A GrGeometryProcessor is a flexible method for rendering a primitive.  The GrGeometryProcessor
 * has complete control over vertex attributes and uniforms (aside from the render target) but it
 * must obey the same contract as any GrPrimitiveProcessor, specifically it must emit a color and
 * coverage into the fragment shader.  Where this color and coverage come from is completely the
 * responsibility of the GrGeometryProcessor.
 *
 * Note that all derived classes should hide their constructors and provide a Make factory
 * function that takes an arena (except for CCPR-specific classes). This is because
 * GrGeometryProcessor's are not ref-counted to must have some other mechanism for managing
 * their lifetime. In particular, geometry processors can be created in either the
 * record-time or flush-time arenas which defined their lifetimes (i.e., a DDLs life time in
 * the first case and a single flush in the second case).
 */
class GrGeometryProcessor : public GrPrimitiveProcessor {
public:
    GrGeometryProcessor(ClassID classID) : INHERITED(classID) {}

protected:
    // GPs that need to use either float or ubyte colors can just call this to get a correctly
    // configured Attribute struct
    static Attribute MakeColorAttribute(const char* name, bool wideColor) {
        return { name,
                 wideColor ? kFloat4_GrVertexAttribType : kUByte4_norm_GrVertexAttribType,
                 kHalf4_GrSLType };
    }

private:
    typedef GrPrimitiveProcessor INHERITED;
};

#endif
