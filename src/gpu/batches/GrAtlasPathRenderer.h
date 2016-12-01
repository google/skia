/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAtlasPathRenderer_DEFINED
#define GrAtlasPathRenderer_DEFINED

#include "GrBatchAtlas.h"
#include "GrPathRenderer.h"
#include "GrRect.h"
#include "GrShape.h"

#include "SkOpts.h"
#include "SkTDynamicHash.h"

class GrContext;

class GrAtlasPathRenderer : public GrPathRenderer {
public:
    GrAtlasPathRenderer();
    virtual ~GrAtlasPathRenderer();

private:
    StencilSupport onGetStencilSupport(const GrShape&) const override {
        return GrPathRenderer::kNoSupport_StencilSupport;
    }

    bool onCanDrawPath(const CanDrawPathArgs&) const override;

    bool onDrawPath(const DrawPathArgs&) override;

    struct ShapeData {
        class Key {
        public:
            Key() {}
            Key(const Key& that) { *this = that; }
            Key(const GrShape& shape, uint32_t dim, const SkMatrix& m) { this->set(shape, dim, m); }

            Key& operator=(const Key& that) {
                fKey.reset(that.fKey.count());
                memcpy(fKey.get(), that.fKey.get(), fKey.count() * sizeof(uint32_t));
                return *this;
            }

            void set(const GrShape& shape, uint32_t dim, const SkMatrix& viewMatrix) {
                // Shapes' keys are for their pre-style geometry, but by now we shouldn't have any
                // relevant styling information.
                SkASSERT(shape.style().isSimpleFill());
                SkASSERT(shape.hasUnstyledKey());
                int shapeKeySize = shape.unstyledKeySize();
                fKey.reset(10 + shapeKeySize);
                fKey[0] = dim;
                memcpy(&fKey[1], &viewMatrix, sizeof(SkMatrix));
                shape.writeUnstyledKey(&fKey[10]);
            }

            bool operator==(const Key& that) const {
                return fKey.count() == that.fKey.count() &&
                        0 == memcmp(fKey.get(), that.fKey.get(), sizeof(uint32_t) * fKey.count());
            }

            int count32() const { return fKey.count(); }
            const uint32_t* data() const { return fKey.get(); }

        private:
            // The key is composed of the dimensions of the DF generated for the path (32x32 max,
            // 64x64 max, 128x128 max) and the GrShape's key.
            SkAutoSTArray<24, uint32_t> fKey;
        };
        Key                   fKey;
        SkScalar              fScale;
        GrBatchAtlas::AtlasID fID;
        SkRect                fBounds;
        SkIPoint16            fAtlasLocation;
        SK_DECLARE_INTERNAL_LLIST_INTERFACE(ShapeData);

        static inline const Key& GetKey(const ShapeData& data) {
            return data.fKey;
        }

        static inline uint32_t Hash(Key key) {
            return SkOpts::hash(key.data(), sizeof(uint32_t) * key.count32());
        }
    };

    static void HandleEviction(GrBatchAtlas::AtlasID, void*);

    typedef SkTDynamicHash<ShapeData, ShapeData::Key> ShapeCache;
    typedef SkTInternalLList<ShapeData> ShapeDataList;

    std::unique_ptr<GrBatchAtlas> fAtlas;
    ShapeCache fShapeCache;
    ShapeDataList fShapeList;

    typedef GrPathRenderer INHERITED;

    friend class AtlasPathBatch;
    friend struct PathTestStruct;
};

#endif
