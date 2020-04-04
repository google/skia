/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef DEBUGLAYERMANAGER_H_
#define DEBUGLAYERMANAGER_H_

#include "include/core/SkImage.h"
#include "include/private/SkTHash.h"
#include "tools/debugger/DebugCanvas.h"

#include <vector>

// A class to assist in playing back and debugging an mskp file containing offscreen layer commands.

// Holds SkPictures necessary to draw layers in one or more DebugCanvases. During
// recording of the mskp file on Android, each layer had a RenderNode id, which is recorded with
// the layer's draw commands.
// Creates one surface (cpu only for now) for each layer, and renders
// pictures to it up to the requested command using a DebugCanvas.

// Animations are expected to, but may not always use a layer on more than frame.
// the layer may be drawn to more than once, and each different draw is saved for reconstructing the
// layer as it was on any frame. Draws may be partial, meaning their commands were clipped to not
// cover the entire layer.

// Clients may ask for a rendering of a given layer by it's RenderNode id and frame, and
// this class will return a rendering of how it looked on that frame.
// returning an SkImage snapshot of the internally managed surface.

class DebugCanvas;

class DebugLayerManager {
public:
    DebugLayerManager() {}

    // Store an SkPicture under a given nodeId (and under the currently set frame number)
    // `dirty` is the recorded rect that was used to call androidFramework_setDeviceClipRestriction
    // when the layer was drawn.
    void storeSkPicture(int nodeId, int frame, sk_sp<SkPicture> picture, SkIRect dirty);

    // Set's the command playback head for a given picture/draw event.
    void setCommand(int nodeId, int frame, int command);

    // getLayerAsImage draws the given layer as it would have looked on frame and returns an image.
    // Though each picture can be played back in as many ways as there are commands, we will let
    // that be determined by the user who sets an independent playhead for each draw event, tracked
    // here, so it stays how they left it.
    // For example: Say we are drawing a layer at frame 10.
    // Frame 0:  Layer was completely redrawn. By default we draw it to it's last command. We always
    //           save the result by (nodeId, frame)
    // Frame 5:  Layer was partially redrawn, and the user has inspected this draw event, leaving
    //           its command playhead at command 50/100. We have drew this at the time and save how
    //           the result looked (all of the commands at frame 0, then half of the commands in the
    //           partial draw at frame 5)
    // Frame 10: Another partial redraw, un-altered, drawn on top of the result from frame 5. We
    //           return this as the image of how the layer should look on frame 10
    // Frame 15: A full redraw
    //
    // If the user then comes along and moves the command playhead of the picture at frame 0,
    // we invalidate the stored images for 0, 5, and 10, but we can leave 15 alone if we have it.
    //
    // Which leaves us with one less degree of freedom to think about when implementing this
    // function: We can assume there is only one way to play back a given picture. :)
    //
    // The reason the public version of this function doesn't let you specify the frame, is that
    // I expect DebugCanvas to call it, which doesn't know which frame it's rendering. The code in
    // debugger_bindings.cpp does know, which it why I'm having it set the frame via setFrame(int)
    sk_sp<SkImage> getLayerAsImage(const int nodeId, const int frame);

    // Mean to be bindable by emscripted and returned to the javascript side
    struct DrawEventSummary {
        int nodeId;
        bool fullRedraw;
        int commandCount;
        int layerWidth;
        int layerHeight;
    };
    // Return a list summarizing the layer draw events on the current frame.
    std::vector<DrawEventSummary> summarizeEvents(int frame) const;

    // Return the list of node ids which have DrawEvents on the given frame
    std::vector<int> listNodesForFrame(int frame) const;
    // Return the list of frames on which the given node had DrawEvents.
    std::vector<int> listFramesForNode(int nodeId) const;

private:
    // This class is basically a map from (frame, node) to draw-event
    // during recording, at the beginning of any frame, one or more layers could have been drawn on.
    // every draw event was recorded, and when reading the mskp file they are stored and organized
    // here.

    struct LayerKey{
        int frame; // frame of animation on which this event was recorded.
        int nodeId; // the render node id of the layer which was drawn to.

        bool operator==(const LayerKey& b) const {
            return this->frame==b.frame && this->nodeId==b.nodeId;
        }
    };

    struct DrawEvent {
        // true the pic's clip equals the layer bounds.
        bool fullRedraw;
        // the saved result of how the layer looks on this frame.
        // null if we don't have it.
        sk_sp<SkImage> image;
        // A debug canvas used for drawing this picture.
        // the SkPicture itself isn't saved, since it's in the DebugCanvas.
        std::unique_ptr<DebugCanvas> debugCanvas;
        // the command index where the debugCanvas was left off.
        int command;
        // the size of the layer this drew into. redundant between multiple DrawEvents on the same
        // layer but helpful.
        SkISize layerBounds;
    };

    SkTHashMap<LayerKey, DrawEvent> fDraws;
    // The list of all keys in the map above (it has no keys() method)
    std::vector<LayerKey> keys;
};

#endif
