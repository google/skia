/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrPrimitiveProcessor.h"

#include "GrCoordTransform.h"

/**
 * We specialize the vertex code for each of these matrix types.
 */
enum MatrixType {
    kNoPersp_MatrixType  = 0,
    kGeneral_MatrixType  = 1,
};

GrPrimitiveProcessor::GrPrimitiveProcessor(ClassID classID) : GrResourceIOProcessor(classID) {}

const GrPrimitiveProcessor::Attribute& GrPrimitiveProcessor::vertexAttribute(int i) const {
    SkASSERT(i >= 0 && i < this->numVertexAttributes());
    const auto& result = this->onVertexAttribute(i);
    SkASSERT(result.isInitialized());
    return result;
}

const GrPrimitiveProcessor::Attribute& GrPrimitiveProcessor::instanceAttribute(int i) const {
    SkASSERT(i >= 0 && i < this->numInstanceAttributes());
    const auto& result = this->onInstanceAttribute(i);
    SkASSERT(result.isInitialized());
    return result;
}

void GrPrimitiveProcessor::setVertexAttributeInfo(int attributeCnt, size_t vertexStride) {
    fVertexStride = vertexStride;
    fVertexAttributeCnt = attributeCnt;
#ifdef SK_DEBUG
    for (int i = 0; i < attributeCnt; ++i) {
        size_t a = this->vertexAttribute(i).offset();
        size_t b = this->vertexAttribute(i).nextOffsetInRecord();
        SkASSERT(b <= vertexStride);
        for (int j = i + 1; j < attributeCnt; ++j) {
            size_t c = this->vertexAttribute(j).offset();
            size_t d = this->vertexAttribute(j).nextOffsetInRecord();
            SkASSERT(b <= c || d <= a);
        }
    }
#endif
}

void GrPrimitiveProcessor::setInstanceAttributeInfo(int attributeCnt, size_t instanceStride) {
    fInstanceStride = instanceStride;
    fInstanceAttributeCnt = attributeCnt;
#ifdef SK_DEBUG
    for (int i = 0; i < attributeCnt; ++i) {
        size_t a = this->instanceAttribute(i).offset();
        size_t b = this->instanceAttribute(i).nextOffsetInRecord();
        SkASSERT(b <= instanceStride);
        for (int j = i + 1; j < attributeCnt; ++j) {
            size_t c = this->instanceAttribute(j).offset();
            size_t d = this->instanceAttribute(j).nextOffsetInRecord();
            SkASSERT(b <= c || d <= a);
        }
    }
#endif
}

uint32_t
GrPrimitiveProcessor::getTransformKey(const SkTArray<const GrCoordTransform*, true>& coords,
                                      int numCoords) const {
    uint32_t totalKey = 0;
    for (int t = 0; t < numCoords; ++t) {
        uint32_t key = 0;
        const GrCoordTransform* coordTransform = coords[t];
        if (coordTransform->getMatrix().hasPerspective()) {
            key |= kGeneral_MatrixType;
        } else {
            key |= kNoPersp_MatrixType;
        }
        key <<= t;
        SkASSERT(0 == (totalKey & key)); // keys for each transform ought not to overlap
        totalKey |= key;
    }
    return totalKey;
}
