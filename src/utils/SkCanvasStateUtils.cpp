/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvasStateUtils.h"

#include "SkCanvas.h"
#include "SkCanvasStack.h"
#include "SkDevice.h"
#include "SkErrorInternals.h"
#include "SkWriter32.h"

/*
 * WARNING: The structs below are part of a stable ABI and as such we explicitly
 * use unambigious primitives (e.g. int32_t instead of an enum).
 *
 * ANY CHANGES TO THE STRUCTS BELOW THAT IMPACT THE ABI SHOULD RESULT IN A NEW
 * NEW SUBCLASS OF SkCanvasState. SUCH CHANGES SHOULD ONLY BE MADE IF ABSOLUTELY
 * NECESSARY!
 *
 * In order to test changes, run the CanvasState tests. gyp/canvas_state_lib.gyp
 * describes how to create a library to pass to the CanvasState tests. The tests
 * should succeed when building the library with your changes and passing that to
 * the tests running in the unchanged Skia.
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

// NOTE: If you add more members, create a new subclass of SkCanvasState with a
// new CanvasState::version.
struct SkCanvasLayerState {
    CanvasBackend type;
    int32_t x, y;
    int32_t width;
    int32_t height;

    SkMCState mcState;

    union {
        struct {
            RasterConfig config; // pixel format: a value from RasterConfigs.
            uint64_t rowBytes;   // Number of bytes from start of one line to next.
            void* pixels;        // The pixels, all (height * rowBytes) of them.
        } raster;
        struct {
            int32_t textureID;
        } gpu;
    };
};

class SkCanvasState {
public:
    SkCanvasState(int32_t version, SkCanvas* canvas) {
        SkASSERT(canvas);
        this->version = version;
        width = canvas->getBaseLayerSize().width();
        height = canvas->getBaseLayerSize().height();

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
    int32_t alignmentPadding;
};

class SkCanvasState_v1 : public SkCanvasState {
public:
    static const int32_t kVersion = 1;

    SkCanvasState_v1(SkCanvas* canvas)
    : INHERITED(kVersion, canvas)
    {
        layerCount = 0;
        layers = nullptr;
        mcState.clipRectCount = 0;
        mcState.clipRects = nullptr;
        originalCanvas = SkRef(canvas);
    }

    ~SkCanvasState_v1() {
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

    SkMCState mcState;

    int32_t layerCount;
    SkCanvasLayerState* layers;
private:
    SkCanvas* originalCanvas;
    typedef SkCanvasState INHERITED;
};

////////////////////////////////////////////////////////////////////////////////

class ClipValidator : public SkCanvas::ClipVisitor {
public:
    ClipValidator() : fFailed(false) {}
    bool failed() { return fFailed; }

    // ClipVisitor
    void clipRect(const SkRect& rect, SkRegion::Op op, bool antialias) override {
        fFailed |= antialias;
    }

    void clipRRect(const SkRRect& rrect, SkRegion::Op op, bool antialias) override {
        fFailed |= antialias;
    }

    void clipPath(const SkPath&, SkRegion::Op, bool antialias) override {
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
    SkSWriter32<4*sizeof(ClipRect)> clipWriter;

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
        return nullptr;
    }

    SkAutoTDelete<SkCanvasState_v1> canvasState(new SkCanvasState_v1(canvas));

    // decompose the total matrix and clip
    setup_MC_state(&canvasState->mcState, canvas->getTotalMatrix(),
                   canvas->internal_private_getTotalClip());

    /*
     * decompose the layers
     *
     * storage is allocated on the stack for the first 3 layers. It is common in
     * some view systems (e.g. Android) that a few non-clipped layers are present
     * and we will not need to malloc any additional memory in those cases.
     */
    SkSWriter32<3*sizeof(SkCanvasLayerState)> layerWriter;
    int layerCount = 0;
    for (SkCanvas::LayerIter layer(canvas, true/*skipEmptyClips*/); !layer.done(); layer.next()) {

        // we currently only work for bitmap backed devices
        SkPixmap pmap;
        if (!layer.device()->accessPixels(&pmap) || 0 == pmap.width() || 0 == pmap.height()) {
            return nullptr;
        }

        SkCanvasLayerState* layerState =
                (SkCanvasLayerState*) layerWriter.reserve(sizeof(SkCanvasLayerState));
        layerState->type = kRaster_CanvasBackend;
        layerState->x = layer.x();
        layerState->y = layer.y();
        layerState->width = pmap.width();
        layerState->height = pmap.height();

        switch (pmap.colorType()) {
            case kN32_SkColorType:
                layerState->raster.config = kARGB_8888_RasterConfig;
                break;
            case kRGB_565_SkColorType:
                layerState->raster.config = kRGB_565_RasterConfig;
                break;
            default:
                return nullptr;
        }
        layerState->raster.rowBytes = pmap.rowBytes();
        layerState->raster.pixels = pmap.writable_addr();

        setup_MC_state(&layerState->mcState, layer.matrix(), layer.clip());
        layerCount++;
    }

    // allocate memory for the layers and then and copy them to the struct
    SkASSERT(layerWriter.bytesWritten() == layerCount * sizeof(SkCanvasLayerState));
    canvasState->layerCount = layerCount;
    canvasState->layers = (SkCanvasLayerState*) sk_malloc_throw(layerWriter.bytesWritten());
    layerWriter.flatten(canvasState->layers);

    return canvasState.release();
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
    SkColorType colorType =
        layerState.raster.config == kARGB_8888_RasterConfig ? kN32_SkColorType :
        layerState.raster.config == kRGB_565_RasterConfig ? kRGB_565_SkColorType :
        kUnknown_SkColorType;

    if (colorType == kUnknown_SkColorType) {
        return nullptr;
    }

    bitmap.installPixels(SkImageInfo::Make(layerState.width, layerState.height,
                                           colorType, kPremul_SkAlphaType),
                         layerState.raster.pixels, (size_t) layerState.raster.rowBytes);

    SkASSERT(!bitmap.empty());
    SkASSERT(!bitmap.isNull());

    SkAutoTUnref<SkCanvas> canvas(new SkCanvas(bitmap));

    // setup the matrix and clip
    setup_canvas_from_MC_state(layerState.mcState, canvas.get());

    return canvas.release();
}

