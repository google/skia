
/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAADistanceFieldPathRenderer_DEFINED
#define GrAADistanceFieldPathRenderer_DEFINED

#include "GrAtlas.h"
#include "GrPathRenderer.h"
#include "GrRect.h"

#include "SkChecksum.h"
#include "SkTDynamicHash.h"

class GrContext;
class GrPlot;

class GrAADistanceFieldPathRenderer : public GrPathRenderer {
public:
    GrAADistanceFieldPathRenderer(GrContext* context);
    virtual ~GrAADistanceFieldPathRenderer();
    
    virtual bool canDrawPath(const GrDrawTarget*,
                             const GrPipelineBuilder*,
                             const SkMatrix& viewMatrix,
                             const SkPath&,
                             const SkStrokeRec&,
                             bool antiAlias) const SK_OVERRIDE;

protected:
    virtual StencilSupport onGetStencilSupport(const GrDrawTarget*,
                                               const GrPipelineBuilder*,
                                               const SkPath&,
                                               const SkStrokeRec&) const SK_OVERRIDE;
    
    virtual bool onDrawPath(GrDrawTarget*,
                            GrPipelineBuilder*,
                            GrColor,
                            const SkMatrix& viewMatrix,
                            const SkPath&,
                            const SkStrokeRec&,
                            bool antiAlias) SK_OVERRIDE;

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
        Key        fKey;
        SkScalar   fScale;
        GrPlot*    fPlot;
        SkRect     fBounds;
        SkIPoint16 fAtlasLocation;
        SK_DECLARE_INTERNAL_LLIST_INTERFACE(PathData);
        
        static inline const Key& GetKey(const PathData& data) {
            return data.fKey;
        }
        
        static inline uint32_t Hash(Key key) {
            return SkChecksum::Murmur3(reinterpret_cast<const uint32_t*>(&key), sizeof(key));
        }
    };
    typedef SkTInternalLList<PathData> PathDataList;
    
    GrContext*                         fContext;
    GrAtlas*                           fAtlas;
    SkAutoTUnref<GrGeometryProcessor>  fCachedGeometryProcessor;
    // current set of flags used to create the cached geometry processor
    uint32_t                           fEffectFlags;
    GrAtlas::ClientPlotUsage           fPlotUsage;
    SkTDynamicHash<PathData, PathData::Key> fPathCache;
    PathDataList                       fPathList;
    
    bool internalDrawPath(GrDrawTarget*, GrPipelineBuilder*, GrColor, const SkMatrix& viewMatrix,
                          const SkPath& path, const PathData* pathData);
    PathData* addPathToAtlas(const SkPath& path, const SkStrokeRec& stroke, bool antiAlias,
                             uint32_t dimension, SkScalar scale);
    bool freeUnusedPlot();
    
    typedef GrPathRenderer INHERITED;
};

#endif
