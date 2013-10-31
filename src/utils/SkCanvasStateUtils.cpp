/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvasStateUtils.h"

#include "SkBitmapDevice.h"
#include "SkCanvas.h"
#include "SkCanvasStack.h"
#include "SkErrorInternals.h"
#include "SkWriter32.h"

#define CANVAS_STATE_VERSION 1
/*
 * WARNING: The structs below are part of a stable ABI and as such we explicitly
 * use unambigious primitives (e.g. int32_t instead of an enum).
 *
 * ANY CHANGES TO THE STRUCTS BELOW THAT IMPACT THE ABI SHOULD RESULT IN AN
 * UPDATE OF THE CANVAS_STATE_VERSION. SUCH CHANGES SHOULD ONLY BE MADE IF
 * ABSOLUTELY NECESSARY!
 */
enum RasterConfigs {
  kUnknown_RasterConfig   = 0,
  kRGB_565_RasterConfig   = 1,
  kARGB_8888_RasterConfig = 2
};
typedef int32_t RasterConfig;

enum CanvasBackends {
    kUnknown_CanvasBackend = 0,
    kRaster_CanvasBackend  = 1,
    kGPU_CanvasBackend     = 2,
    kPDF_CanvasBackend     = 3
};
typedef int32_t CanvasBackend;

struct ClipRect {
    int32_t left, top, right, bottom;
};

struct SkMCState {
    float matrix[9];
    // NOTE: this only works for non-antialiased clips
    int32_t clipRectCount;
    ClipRect* clipRects;
};

// NOTE: If you add more members, bump CanvasState::version.
struct SkCanvasLayerState {
    CanvasBackend type;
    int32_t x, y;
    int32_t width;
    int32_t height;

    SkMCState mcState;

    union {
        struct {
            RasterConfig config; // pixel format: a value from RasterConfigs.
            size_t rowBytes;     // Number of bytes from start of one line to next.
            void* pixels;        // The pixels, all (height * rowBytes) of them.
        } raster;
        struct {
            int32_t textureID;
        } gpu;
    };
};

class SkCanvasState {
public:
    SkCanvasState(SkCanvas* canvas) {
        SkASSERT(canvas);
        version = CANVAS_STATE_VERSION;
        width = canvas->getDeviceSize().width();
        height = canvas->getDeviceSize().height();
        layerCount = 0;
        layers = NULL;
        originalCanvas = SkRef(canvas);

        mcState.clipRectCount = 0;
        mcState.clipRects = NULL;
    }

    ~SkCanvasState() {
        // loop through the layers and free the data allocated to the clipRects
        for (int i = 0; i < layerCount; ++i) {
            sk_free(layers[i].mcState.clipRects);
        }

        sk_free(mcState.clipRects);
        sk_free(layers);

        // it is now safe to free the canvas since there should be no remaining
        // references to the content that is referenced by this canvas (e.g. pixels)
        originalCanvas->unref();
    }

    /**
     * The version this struct was built with.  This field must always appear
     * first in the struct so that when the versions don't match (and the
     * remaining contents and size are potentially different) we can still
     * compare the version numbers.
     */
    int32_t version;

    int32_t width;
    int32_t height;

    SkMCState mcState;

    int32_t layerCount;
    SkCanvasLayerState* layers;

private:
    SkCanvas* originalCanvas;
};

////////////////////////////////////////////////////////////////////////////////

class ClipValidator : public SkCanvas::ClipVisitor {
public:
    ClipValidator() : fFailed(false) {}
    bool failed() { return fFailed; }

    // ClipVisitor
    virtual void clipRect(const SkRect& rect, SkRegion::Op op, bool antialias) SK_OVERRIDE {
        fFailed |= antialias;
    }

    virtual void clipPath(const SkPath&, SkRegion::Op, bool antialias) SK_OVERRIDE {
        fFailed |= antialias;
    }

private:
    bool fFailed;
};

