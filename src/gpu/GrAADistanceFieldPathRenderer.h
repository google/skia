
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
    GrAADistanceFieldPathRenderer(GrContext* context);
    virtual ~GrAADistanceFieldPathRenderer();
    
    virtual bool canDrawPath(const GrDrawTarget*,
                             const GrPipelineBuilder*,
                             const SkMatrix& viewMatrix,
                             const SkPath&,
                             const GrStrokeInfo&,
                             bool antiAlias) const override;

protected:
    virtual StencilSupport onGetStencilSupport(const GrDrawTarget*,
                                               const GrPipelineBuilder*,
                                               const SkPath&,
                                               const GrStrokeInfo&) const override;
    
    virtual bool onDrawPath(GrDrawTarget*,
                            GrPipelineBuilder*,
                            GrColor,
                            const SkMatrix& viewMatrix,
                            const SkPath&,
                            const GrStrokeInfo&,
                            bool antiAlias) override;

private:
    struct PathData {
        struct Key {
            uint32_t   fGenID;
            // rendered size for stored path (32x32 max, 64x64 max, 128x128 max)
            uint32_t   fDimension;
            bool operator==(const Key& other) const {
                return other.fGenID == fGenID && other.fDimension == fDimension;
            }
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
    
    GrContext*                         fContext;
    GrBatchAtlas*                      fAtlas;
    PathCache                          fPathCache;
    PathDataList                       fPathList;
    
    typedef GrPathRenderer INHERITED;

    friend class AADistanceFieldPathBatch;
    friend struct PathTestStruct;
};

#endif
