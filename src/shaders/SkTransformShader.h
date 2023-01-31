/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkTextCoordShader_DEFINED
#define SkTextCoordShader_DEFINED

#include "src/core/SkVM.h"
#include "src/shaders/SkShaderBase.h"

// SkTransformShader applies a matrix transform to the shader coordinates, like a local matrix
// shader. The difference with a typical local matrix shader is that this shader's matrix is
// not combined with the inverse CTM or other local matrices in order to facilitate modifying the
// matrix between uses of the SkVM or SkRasterPipeline. This supports drawVertices and drawAtlas, in
// which the mapping from each triangle (when explicit texture coords are used) or atlas quad to
// shader space is different.
class SkTransformShader : public SkShaderBase {
public:
    explicit SkTransformShader(const SkShaderBase& shader, bool allowPerspective);

    // Adds instructions to use the mapping stored in the uniforms represented by fMatrix. After
    // generating a new skvm::Coord, it passes the mapped coordinates to fShader's program
    // along with the identity matrix.
    skvm::Color program(skvm::Builder* b,
                        skvm::Coord device,
                        skvm::Coord local,
                        skvm::Color color,
                        const MatrixRec& mRec,
                        const SkColorInfo& dst,
                        skvm::Uniforms* uniforms,
                        SkArenaAlloc* alloc) const override;

    // Adds a pipestage to multiply the incoming coords in 'r' and 'g' by the matrix. The child
    // shader is called with no pending local matrix and the total transform as unknowable.
    bool appendStages(const SkStageRec& rec, const MatrixRec&) const override;

    // Change the matrix used by the generated SkRasterpipeline or SkVM.
    bool update(const SkMatrix& matrix);

    // These are never serialized/deserialized
    Factory getFactory() const override {
        SkDEBUGFAIL("SkTransformShader shouldn't be serialized.");
        return {};
    }
    const char* getTypeName() const override {
        SkDEBUGFAIL("SkTransformShader shouldn't be serialized.");
        return nullptr;
    }

    bool isOpaque() const override { return fShader.isOpaque(); }

private:
    const SkShaderBase& fShader;
    SkScalar fMatrixStorage[9];  // actual memory used by generated RP or VM
    bool fAllowPerspective;
};
#endif  //SkTextCoordShader_DEFINED
