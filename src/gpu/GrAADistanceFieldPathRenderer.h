
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
    GrAADistanceFieldPathRenderer(GrContext* context)
        : fContext(context)
        , fAtlas(NULL) {
    }
    
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
        uint32_t   fGenID;
        GrPlot*    fPlot;
        SkRect     fBounds;
        SkIPoint16 fAtlasLocation;
        SK_DECLARE_INTERNAL_LLIST_INTERFACE(PathData);
        
        static inline const uint32_t& GetKey(const PathData& data) {
            return data.fGenID;
        }
        
        static inline uint32_t Hash(uint32_t key) {
            return SkChecksum::Murmur3(&key, sizeof(key));
        }
    };
    typedef SkTInternalLList<PathData> PathDataList;
    
    GrContext*                         fContext;
    GrAtlas*                           fAtlas;
    GrAtlas::ClientPlotUsage           fPlotUsage;
    SkTDynamicHash<PathData, uint32_t> fPathCache;
    PathDataList                       fPathList;
    
    bool internalDrawPath(const SkPath& path, const PathData* pathData, GrDrawTarget* target);
    PathData* addPathToAtlas(const SkPath& path, const SkStrokeRec& stroke, bool antiAlias);
    bool freeUnusedPlot();
    
    typedef GrPathRenderer INHERITED;
};

#endif
