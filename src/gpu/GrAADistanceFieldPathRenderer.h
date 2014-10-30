
/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAADistanceFieldPathRenderer_DEFINED
#define GrAADistanceFieldPathRenderer_DEFINED

#include "GrAllocPool.h"
#include "GrAtlas.h"
#include "GrPathRenderer.h"
#include "GrRect.h"

#include "SkChecksum.h"

class GrContext;
class GrPlot;

class GrAADistanceFieldPathRenderer : public GrPathRenderer {
public:
    GrAADistanceFieldPathRenderer(GrContext* context);
    virtual ~GrAADistanceFieldPathRenderer();
    
    virtual bool canDrawPath(const SkPath& path,
                             const SkStrokeRec& stroke,
                             const GrDrawTarget* target,
                             bool antiAlias) const SK_OVERRIDE;

protected:
    virtual StencilSupport onGetStencilSupport(const SkPath&,
                                               const SkStrokeRec&,
                                               const GrDrawTarget*) const SK_OVERRIDE;
    
    virtual bool onDrawPath(const SkPath& path,
                            const SkStrokeRec& stroke,
                            GrDrawTarget* target,
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
    
    bool internalDrawPath(const SkPath& path, const PathData* pathData, GrDrawTarget* target);
    PathData* addPathToAtlas(const SkPath& path, const SkStrokeRec& stroke, bool antiAlias,
                             uint32_t dimension, SkScalar scale);
    bool freeUnusedPlot();
    
    typedef GrPathRenderer INHERITED;
};

#endif
