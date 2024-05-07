/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLSLVarying_DEFINED
#define GrGLSLVarying_DEFINED

#include "include/core/SkString.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/base/SkTBlockList.h"
#include "src/core/SkSLTypeShared.h"
#include "src/gpu/ganesh/GrShaderVar.h"

class GrGLSLProgramBuilder;
class GrGeometryProcessor;

#ifdef SK_DEBUG
static bool is_matrix(SkSLType type) {
    switch (type) {
        case SkSLType::kFloat2x2:
        case SkSLType::kFloat3x3:
        case SkSLType::kFloat4x4:
        case SkSLType::kHalf2x2:
        case SkSLType::kHalf3x3:
        case SkSLType::kHalf4x4:
            return true;
        default:
            return false;
    }
}
#endif

class GrGLSLVarying {
public:
    enum class Scope {
        kVertToFrag,
        kVertToGeo,
        kGeoToFrag
    };

    GrGLSLVarying() = default;
    GrGLSLVarying(SkSLType type, Scope scope = Scope::kVertToFrag)
        : fType(type)
        , fScope(scope) {
        // Metal doesn't support varying matrices, so we disallow them everywhere for consistency
        SkASSERT(!is_matrix(type));
    }

    void reset(SkSLType type, Scope scope = Scope::kVertToFrag) {
        // Metal doesn't support varying matrices, so we disallow them everywhere for consistency
        SkASSERT(!is_matrix(type));
        *this = GrGLSLVarying();
        fType = type;
        fScope = scope;
    }

    SkSLType type() const { return fType; }
    Scope scope() const { return fScope; }
    bool isInVertexShader() const { return Scope::kGeoToFrag != fScope; }
    bool isInFragmentShader() const { return Scope::kVertToGeo != fScope; }

    const char* vsOut() const { SkASSERT(this->isInVertexShader()); return fVsOut; }
    const char* fsIn() const { SkASSERT(this->isInFragmentShader()); return fFsIn; }

    GrShaderVar vsOutVar() const {
        SkASSERT(this->isInVertexShader());
        return GrShaderVar(this->vsOut(), fType, GrShaderVar::TypeModifier::Out);
    }

    GrShaderVar fsInVar() const {
        SkASSERT(this->isInFragmentShader());
        return GrShaderVar(this->fsIn(), fType, GrShaderVar::TypeModifier::In);
    }

private:
    SkSLType fType = SkSLType::kVoid;
    Scope fScope = Scope::kVertToFrag;
    const char* fVsOut = nullptr;
    const char* fFsIn = nullptr;

    friend class GrGLSLVaryingHandler;
};

static const int kVaryingsPerBlock = 8;

class GrGLSLVaryingHandler {
public:
    explicit GrGLSLVaryingHandler(GrGLSLProgramBuilder* program)
        : fVaryings(kVaryingsPerBlock)
        , fVertexInputs(kVaryingsPerBlock)
        , fVertexOutputs(kVaryingsPerBlock)
        , fFragInputs(kVaryingsPerBlock)
        , fFragOutputs(kVaryingsPerBlock)
        , fProgramBuilder(program)
        , fDefaultInterpolationModifier(nullptr) {}

    virtual ~GrGLSLVaryingHandler() {}

    /**
     * Notifies the varying handler that this shader will never emit geometry in perspective and
     * therefore does not require perspective-correct interpolation. When supported, this allows
     * varyings to use the "noperspective" keyword, which means the GPU can use cheaper math for
     * interpolation.
     */
    void setNoPerspective();

    enum class Interpolation {
        kInterpolated,
        kCanBeFlat, // Use "flat" if it will be faster.
        kMustBeFlat // Use "flat" even if it is known to be slow.
    };

    /**
     * addVarying allows fine grained control for setting up varyings between stages. Calling this
     * function will make sure all necessary decls are setup for the client. The client however is
     * responsible for setting up all shader code (e.g "vOut = vIn;") If you just need to take an
     * attribute and pass it through to an output value in a fragment shader, use
     * addPassThroughAttribute.
     * TODO convert most uses of addVarying to addPassThroughAttribute
     */
    void addVarying(const char* name, GrGLSLVarying* varying,
                    Interpolation = Interpolation::kInterpolated);

    /**
     * The GP can use these calls to pass a vertex shader variable directly to 'output' in the
     * fragment shader. Though this adds code to vertex and fragment stages, 'output' is expected to
     * be defined in the fragment shader before the call is made.
     * TODO it might be nicer behavior to have a flag to declare output inside these calls
     */
    void addPassThroughAttribute(const GrShaderVar& vsVar,
                                 const char* output,
                                 Interpolation = Interpolation::kInterpolated);

    void emitAttributes(const GrGeometryProcessor&);

    // This should be called once all attributes and varyings have been added to the
    // GrGLSLVaryingHanlder and before getting/adding any of the declarations to the shaders.
    void finalize();

    void getVertexDecls(SkString* inputDecls, SkString* outputDecls) const;
    void getFragDecls(SkString* inputDecls, SkString* outputDecls) const;

protected:
    struct VaryingInfo {
        SkSLType         fType;
        bool             fIsFlat;
        SkString         fVsOut;
        GrShaderFlags    fVisibility;
    };

    typedef SkTBlockList<VaryingInfo> VaryingList;
    typedef SkTBlockList<GrShaderVar> VarArray;

    VaryingList    fVaryings;
    VarArray       fVertexInputs;
    VarArray       fVertexOutputs;
    VarArray       fFragInputs;
    VarArray       fFragOutputs;

    // This is not owned by the class
    GrGLSLProgramBuilder* fProgramBuilder;

private:
    void addAttribute(const GrShaderVar& var);

    virtual void onFinalize() = 0;

    // helper function for get*Decls
    void appendDecls(const VarArray& vars, SkString* out) const;

    const char* fDefaultInterpolationModifier;

    friend class GrGLSLProgramBuilder;
};

#endif
