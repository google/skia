
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrLayerAtlas_DEFINED
#define GrLayerAtlas_DEFINED

#include "GrTexture.h"

#include "SkPoint.h"
#include "SkTDArray.h"
#include "SkTInternalLList.h"

class GrLayerAtlas;
class GrTextureProvider;
class GrRectanizer;

// The backing GrTexture for a GrLayerAtlas is broken into a spatial grid of Plots. When
// the atlas needs space on the texture (i.e., in response to an addToAtlas call), it 
// iterates through the plots in use by the requesting client looking for space and, 
// if no space is found, opens up a new Plot for that client. The Plots keep track of 
// subimage placement via their GrRectanizer. 
//
// If all Plots are full, the replacement strategy is up to the client. The Plot::reset
// call will remove a Plot's knowledge of any allocated rects - freeing its space for reuse.

class GrLayerAtlas {
public:
    class Plot {
        SK_DECLARE_INTERNAL_LLIST_INTERFACE(Plot); // In an MRU llist

    public:
        // This returns a plot ID unique to each plot in the atlas. They are
        // consecutive and start at 0.
        int id() const { return fID; }

        void reset();

    private:
        friend class GrLayerAtlas;

        Plot();
        ~Plot(); // does not try to delete the fNext field

        void init(int id, int offX, int offY, int width, int height);

        bool allocateRect(int width, int height, SkIPoint16*);

        int                     fID;
        GrRectanizer*           fRects;
        SkIPoint16              fOffset;        // the offset of the plot in the backing texture
    };

    // This class allows each client to independently track the Plots in
    // which its data is stored.
    // For example, multiple pictures may simultaneously store their layers in the 
    // layer atlas. When a picture goes away it can use the ClientPlotUsage to remove itself
    // from those plots.
    class ClientPlotUsage {
    public:
        ClientPlotUsage(int maxPlots)
            SkDEBUGCODE(: fMaxPlots(maxPlots)) {
            fPlots.setReserve(maxPlots);
        }

        bool isEmpty() const { return 0 == fPlots.count(); }

        int numPlots() const { return fPlots.count(); }
        Plot* plot(int index) { return fPlots[index]; }

        void appendPlot(Plot* plot) {
            SkASSERT(fPlots.count() <= fMaxPlots);
            SkASSERT(!fPlots.contains(plot));
            *fPlots.append() = plot;
        }

        // remove reference to 'plot'
        void removePlot(const Plot* plot) {
            int index = fPlots.find(const_cast<Plot*>(plot));
            if (index >= 0) {
                fPlots.remove(index);
            }
        }

#ifdef SK_DEBUG
        bool contains(const Plot* plot) const { 
            return fPlots.contains(const_cast<Plot*>(plot)); 
        }
#endif

    private:
        SkTDArray<Plot*> fPlots;
        SkDEBUGCODE(int fMaxPlots;)
    };

    GrLayerAtlas(GrTextureProvider*, GrPixelConfig, GrSurfaceFlags flags, 
                 const SkISize& backingTextureSize,
                 int numPlotsX, int numPlotsY);
    ~GrLayerAtlas();

    // Requests a width x height block in the atlas. Upon success it returns 
    // the containing Plot and absolute location in the backing texture. 
    // nullptr is returned if there is no more space in the atlas.
    Plot* addToAtlas(ClientPlotUsage*, int width, int height, SkIPoint16* loc);

    GrTexture* getTextureOrNull() const {
        return fTexture;
    }

    GrTexture* getTexture() const {
        SkASSERT(fTexture);
        return fTexture;
    }

    bool reattachBackingTexture();

    void detachBackingTexture() {
        fTexture.reset(nullptr);
    }

    void resetPlots();

    enum IterOrder {
        kLRUFirst_IterOrder,
        kMRUFirst_IterOrder
    };

    typedef SkTInternalLList<Plot> PlotList;
    typedef PlotList::Iter PlotIter;
    Plot* iterInit(PlotIter* iter, IterOrder order) {
        return iter->init(fPlotList, kLRUFirst_IterOrder == order
                                                       ? PlotList::Iter::kTail_IterStart
                                                       : PlotList::Iter::kHead_IterStart);
    }

private:
    void createBackingTexture();

    void makeMRU(Plot* plot);

    GrTextureProvider* fTexProvider;
    GrPixelConfig      fPixelConfig;
    GrSurfaceFlags     fFlags;
    SkAutoTUnref<GrTexture> fTexture;

    SkISize            fBackingTextureSize;

    // allocated array of Plots
    Plot*              fPlotArray;
    // LRU list of Plots (MRU at head - LRU at tail)
    PlotList           fPlotList;
};

#endif
