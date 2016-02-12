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
        : fVaryings(kVaryingsPerBlock)
        , fVertexInputs(kVaryingsPerBlock)
        , fVertexOutputs(kVaryingsPerBlock)
        , fGeomInputs(kVaryingsPerBlock)
        , fGeomOutputs(kVaryingsPerBlock)
        , fFragInputs(kVaryingsPerBlock)
        , fFragOutputs(kVaryingsPerBlock)
        , fProgramBuilder(program)
        , fDefaultInterpolationModifier(nullptr) {}

    virtual ~GrGLSLVaryingHandler() {}

    /*
     * Notifies the varying handler that this shader will never emit geometry in perspective and
     * therefore does not require perspective-correct interpolation. When supported, this allows
     * varyings to use the "noperspective" keyword, which means the GPU can use cheaper math for
     * interpolation.
     */
    void setNoPerspective();

    /*
     * addVarying allows fine grained control for setting up varyings between stages. Calling this
     * functions will make sure all necessary decls are setup for the client. The client however is
     * responsible for setting up all shader code (e.g "vOut = vIn;") If you just need to take an
     * attribute and pass it through to an output value in a fragment shader, use
     * addPassThroughAttribute.
     * TODO convert most uses of addVarying to addPassThroughAttribute
     */
    void addVarying(const char* name,
                    GrGLSLVarying* varying,
                    GrSLPrecision precision = kDefault_GrSLPrecision) {
        SkASSERT(GrSLTypeIsFloatType(varying->type())); // Integers must use addFlatVarying.
        this->internalAddVarying(name, varying, precision, false /*flat*/);
    }

    /*
     * addFlatVarying sets up a varying whose value is constant across every fragment. The graphics
     * pipeline will pull its value from the final vertex of the draw primitive (provoking vertex).
     * Flat interpolation is not always supported and the user must check the caps before using.
     * TODO: Some platforms can change the provoking vertex. Should we be resetting this knob?
     */
    void addFlatVarying(const char* name,
                        GrGLSLVarying* varying,
                        GrSLPrecision precision = kDefault_GrSLPrecision) {
        this->internalAddVarying(name, varying, precision, true /*flat*/);
    }

    /*
     * The GP can use these calls to pass an attribute through all shaders directly to 'output' in
     * the fragment shader.  Though these calls affect both the vertex shader and fragment shader,
     * they expect 'output' to be defined in the fragment shader before the call is made. If there
     * is a geometry shader, we will simply take the value of the varying from the first vertex and
     * that will be set as the output varying for all emitted vertices.
     * TODO it might be nicer behavior to have a flag to declare output inside these calls
     */
    void addPassThroughAttribute(const GrGeometryProcessor::Attribute*, const char* output,
                                 GrSLPrecision = kDefault_GrSLPrecision);
    void addFlatPassThroughAttribute(const GrGeometryProcessor::Attribute*, const char* output,
                                     GrSLPrecision = kDefault_GrSLPrecision);

    void emitAttributes(const GrGeometryProcessor& gp);

    // This should be called once all attributes and varyings have been added to the
    // GrGLSLVaryingHanlder and before getting/adding any of the declarations to the shaders.
    void finalize();

    void getVertexDecls(SkString* inputDecls, SkString* outputDecls) const;
    void getGeomDecls(SkString* inputDecls, SkString* outputDecls) const;
    void getFragDecls(SkString* inputDecls, SkString* outputDecls) const;

protected:
    struct VaryingInfo {
        GrSLType         fType;
        GrSLPrecision    fPrecision;
        bool             fIsFlat;
        SkString         fVsOut;
        SkString         fGsOut;
        GrShaderFlags    fVisibility;
    };

    typedef GrTAllocator<VaryingInfo> VaryingList;
    typedef GrTAllocator<GrGLSLShaderVar> VarArray;
    typedef GrGLSLProgramDataManager::VaryingHandle VaryingHandle;

    VaryingList    fVaryings;
    VarArray       fVertexInputs;
    VarArray       fVertexOutputs;
    VarArray       fGeomInputs;
    VarArray       fGeomOutputs;
    VarArray       fFragInputs;
    VarArray       fFragOutputs;

    // This is not owned by the class
    GrGLSLProgramBuilder* fProgramBuilder;

private:
    void internalAddVarying(const char* name, GrGLSLVarying*, GrSLPrecision, bool flat);
    void writePassThroughAttribute(const GrGeometryProcessor::Attribute*, const char* output,
                                   const GrGLSLVarying&);

    void addAttribute(const GrShaderVar& var);

    virtual void onFinalize() = 0;

    // helper function for get*Decls
    void appendDecls(const VarArray& vars, SkString* out) const;

    const char* fDefaultInterpolationModifier;

    friend class GrGLSLProgramBuilder;
};

#endif
