/*
    Copyright 2011 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */

#ifndef GrContext_impl_DEFINED
#define GrContext_impl_DEFINED

struct GrContext::OffscreenRecord {
    OffscreenRecord() { fEntry0 = NULL; fEntry1 = NULL; }
    ~OffscreenRecord() { GrAssert(NULL == fEntry0 && NULL == fEntry1); }

    enum Downsample {
        k4x4TwoPass_Downsample,
        k4x4SinglePass_Downsample,
        kFSAA_Downsample
    }                              fDownsample;
    GrTextureEntry*                fEntry0;
    GrTextureEntry*                fEntry1;
    GrDrawTarget::SavedDrawState   fSavedState;
};

template <typename POS_SRC, typename TEX_SRC,
          typename COL_SRC, typename IDX_SRC>
inline void GrContext::drawCustomVertices(const GrPaint& paint,
                                          GrPrimitiveType primitiveType,
                                          const POS_SRC& posSrc,
                                          const TEX_SRC* texCoordSrc,
                                          const COL_SRC* colorSrc,
                                          const IDX_SRC* idxSrc) {

    GrDrawTarget::AutoReleaseGeometry geo;

    GrDrawTarget* target = this->prepareToDraw(paint, kUnbuffered_DrawCategory);

    bool hasTexCoords[GrPaint::kTotalStages] = {
        NULL != texCoordSrc, // texCoordSrc provides explicit stage 0 coords
        0                    // remaining stages use positions
    };
    GrVertexLayout layout = PaintStageVertexLayoutBits(paint, hasTexCoords);

    if (NULL != colorSrc) {
        layout |= GrDrawTarget::kColor_VertexLayoutBit;
    }

    int vertexCount = posSrc.count();
    int indexCount = (NULL != idxSrc) ? idxSrc->count() : 0;

    if (!geo.set(target, layout, vertexCount, indexCount)) {
        GrPrintf("Failed to get space for vertices!");
        return;
    }

    int texOffsets[GrDrawTarget::kMaxTexCoords];
    int colorOffset;
    int vsize = GrDrawTarget::VertexSizeAndOffsetsByIdx(layout,
                                                        texOffsets,
                                                        &colorOffset);
    void* curVertex = geo.vertices();

    for (int i = 0; i < vertexCount; ++i) {
        posSrc.writeValue(i, (GrPoint*)curVertex);

        if (texOffsets[0] > 0) {
            texCoordSrc->writeValue(i, (GrPoint*)((intptr_t)curVertex + texOffsets[0]));
        }
        if (colorOffset > 0) {
            colorSrc->writeValue(i, (GrColor*)((intptr_t)curVertex + colorOffset));
        }
        curVertex = (void*)((intptr_t)curVertex + vsize);
    }

    uint16_t* indices = (uint16_t*) geo.indices();
    for (int i = 0; i < indexCount; ++i) {
        idxSrc->writeValue(i, indices + i);
    }

    bool doAA = false;
    OffscreenRecord record;
    GrIRect bounds;

    if (-1 == texOffsets[0] && -1 == colorOffset && 
        this->doOffscreenAA(target, paint, GrIsPrimTypeLines(primitiveType))) {
        GrRect b;
        b.setBounds(geo.positions(), vertexCount);
        target->getViewMatrix().mapRect(&b);
        b.roundOut(&bounds);
        if (this->setupOffscreenAAPass1(target, false, bounds, &record)) {
            doAA = true;
        }
    }

    if (NULL == idxSrc) {
        target->drawNonIndexed(primitiveType, 0, vertexCount);
    } else {
        target->drawIndexed(primitiveType, 0, 0, vertexCount, indexCount);
    }

    if (doAA) {
        geo.set(NULL, 0, 0, 0); // have to release geom before can draw again
        this->offscreenAAPass2(target, paint, bounds, &record);
    }
}

class GrNullTexCoordSource {
public:
    void writeValue(int i, GrPoint* dstCoord) const { GrAssert(false); }
};

class GrNullColorSource {
public:
    void writeValue(int i, GrColor* dstColor) const { GrAssert(false); }
};

class GrNullIndexSource {
public:
    void writeValue(int i, uint16_t* dstIndex) const { GrAssert(false); }
    int count() const { GrAssert(false); return 0; }
};

template <typename POS_SRC>
inline void GrContext::drawCustomVertices(const GrPaint& paint,
                                          GrPrimitiveType primitiveType,
                                          const POS_SRC& posSrc) {
    this->drawCustomVertices<POS_SRC,
                             GrNullTexCoordSource,
                             GrNullColorSource,
                             GrNullIndexSource>(paint, primitiveType, posSrc,
                                                NULL, NULL, NULL);
}

template <typename POS_SRC, typename TEX_SRC>
inline void GrContext::drawCustomVertices(const GrPaint& paint,
                                          GrPrimitiveType primitiveType,
                                          const POS_SRC& posSrc,
                                          const TEX_SRC* texCoordSrc) {
    this->drawCustomVertices<POS_SRC, TEX_SRC,
                             GrNullColorSource,
                             GrNullIndexSource>(paint, primitiveType, posSrc,
                                                texCoordSrc, NULL, NULL);
}

template <typename POS_SRC, typename TEX_SRC, typename COL_SRC>
inline void GrContext::drawCustomVertices(const GrPaint& paint,
                                          GrPrimitiveType primitiveType,
                                          const POS_SRC& posSrc,
                                          const TEX_SRC* texCoordSrc,
                                          const COL_SRC* colorSrc) {
    drawCustomVertices<POS_SRC, TEX_SRC, COL_SRC,
                       GrNullIndexSource>(paint, primitiveType, posSrc, 
                                          texCoordSrc, colorSrc, NULL);
}

#endif
