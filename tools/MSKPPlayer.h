/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef MSKPPlayer_DEFINED
#define MSKPPlayer_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"

#include <unordered_map>
#include <vector>

class SkCanvas;
class SkImage;
class SkStreamSeekable;
class SkSurface;

/**
 * Plays frames/pages of a MSKP to a canvas. This class uses the term "frame" as though the MSKP
 * contains an animation, though it could indeed contain pages of a static document.
 */
class MSKPPlayer {
public:
    ~MSKPPlayer();

    /** Make a player from a MSKP stream, or null if stream can't be read as MSKP. */
    static std::unique_ptr<MSKPPlayer> Make(SkStreamSeekable* stream);

    /** Maximum width and height across all frames. */
    SkISize maxDimensions() const { return fMaxDimensions; }

    /** Total number of frames. */
    int numFrames() const { return static_cast<int>(fRootLayers.size()); }

    /** Size of an individual frame. */
    SkISize frameDimensions(int i) const;

    /**
     * Plays a frame into the passed canvas. Frames can be randomly accessed. Offscreen layers are
     * incrementally updated from their current state to the state required for the frame
     * (redrawing from scratch if their current state is ahead of the passed frame index).
     */
    bool playFrame(SkCanvas* canvas, int i);

    /** Destroys any cached offscreen layers. */
    void resetLayers();

    /**
     * Forces all offscreen layers to re-render the next time they're required for a frame but
     * preserves the backing stores for them if already allocated.
     */
    void rewindLayers();

    /**
     * Creates backing stores for any offscreen layers using the passed canvas's makeSurface().
     * Existing layers that match the canvas's recording context are not reallocated or rewound.
     */
    void allocateLayers(SkCanvas*);

    /**
     * A set of IDs of offscreen layers in no particular order. If frame value >= 0 is specified
     * then the layer set is filtered to layers used by that frame (or empty if >= numFrames). If
     * < 0 then gathers all the layers across all frames.
     */
    std::vector<int> layerIDs(int frame = -1) const;

    /**
     * Gets the contents of an offscreen layer. It's contents will depend on current playback state
     * (playFrame(), updateFrameLayers(), resetLayers()). If the layer currently has no backing
     * store because it hasn't been drawn or resetLayers() was called then this will return nullptr.
     * Layer contents are not affected by rewindLayers() as that simply lazily redraws the frame
     * contents the next time it is required by playFrame*() or updateFrameLayers().
     */
    sk_sp<SkImage> layerSnapshot(int layerID) const;

private:
    MSKPPlayer() = default;
    // noncopyable, nonmoveable.
    MSKPPlayer(const MSKPPlayer&) = delete;
    MSKPPlayer(MSKPPlayer&&) = delete;
    MSKPPlayer& operator=(const MSKPPlayer&) = delete;
    MSKPPlayer& operator=(MSKPPlayer&&) = delete;

    // Cmds are used to draw content to the frame root layer and to offscreen layers.
    struct Cmd;
    // Draws a SkPicture.
    struct PicCmd;
    // Draws another layer. Stores the ID of the layer to draw and what command index on that
    // layer should be current when the layer is drawn. The layer contents are updated to the
    // stored command index before the layer is drawn.
    struct DrawLayerCmd;

    // The commands for a root/offscreen layer and dimensions of the layer.
    struct LayerCmds {
        LayerCmds() = default;
        LayerCmds(LayerCmds&&) = default;
        SkISize fDimensions;
        std::vector<std::unique_ptr<Cmd>> fCmds;
    };

    // Playback state of layer: the last command index drawn to it and the SkSurface with contents.
    struct LayerState {
        size_t fCurrCmd = -1;
        sk_sp<SkSurface> fSurface;
    };

    static sk_sp<SkSurface> MakeSurfaceForLayer(const LayerCmds&, SkCanvas* rootCanvas);

    void collectReferencedLayers(const LayerCmds& layer, std::vector<int>*) const;

    // MSKP layer ID -> LayerCmds
    using LayerMap = std::unordered_map<int, LayerCmds>;
    // MSKP layer ID -> LayerState
    using LayerStateMap = std::unordered_map<int, LayerState>;

    /**
     * A SkCanvas that consumes the SkPicture and records Cmds into a Layer. It will spawn
     * additional Layers and record nested SkPictures into those using additional CmdRecordCanvas
     * CmdRecordCanvas instances. It needs access to fOffscreenLayers to create and update LayerCmds
     * structs for offscreen layers.
     */
    class CmdRecordCanvas;

    SkISize            fMaxDimensions = {0, 0};  // Max dimensions across all frames.
    LayerMap           fOffscreenLayers;         // All the offscreen layers for all frames.
    LayerStateMap      fOffscreenLayerStates;    // Current surfaces and command idx for offscreen
                                                 // layers
    std::vector<LayerCmds> fRootLayers;          // One root layer for each frame.
};

#endif
