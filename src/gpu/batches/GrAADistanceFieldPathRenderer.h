/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAADistanceFieldPathRenderer_DEFINED
#define GrAADistanceFieldPathRenderer_DEFINED

#include "GrBatchAtlas.h"
#include "GrPathRenderer.h"
#include "GrRect.h"

#include "SkChecksum.h"
#include "SkTDynamicHash.h"

class GrContext;

class GrAADistanceFieldPathRenderer : public GrPathRenderer {
public:
    GrAADistanceFieldPathRenderer();
    virtual ~GrAADistanceFieldPathRenderer();

private:
    StencilSupport onGetStencilSupport(const SkPath&, const GrStrokeInfo&) const override {
        return GrPathRenderer::kNoSupport_StencilSupport;
    }

    bool onCanDrawPath(const CanDrawPathArgs&) const override;

    bool onDrawPath(const DrawPathArgs&) override;

    struct PathData {
        class Key {
        public:
            // default ctor needed for new of uninitialized PathData
            // since fStroke has no default ctor
            Key()
                : fGenID(0)
                , fDimension(0)
                , fStroke(SkStrokeRec::kFill_InitStyle) {}
            Key(uint32_t genID, uint32_t dim, const SkStrokeRec& stroke)
                : fGenID(genID)
                , fDimension(dim)
                , fStroke(stroke) {}

            bool operator==(const Key& other) const {
                return other.fGenID == fGenID && other.fDimension == fDimension &&
                       fStroke.hasEqualEffect(other.fStroke);
            }

        private:
            uint32_t   fGenID;
            // rendered size for stored path (32x32 max, 64x64 max, 128x128 max)
            uint32_t   fDimension;
            // stroking information
            SkStrokeRec fStroke;
        };
        Key                   fKey;
        SkScalar              fScale;
        GrBatchAtlas::AtlasID fID;
        SkRect                fBounds;
        SkIPoint16            fAtlasLocation;
        SK_DECLARE_INTERNAL_LLIST_INTERFACE(PathData);

        static inline const Key& GetKey(const PathData& data) {
            return data.fKey;
        }

        static inline uint32_t Hash(Key key) {
            return SkChecksum::Murmur3(reinterpret_cast<const uint32_t*>(&key), sizeof(key));
        }
    };

    static void HandleEviction(GrBatchAtlas::AtlasID, void*);

    typedef SkTDynamicHash<PathData, PathData::Key> PathCache;
    typedef SkTInternalLList<PathData> PathDataList;

    GrBatchAtlas*                      fAtlas;
    PathCache                          fPathCache;
    PathDataList                       fPathList;

    typedef GrPathRenderer INHERITED;

    friend class AADistanceFieldPathBatch;
    friend struct PathTestStruct;
};

#endif
