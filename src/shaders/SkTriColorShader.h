/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkTriColorShader_DEFINED
#define SkTriColorShader_DEFINED

#include "include/core/SkMatrix.h"
#include "include/core/SkTypes.h"
#include "src/core/SkColorData.h"
#include "src/shaders/SkShaderBase.h"

struct SkPoint;
struct SkStageRec;

class SkTriColorShader : public SkShaderBase {
public:
    SkTriColorShader(bool isOpaque, bool usePersp) : fIsOpaque(isOpaque), fUsePersp(usePersp) {}

    ShaderType type() const override { return ShaderType::kTriColor; }

    // This gets called for each triangle, without re-calling appendStages.
    bool update(const SkMatrix& ctmInv,
                const SkPoint pts[],
                const SkPMColor4f colors[],
                int index0,
                int index1,
                int index2);

protected:
    bool appendStages(const SkStageRec& rec, const SkShaders::MatrixRec&) const override;

private:
    bool isOpaque() const override { return fIsOpaque; }
    // For serialization.  This will never be called.
    Factory getFactory() const override { return nullptr; }
    const char* getTypeName() const override { return nullptr; }

    struct Matrix43 {
        float fMat[12];  // column major

        // Pass a by value, so we don't have to worry about aliasing with this
        void setConcat(const Matrix43 a, const SkMatrix& b) {
            SkASSERT(!b.hasPerspective());

            fMat[0] = a.dot(0, b.getScaleX(), b.getSkewY());
            fMat[1] = a.dot(1, b.getScaleX(), b.getSkewY());
            fMat[2] = a.dot(2, b.getScaleX(), b.getSkewY());
            fMat[3] = a.dot(3, b.getScaleX(), b.getSkewY());

            fMat[4] = a.dot(0, b.getSkewX(), b.getScaleY());
            fMat[5] = a.dot(1, b.getSkewX(), b.getScaleY());
            fMat[6] = a.dot(2, b.getSkewX(), b.getScaleY());
            fMat[7] = a.dot(3, b.getSkewX(), b.getScaleY());

            fMat[8] = a.dot(0, b.getTranslateX(), b.getTranslateY()) + a.fMat[8];
            fMat[9] = a.dot(1, b.getTranslateX(), b.getTranslateY()) + a.fMat[9];
            fMat[10] = a.dot(2, b.getTranslateX(), b.getTranslateY()) + a.fMat[10];
            fMat[11] = a.dot(3, b.getTranslateX(), b.getTranslateY()) + a.fMat[11];
        }

    private:
        float dot(int index, float x, float y) const {
            return fMat[index + 0] * x + fMat[index + 4] * y;
        }
    };

    // If fUsePersp, we need both of these matrices,
    // otherwise we can combine them, and only use fM43

    Matrix43 fM43;
    SkMatrix fM33;
    const bool fIsOpaque;
    const bool fUsePersp;  // controls our stages, and what we do in update()
};

#endif
