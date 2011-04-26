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
    OffscreenRecord() { fEntry = NULL; }
    ~OffscreenRecord() { GrAssert(NULL == fEntry); }

    GrTextureEntry*                fEntry;
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

    GrVertexLayout layout = 0;

    GrDrawTarget::AutoReleaseGeometry geo;

    GrDrawTarget* target = this->prepareToDraw(paint, kUnbuffered_DrawCategory);

    if (NULL != paint.getTexture()) {
        if (NULL != texCoordSrc) {
            layout |= GrDrawTarget::StageTexCoordVertexLayoutBit(0,0);
        } else {
            layout |= GrDrawTarget::StagePosAsTexCoordVertexLayoutBit(0);
        }
    }

    if (NULL != colorSrc) {
        layout |= GrDrawTarget::kColor_VertexLayoutBit;
    }

    bool doOffscreenAA = false;
    OffscreenRecord record;
    if (paint.fAntiAlias &&
        !this->getRenderTarget()->isMultisampled() &&
        !(GrIsPrimTypeLines(primitiveType) && fGpu->supportsAALines()) &&
        this->setupOffscreenAAPass1(target, false, &record)) {
        doOffscreenAA = true;
        layout |= GrDrawTarget::StagePosAsTexCoordVertexLayoutBit(kOffscreenStage);
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

    if (NULL == idxSrc) {
        target->drawNonIndexed(primitiveType, 0, vertexCount);
    } else {
        target->drawIndexed(primitiveType, 0, 0, vertexCount, indexCount);
    }

    if (doOffscreenAA) {
        // draw to the offscreen
        if (NULL != indices) {
            target->drawIndexed(primitiveType, 0, 0, vertexCount, indexCount);
        } else {
            target->drawNonIndexed(primitiveType, 0, vertexCount);
        }
        // When there are custom texture coordinates we can't just draw
        // a quad to sample the offscreen. Instead we redraw the geometry to
        // specify the texture coords. This isn't quite right either, primitives
        // will only be eroded at the edges, not expanded into partial pixels.
        bool useRect = 0 == (layout & GrDrawTarget::StageTexCoordVertexLayoutBit(0,0));
        if (useRect) {
            target->setViewMatrix(GrMatrix::I());
        }
        this->setupOffscreenAAPass2(target, paint, &record);
        if (useRect) {
            geo.set(NULL, 0, 0, 0);
            int stages = (NULL != paint.getTexture()) ? 0x1 : 0x0;
            stages |= (1 << kOffscreenStage);
            GrRect dstRect(0, 0, 
                        target->getRenderTarget()->width(),
                        target->getRenderTarget()->height());
                        target->drawSimpleRect(dstRect, NULL, stages);
            target->drawSimpleRect(dstRect, NULL, stages);
        } else {
            if (NULL != indices) {
                target->drawIndexed (primitiveType, 0, 0, vertexCount, indexCount);
            } else {
                target->drawNonIndexed(primitiveType, 0, vertexCount);
            }
        }
        this->endOffscreenAA(target, &record);
    } else {
        if (NULL != indices) {
            target->drawIndexed(primitiveType, 0, 0, vertexCount, indexCount);
        } else {
            target->drawNonIndexed(primitiveType, 0, vertexCount);
        }
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
