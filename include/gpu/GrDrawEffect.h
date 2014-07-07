
#ifndef GrDrawEffect_DEFINED
#define GrDrawEffect_DEFINED

#include "GrEffectStage.h"

/**
 * This class is used to communicate the particular GrEffect used in a draw to the backend-specific
 * effect subclass (e.g. GrGLEffect). It is used to by the backend-specific class to generate a
 * cache key for the effect, generate code on a program cache miss, and to upload uniform values to
 * the program.
 * In addition to the effect, it also communicates any changes between the relationship between
 * the view matrix and local coordinate system since the effect was installed in its GrDrawState.
 * The typical use case is that sometime after an effect was installed a decision was made to draw
 * in device coordinates (i.e. use an identity view-matrix). In such a case the GrDrawEffect's
 * coord-change-matrix would be the inverse of the view matrix that was set when the effect was
 * installed.
 */
class GrDrawEffect {
public:
    GrDrawEffect(const GrEffectStage& stage, bool explicitLocalCoords)
        : fEffectStage(&stage)
        , fExplicitLocalCoords(explicitLocalCoords) {
        SkASSERT(NULL != fEffectStage);
        SkASSERT(NULL != fEffectStage->getEffect());
    }
    const GrEffect* effect() const { return fEffectStage->getEffect(); }

    template <typename T>
    const T& castEffect() const { return *static_cast<const T*>(this->effect()); }

    const SkMatrix& getCoordChangeMatrix() const {
        if (fExplicitLocalCoords) {
            return SkMatrix::I();
        } else {
            return fEffectStage->getCoordChangeMatrix();
        }
    }

    bool programHasExplicitLocalCoords() const { return fExplicitLocalCoords; }

    const int* getVertexAttribIndices() const { return fEffectStage->getVertexAttribIndices(); }
    int getVertexAttribIndexCount() const { return fEffectStage->getVertexAttribIndexCount(); }

private:
    const GrEffectStage*    fEffectStage;
    bool                    fExplicitLocalCoords;
};

#endif