static void setup_MC_state(SkMCState* state, const SkMatrix& matrix, const SkRegion& clip) {
    // initialize the struct
    state->clipRectCount = 0;

    // capture the matrix
    for (int i = 0; i < 9; i++) {
        state->matrix[i] = matrix.get(i);
    }

    /*
     * capture the clip
     *
     * storage is allocated on the stack for the first 4 rects. This value was
     * chosen somewhat arbitrarily, but does allow us to represent simple clips
     * and some more common complex clips (e.g. a clipRect with a sub-rect
     * clipped out of its interior) without needing to malloc any additional memory.
     */
    const int clipBufferSize = 4 * sizeof(ClipRect);
    char clipBuffer[clipBufferSize];
    SkWriter32 clipWriter(sizeof(ClipRect), clipBuffer, clipBufferSize);

    if (!clip.isEmpty()) {
        // only returns the b/w clip so aa clips fail
        SkRegion::Iterator clip_iterator(clip);
        for (; !clip_iterator.done(); clip_iterator.next()) {
            // this assumes the SkIRect is stored in l,t,r,b ordering which
            // matches the ordering of our ClipRect struct
            clipWriter.writeIRect(clip_iterator.rect());
            state->clipRectCount++;
        }
    }

    // allocate memory for the clip then and copy them to the struct
    state->clipRects = (ClipRect*) sk_malloc_throw(clipWriter.bytesWritten());
    clipWriter.flatten(state->clipRects);
}



SkCanvasState* SkCanvasStateUtils::CaptureCanvasState(SkCanvas* canvas) {
    SkASSERT(canvas);

    // Check the clip can be decomposed into rectangles (i.e. no soft clips).
    ClipValidator validator;
    canvas->replayClips(&validator);
    if (validator.failed()) {
        SkErrorInternals::SetError(kInvalidOperation_SkError,
                "CaptureCanvasState does not support canvases with antialiased clips.\n");
        return NULL;
    }

    SkAutoTDelete<SkCanvasState> canvasState(SkNEW_ARGS(SkCanvasState, (canvas)));

    // decompose the total matrix and clip
    setup_MC_state(&canvasState->mcState, canvas->getTotalMatrix(), canvas->getTotalClip());

    /*
     * decompose the layers
     *
     * storage is allocated on the stack for the first 3 layers. It is common in
     * some view systems (e.g. Android) that a few non-clipped layers are present
     * and we will not need to malloc any additional memory in those cases.
     */
    const int layerBufferSize = 3 * sizeof(SkCanvasLayerState);
    char layerBuffer[layerBufferSize];
    SkWriter32 layerWriter(sizeof(SkCanvasLayerState), layerBuffer, layerBufferSize);
    int layerCount = 0;
    for (SkCanvas::LayerIter layer(canvas, true/*skipEmptyClips*/); !layer.done(); layer.next()) {

        // we currently only work for bitmap backed devices
        const SkBitmap& bitmap = layer.device()->accessBitmap(true/*changePixels*/);
        if (bitmap.empty() || bitmap.isNull() || !bitmap.lockPixelsAreWritable()) {
            return NULL;
        }

        SkCanvasLayerState* layerState =
                (SkCanvasLayerState*) layerWriter.reserve(sizeof(SkCanvasLayerState));
        layerState->type = kRaster_CanvasBackend;
        layerState->x = layer.x();
        layerState->y = layer.y();
        layerState->width = bitmap.width();
        layerState->height = bitmap.height();

        switch (bitmap.config()) {
            case SkBitmap::kARGB_8888_Config:
                layerState->raster.config = kARGB_8888_RasterConfig;
                break;
            case SkBitmap::kRGB_565_Config:
                layerState->raster.config = kRGB_565_RasterConfig;
                break;
            default:
                return NULL;
        }
        layerState->raster.rowBytes = bitmap.rowBytes();
        layerState->raster.pixels = bitmap.getPixels();

        setup_MC_state(&layerState->mcState, layer.matrix(), layer.clip());
        layerCount++;
    }

    // allocate memory for the layers and then and copy them to the struct
    SkASSERT(layerWriter.bytesWritten() == layerCount * sizeof(SkCanvasLayerState));
    canvasState->layerCount = layerCount;
    canvasState->layers = (SkCanvasLayerState*) sk_malloc_throw(layerWriter.bytesWritten());
    layerWriter.flatten(canvasState->layers);

    // for now, just ignore any client supplied DrawFilter.
    if (canvas->getDrawFilter()) {
//        SkDEBUGF(("CaptureCanvasState will ignore the canvases draw filter.\n"));
    }

    return canvasState.detach();
}