SkCanvas* SkCanvasStateUtils::CreateFromCanvasState(const SkCanvasState* state) {
    SkASSERT(state);
    // Currently there is only one possible version.
    SkASSERT(SkCanvasState_v1::kVersion == state->version);

    const SkCanvasState_v1* state_v1 = static_cast<const SkCanvasState_v1*>(state);

    if (state_v1->layerCount < 1) {
        return nullptr;
    }

    SkAutoTUnref<SkCanvasStack> canvas(new SkCanvasStack(state->width, state->height));

    // setup the matrix and clip on the n-way canvas
    setup_canvas_from_MC_state(state_v1->mcState, canvas);

    // Iterate over the layers and add them to the n-way canvas
    for (int i = state_v1->layerCount - 1; i >= 0; --i) {
        SkAutoTUnref<SkCanvas> canvasLayer(create_canvas_from_canvas_layer(state_v1->layers[i]));
        if (!canvasLayer.get()) {
            return nullptr;
        }
        canvas->pushCanvas(canvasLayer.get(), SkIPoint::Make(state_v1->layers[i].x,
                                                             state_v1->layers[i].y));
    }

    return canvas.release();
}

////////////////////////////////////////////////////////////////////////////////

void SkCanvasStateUtils::ReleaseCanvasState(SkCanvasState* state) {
    SkASSERT(!state || SkCanvasState_v1::kVersion == state->version);
    // Upcast to the correct version of SkCanvasState. This avoids having a virtual destructor on
    // SkCanvasState. That would be strange since SkCanvasState has no other virtual functions, and
    // instead uses the field "version" to determine how to behave.
    delete static_cast<SkCanvasState_v1*>(state);
}
