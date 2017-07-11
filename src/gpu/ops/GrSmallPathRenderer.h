/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSmallPathRenderer_DEFINED
#define GrSmallPathRenderer_DEFINED

#include "GrDrawOpAtlas.h"
#include "GrPathRenderer.h"
#include "GrRect.h"
#include "GrShape.h"

#include "SkOpts.h"
#include "SkTDynamicHash.h"

class GrContext;

class GrSmallPathRenderer : public GrPathRenderer {
public:
    GrSmallPathRenderer();
    ~GrSmallPathRenderer() override;

    class SmallPathOp;
    struct PathTestStruct;

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
            Key(const GrShape& shape, uint32_t dim) { this->set(shape, dim); }
            Key(const GrShape& shape, const SkMatrix& ctm) { this->set(shape, ctm); }

            Key& operator=(const Key& that) {
                fKey.reset(that.fKey.count());
                memcpy(fKey.get(), that.fKey.get(), fKey.count() * sizeof(uint32_t));
                return *this;
            }

            void set(const GrShape& shape, uint32_t dim) {
                // Shapes' keys are for their pre-style geometry, but by now we shouldn't have any
                // relevant styling information.
                SkASSERT(shape.style().isSimpleFill());
                SkASSERT(shape.hasUnstyledKey());
                int shapeKeySize = shape.unstyledKeySize();
                fKey.reset(1 + shapeKeySize);
                fKey[0] = dim;
                shape.writeUnstyledKey(&fKey[1]);
            }

            void set(const GrShape& shape, const SkMatrix& ctm) {
                GrUniqueKey maskKey;
                struct KeyData {
                    SkScalar fFractionalTranslateX;
                    SkScalar fFractionalTranslateY;
                };

                // Shapes' keys are for their pre-style geometry, but by now we shouldn't have any
                // relevant styling information.
                SkASSERT(shape.style().isSimpleFill());
                SkASSERT(shape.hasUnstyledKey());
                // We require the upper left 2x2 of the matrix to match exactly for a cache hit.
                SkScalar sx = ctm.get(SkMatrix::kMScaleX);
                SkScalar sy = ctm.get(SkMatrix::kMScaleY);
                SkScalar kx = ctm.get(SkMatrix::kMSkewX);
                SkScalar ky = ctm.get(SkMatrix::kMSkewY);
                SkScalar tx = ctm.get(SkMatrix::kMTransX);
                SkScalar ty = ctm.get(SkMatrix::kMTransY);
                // Allow 8 bits each in x and y of subpixel positioning.
                SkFixed fracX = SkScalarToFixed(SkScalarFraction(tx)) & 0x0000FF00;
                SkFixed fracY = SkScalarToFixed(SkScalarFraction(ty)) & 0x0000FF00;
                int shapeKeySize = shape.unstyledKeySize();
                fKey.reset(5 + shapeKeySize);
                fKey[0] = SkFloat2Bits(sx);
                fKey[1] = SkFloat2Bits(sy);
                fKey[2] = SkFloat2Bits(kx);
                fKey[3] = SkFloat2Bits(ky);
                fKey[4] = fracX | (fracY >> 8);
                shape.writeUnstyledKey(&fKey[5]);
            }

            bool operator==(const Key& that) const {
                return fKey.count() == that.fKey.count() &&
                        0 == memcmp(fKey.get(), that.fKey.get(), sizeof(uint32_t) * fKey.count());
            }

            int count32() const { return fKey.count(); }
            const uint32_t* data() const { return fKey.get(); }

        private:
            // The key is composed of the GrShape's key, and either the dimensions of the DF
            // generated for the path (32x32 max, 64x64 max, 128x128 max) if an SDF image or
            // the matrix for the path with only fractional translation.
            SkAutoSTArray<24, uint32_t> fKey;
        };
        Key fKey;
        GrDrawOpAtlas::AtlasID fID;
        SkRect   fBounds;
        SkScalar fScale;
        SkVector fTranslate;
        SK_DECLARE_INTERNAL_LLIST_INTERFACE(ShapeData);

        static inline const Key& GetKey(const ShapeData& data) {
            return data.fKey;
        }

        static inline uint32_t Hash(Key key) {
            return SkOpts::hash(key.data(), sizeof(uint32_t) * key.count32());
        }
    };

    static void HandleEviction(GrDrawOpAtlas::AtlasID, void*);

    typedef SkTDynamicHash<ShapeData, ShapeData::Key> ShapeCache;
    typedef SkTInternalLList<ShapeData> ShapeDataList;

    std::unique_ptr<GrDrawOpAtlas> fAtlas;
    ShapeCache fShapeCache;
    ShapeDataList fShapeList;

    typedef GrPathRenderer INHERITED;
};

#endif