////////////////////////////////////////////////////////////////////////////////

static void setup_canvas_from_MC_state(const SkMCState& state, SkCanvas* canvas) {
    // reconstruct the matrix
    SkMatrix matrix;
    for (int i = 0; i < 9; i++) {
        matrix.set(i, state.matrix[i]);
    }

    // reconstruct the clip
    SkRegion clip;
    for (int i = 0; i < state.clipRectCount; ++i) {
        clip.op(SkIRect::MakeLTRB(state.clipRects[i].left,
                                  state.clipRects[i].top,
                                  state.clipRects[i].right,
                                  state.clipRects[i].bottom),
                SkRegion::kUnion_Op);
    }

    canvas->setMatrix(matrix);
    canvas->setClipRegion(clip);
}

static SkCanvas* create_canvas_from_canvas_layer(const SkCanvasLayerState& layerState) {
    SkASSERT(kRaster_CanvasBackend == layerState.type);

    SkBitmap bitmap;
    SkBitmap::Config config =
        layerState.raster.config == kARGB_8888_RasterConfig ? SkBitmap::kARGB_8888_Config :
        layerState.raster.config == kRGB_565_RasterConfig ? SkBitmap::kRGB_565_Config :
        SkBitmap::kNo_Config;

    if (config == SkBitmap::kNo_Config) {
        return NULL;
    }

    bitmap.setConfig(config, layerState.width, layerState.height,
                     layerState.raster.rowBytes);
    bitmap.setPixels(layerState.raster.pixels);

    SkASSERT(!bitmap.empty());
    SkASSERT(!bitmap.isNull());

    // create a device & canvas
    SkAutoTUnref<SkBitmapDevice> device(SkNEW_ARGS(SkBitmapDevice, (bitmap)));
    SkAutoTUnref<SkCanvas> canvas(SkNEW_ARGS(SkCanvas, (device.get())));

    // setup the matrix and clip
    setup_canvas_from_MC_state(layerState.mcState, canvas.get());

    return canvas.detach();
}

SkCanvas* SkCanvasStateUtils::CreateFromCanvasState(const SkCanvasState* state) {
    SkASSERT(state);

    // check that the versions match
    if (CANVAS_STATE_VERSION != state->version) {
        SkDebugf("CreateFromCanvasState version does not match the one use to create the input");
        return NULL;
    }

    if (state->layerCount < 1) {
        return NULL;
    }

    SkAutoTUnref<SkCanvasStack> canvas(SkNEW_ARGS(SkCanvasStack, (state->width, state->height)));

    // setup the matrix and clip on the n-way canvas
    setup_canvas_from_MC_state(state->mcState, canvas);

    // Iterate over the layers and add them to the n-way canvas
    for (int i = state->layerCount - 1; i >= 0; --i) {
        SkAutoTUnref<SkCanvas> canvasLayer(create_canvas_from_canvas_layer(state->layers[i]));
        if (!canvasLayer.get()) {
            return NULL;
        }
        canvas->pushCanvas(canvasLayer.get(), SkIPoint::Make(state->layers[i].x,
                                                             state->layers[i].y));
    }

    return canvas.detach();
}

////////////////////////////////////////////////////////////////////////////////

void SkCanvasStateUtils::ReleaseCanvasState(SkCanvasState* state) {
    SkDELETE(state);
}
