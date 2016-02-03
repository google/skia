/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLSLVarying_DEFINED
#define GrGLSLVarying_DEFINED

#include "GrAllocator.h"
#include "GrGeometryProcessor.h"
#include "GrTypesPriv.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLShaderVar.h"

class GrGLSLProgramBuilder;

class GrGLSLVarying {
public:
    bool vsVarying() const { return kVertToFrag_Varying == fVarying ||
                                    kVertToGeo_Varying == fVarying; }
    bool fsVarying() const { return kVertToFrag_Varying == fVarying ||
                                    kGeoToFrag_Varying == fVarying; }
    const char* vsOut() const { return fVsOut; }
    const char* gsIn() const { return fGsIn; }
    const char* gsOut() const { return fGsOut; }
    const char* fsIn() const { return fFsIn; }
    GrSLType type() const { return fType; }

protected:
    enum Varying {
        kVertToFrag_Varying,
        kVertToGeo_Varying,
        kGeoToFrag_Varying,
    };

    GrGLSLVarying(GrSLType type, Varying varying)
        : fVarying(varying), fType(type), fVsOut(nullptr), fGsIn(nullptr), fGsOut(nullptr),
          fFsIn(nullptr) {}

    Varying fVarying;

private:
    GrSLType fType;
    const char* fVsOut;
    const char* fGsIn;
    const char* fGsOut;
    const char* fFsIn;

    friend class GrGLSLVaryingHandler;
};

struct GrGLSLVertToFrag : public GrGLSLVarying {
    GrGLSLVertToFrag(GrSLType type)
        : GrGLSLVarying(type, kVertToFrag_Varying) {}
};

struct GrGLSLVertToGeo : public GrGLSLVarying {
    GrGLSLVertToGeo(GrSLType type)
        : GrGLSLVarying(type, kVertToGeo_Varying) {}
};

struct GrGLSLGeoToFrag : public GrGLSLVarying {
    GrGLSLGeoToFrag(GrSLType type)
        : GrGLSLVarying(type, kGeoToFrag_Varying) {}
};

static const int kVaryingsPerBlock = 8;

class GrGLSLVaryingHandler {
public:
    explicit GrGLSLVaryingHandler(GrGLSLProgramBuilder* program)
        : fVertexInputs(kVaryingsPerBlock)
        , fVertexOutputs(kVaryingsPerBlock)
        , fGeomInputs(kVaryingsPerBlock)
        , fGeomOutputs(kVaryingsPerBlock)
        , fFragInputs(kVaryingsPerBlock)
        , fFragOutputs(kVaryingsPerBlock)
        , fProgramBuilder(program) {}

    typedef GrTAllocator<GrGLSLShaderVar> VarArray;
    typedef GrGLSLProgramDataManager::VaryingHandle VaryingHandle;

    /*
     * addVarying allows fine grained control for setting up varyings between stages. Calling this
     * functions will make sure all necessary decls are setup for the client. The client however is
     * responsible for setting up all shader code (e.g "vOut = vIn;") If you just need to take an
     * attribute and pass it through to an output value in a fragment shader, use
     * addPassThroughAttribute.
     * TODO convert most uses of addVarying to addPassThroughAttribute
     */
    void addVarying(const char* name,
                    GrGLSLVarying*,
                    GrSLPrecision precision = kDefault_GrSLPrecision);

    /*
     * This call can be used by GP to pass an attribute through all shaders directly to 'output' in
     * the fragment shader.  Though this call effects both the vertex shader and fragment shader,
     * it expects 'output' to be defined in the fragment shader before this call is made. If there
     * is a geometry shader, we will simply take the value of the varying from the first vertex and
     * that will be set as the output varying for all emitted vertices.
     * TODO it might be nicer behavior to have a flag to declare output inside this call
     */
    void addPassThroughAttribute(const GrGeometryProcessor::Attribute*, const char* output);

    void emitAttributes(const GrGeometryProcessor& gp);

    void getVertexDecls(SkString* inputDecls, SkString* outputDecls) const;
    void getGeomDecls(SkString* inputDecls, SkString* outputDecls) const;
    void getFragDecls(SkString* inputDecls, SkString* outputDecls) const;
protected:
    VarArray fVertexInputs;
    VarArray fVertexOutputs;
    VarArray fGeomInputs;
    VarArray fGeomOutputs;
    VarArray fFragInputs;
    VarArray fFragOutputs;

    // This is not owned by the class
    GrGLSLProgramBuilder* fProgramBuilder;

private:
    void addVertexVarying(const char* name, GrSLPrecision precision, GrGLSLVarying* v);
    void addGeomVarying(const char* name, GrSLPrecision precision, GrGLSLVarying* v);
    void addFragVarying(GrSLPrecision precision, GrGLSLVarying* v);

    void addAttribute(const GrShaderVar& var);

    // helper function for get*Decls
    void appendDecls(const VarArray& vars, SkString* out) const;

    friend class GrGLSLProgramBuilder;
};

#endif
