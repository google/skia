/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "PictureRenderer.h"
#include "picture_utils.h"
#include "SamplePipeControllers.h"
#include "SkCanvas.h"
#include "SkDevice.h"
#include "SkGPipe.h"
#if SK_SUPPORT_GPU
#include "SkGpuDevice.h"
#endif
#include "SkGraphics.h"
#include "SkImageEncoder.h"
#include "SkMatrix.h"
#include "SkPicture.h"
#include "SkScalar.h"
#include "SkString.h"
#include "SkTemplates.h"
#include "SkTDArray.h"
#include "SkThreadUtils.h"
#include "SkTypes.h"

namespace sk_tools {

enum {
    kDefaultTileWidth = 256,
    kDefaultTileHeight = 256
};

void PictureRenderer::init(SkPicture* pict) {
    SkASSERT(NULL == fPicture);
    SkASSERT(NULL == fCanvas.get());
    if (fPicture != NULL || NULL != fCanvas.get()) {
        return;
    }

    SkASSERT(pict != NULL);
    if (NULL == pict) {
        return;
    }

    fPicture = pict;
    fCanvas.reset(this->setupCanvas());
}

SkCanvas* PictureRenderer::setupCanvas() {
    return this->setupCanvas(fPicture->width(), fPicture->height());
}

SkCanvas* PictureRenderer::setupCanvas(int width, int height) {
    switch(fDeviceType) {
        case kBitmap_DeviceType: {
            SkBitmap bitmap;
            sk_tools::setup_bitmap(&bitmap, width, height);
            return SkNEW_ARGS(SkCanvas, (bitmap));
            break;
        }
#if SK_SUPPORT_GPU
        case kGPU_DeviceType: {
            SkAutoTUnref<SkGpuDevice> device(SkNEW_ARGS(SkGpuDevice,
                                                    (fGrContext, SkBitmap::kARGB_8888_Config,
                                                    width, height)));
            return SkNEW_ARGS(SkCanvas, (device.get()));
            break;
        }
#endif
        default:
            SkASSERT(0);
    }

    return NULL;
}

void PictureRenderer::end() {
    this->resetState();
    fPicture = NULL;
    fCanvas.reset(NULL);
}

void PictureRenderer::resetState() {
#if SK_SUPPORT_GPU
    if (this->isUsingGpuDevice()) {
        SkGLContext* glContext = fGrContextFactory.getGLContext(
            GrContextFactory::kNative_GLContextType);

        SkASSERT(glContext != NULL);
        if (NULL == glContext) {
            return;
        }

        fGrContext->flush();
        SK_GL(*glContext, Finish());
    }
#endif
}

bool PictureRenderer::write(SkCanvas* canvas, SkString path) const {
    SkASSERT(canvas != NULL);
    SkASSERT(fPicture != NULL);
    if (NULL == canvas || NULL == fPicture) {
        return false;
    }

    SkBitmap bitmap;
    SkISize size = canvas->getDeviceSize();
    sk_tools::setup_bitmap(&bitmap, size.width(), size.height());

    canvas->readPixels(&bitmap, 0, 0);
    sk_tools::force_all_opaque(bitmap);

    // Since path is passed in by value, it is okay to modify it.
    path.append(".png");
    return SkImageEncoder::EncodeFile(path.c_str(), bitmap, SkImageEncoder::kPNG_Type, 100);
}

///////////////////////////////////////////////////////////////////////////////////////////////

bool RecordPictureRenderer::render(const SkString*) {
    SkPicture replayer;
    SkCanvas* recorder = replayer.beginRecording(fPicture->width(), fPicture->height());
    fPicture->draw(recorder);
    replayer.endRecording();
    // Since this class does not actually render, return false.
    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////

bool PipePictureRenderer::render(const SkString* path) {
    SkASSERT(fCanvas.get() != NULL);
    SkASSERT(fPicture != NULL);
    if (NULL == fCanvas.get() || NULL == fPicture) {
        return false;
    }

    PipeController pipeController(fCanvas.get());
    SkGPipeWriter writer;
    SkCanvas* pipeCanvas = writer.startRecording(&pipeController);
    pipeCanvas->drawPicture(*fPicture);
    writer.endRecording();
    fCanvas->flush();
    return path != NULL && this->write(fCanvas, *path);
}

///////////////////////////////////////////////////////////////////////////////////////////////

bool SimplePictureRenderer::render(const SkString* path) {
    SkASSERT(fCanvas.get() != NULL);
    SkASSERT(fPicture != NULL);
    if (NULL == fCanvas.get() || NULL == fPicture) {
        return false;
    }

    fCanvas->drawPicture(*fPicture);
    fCanvas->flush();
    return path != NULL && this->write(fCanvas, *path);
}

///////////////////////////////////////////////////////////////////////////////////////////////

TiledPictureRenderer::TiledPictureRenderer()
    : fUsePipe(false)
    , fTileWidth(kDefaultTileWidth)
    , fTileHeight(kDefaultTileHeight)
    , fTileWidthPercentage(0.0)
    , fTileHeightPercentage(0.0)
    , fTileMinPowerOf2Width(0)
    , fTileCounter(0)
    , fNumThreads(1)
    , fPictureClones(NULL)
    , fPipeController(NULL) { }

void TiledPictureRenderer::init(SkPicture* pict) {
    SkASSERT(pict != NULL);
    SkASSERT(0 == fTileRects.count());
    if (NULL == pict || fTileRects.count() != 0) {
        return;
    }

    // Do not call INHERITED::init(), which would create a (potentially large) canvas which is not
    // used by bench_pictures.
    fPicture = pict;

    if (fTileWidthPercentage > 0) {
        fTileWidth = sk_float_ceil2int(float(fTileWidthPercentage * fPicture->width() / 100));
    }
    if (fTileHeightPercentage > 0) {
        fTileHeight = sk_float_ceil2int(float(fTileHeightPercentage * fPicture->height() / 100));
    }

    if (fTileMinPowerOf2Width > 0) {
        this->setupPowerOf2Tiles();
    } else {
        this->setupTiles();
    }

    if (this->multiThreaded()) {
        for (int i = 0; i < fNumThreads; ++i) {
            *fCanvasPool.append() = this->setupCanvas(fTileWidth, fTileHeight);
        }
        if (!fUsePipe) {
            SkASSERT(NULL == fPictureClones);
            // Only need to create fNumThreads - 1 clones, since one thread will use the base
            // picture.
            int numberOfClones = fNumThreads - 1;
            // This will be deleted in end().
            fPictureClones = SkNEW_ARRAY(SkPicture, numberOfClones);
            fPictureClones->clone(fPictureClones, numberOfClones);
        }
    }
}

void TiledPictureRenderer::end() {
    fTileRects.reset();
    SkDELETE_ARRAY(fPictureClones);
    fPictureClones = NULL;
    fCanvasPool.unrefAll();
    if (fPipeController != NULL) {
        SkASSERT(fUsePipe);
        SkDELETE(fPipeController);
        fPipeController = NULL;
    }
    this->INHERITED::end();
}

TiledPictureRenderer::~TiledPictureRenderer() {
    // end() must be called to delete fPictureClones and fPipeController
    SkASSERT(NULL == fPictureClones);
    SkASSERT(NULL == fPipeController);
}

void TiledPictureRenderer::setupTiles() {
    for (int tile_y_start = 0; tile_y_start < fPicture->height(); tile_y_start += fTileHeight) {
        for (int tile_x_start = 0; tile_x_start < fPicture->width(); tile_x_start += fTileWidth) {
            *fTileRects.append() = SkRect::MakeXYWH(SkIntToScalar(tile_x_start),
                                                    SkIntToScalar(tile_y_start),
                                                    SkIntToScalar(fTileWidth),
                                                    SkIntToScalar(fTileHeight));
        }
    }
}

// The goal of the powers of two tiles is to minimize the amount of wasted tile
// space in the width-wise direction and then minimize the number of tiles. The
// constraints are that every tile must have a pixel width that is a power of
// two and also be of some minimal width (that is also a power of two).
//
// This is solved by first taking our picture size and rounding it up to the
// multiple of the minimal width. The binary representation of this rounded
// value gives us the tiles we need: a bit of value one means we need a tile of
// that size.
void TiledPictureRenderer::setupPowerOf2Tiles() {
    int rounded_value = fPicture->width();
    if (fPicture->width() % fTileMinPowerOf2Width != 0) {
        rounded_value = fPicture->width() - (fPicture->width() % fTileMinPowerOf2Width)
            + fTileMinPowerOf2Width;
    }

    int num_bits = SkScalarCeilToInt(SkScalarLog2(SkIntToScalar(fPicture->width())));
    int largest_possible_tile_size = 1 << num_bits;

    // The tile height is constant for a particular picture.
    for (int tile_y_start = 0; tile_y_start < fPicture->height(); tile_y_start += fTileHeight) {
        int tile_x_start = 0;
        int current_width = largest_possible_tile_size;
        // Set fTileWidth to be the width of the widest tile, so that each canvas is large enough
        // to draw each tile.
        fTileWidth = current_width;

        while (current_width >= fTileMinPowerOf2Width) {
            // It is very important this is a bitwise AND.
            if (current_width & rounded_value) {
                *fTileRects.append() = SkRect::MakeXYWH(SkIntToScalar(tile_x_start),
                                                        SkIntToScalar(tile_y_start),
                                                        SkIntToScalar(current_width),
                                                        SkIntToScalar(fTileHeight));
                tile_x_start += current_width;
            }

            current_width >>= 1;
        }
    }
}

/**
 * Draw the specified playback to the canvas translated to rectangle provided, so that this mini
 * canvas represents the rectangle's portion of the overall picture.
 * Saves and restores so that the initial clip and matrix return to their state before this function
 * is called.
 */
template<class T>
static void DrawTileToCanvas(SkCanvas* canvas, const SkRect& tileRect, T* playback) {
    int saveCount = canvas->save();
    // Translate so that we draw the correct portion of the picture
    canvas->translate(-tileRect.fLeft, -tileRect.fTop);
    playback->draw(canvas);
    canvas->restoreToCount(saveCount);
    canvas->flush();
}

///////////////////////////////////////////////////////////////////////////////////////////////
// Base class for data used both by pipe and clone picture multi threaded drawing.

struct ThreadData {
    ThreadData(SkCanvas* target, int* tileCounter, SkTDArray<SkRect>* tileRects)
    : fCanvas(target)
    , fTileCounter(tileCounter)
    , fTileRects(tileRects) {
        SkASSERT(target != NULL && tileCounter != NULL && tileRects != NULL);
    }

    const SkRect* nextTile() {
        if (int32_t i = sk_atomic_inc(fTileCounter) < fTileRects->count()) {
            return &fTileRects->operator[](i);
        }
        return NULL;
    }

    // All of these are pointers to objects owned elsewhere
    SkCanvas*                fCanvas;
private:
    // Shared by all threads, this states which is the next tile to be drawn.
    int32_t*                 fTileCounter;
    // Points to the array of rectangles. The array is already created before any threads are
    // started and then it is unmodified, so there is no danger of race conditions.
    const SkTDArray<SkRect>* fTileRects;
};

///////////////////////////////////////////////////////////////////////////////////////////////
// Draw using Pipe

struct TileData : public ThreadData {
    TileData(ThreadSafePipeController* controller, SkCanvas* canvas, int* tileCounter,
             SkTDArray<SkRect>* tileRects)
    : INHERITED(canvas, tileCounter, tileRects)
    , fController(controller) {}

    ThreadSafePipeController* fController;

    typedef ThreadData INHERITED;
};

static void DrawTile(void* data) {
    SkGraphics::SetTLSFontCacheLimit(1 * 1024 * 1024);
    TileData* tileData = static_cast<TileData*>(data);

    const SkRect* tileRect;
    while ((tileRect = tileData->nextTile()) != NULL) {
        DrawTileToCanvas(tileData->fCanvas, *tileRect, tileData->fController);
    }
    SkDELETE(tileData);
}

///////////////////////////////////////////////////////////////////////////////////////////////
// Draw using Picture

struct CloneData : public ThreadData {
    CloneData(SkPicture* clone, SkCanvas* target, int* tileCounter, SkTDArray<SkRect>* tileRects)
    : INHERITED(target, tileCounter, tileRects)
    , fClone(clone) {}

    SkPicture* fClone;

    typedef ThreadData INHERITED;
};

static void DrawClonedTiles(void* data) {
    SkGraphics::SetTLSFontCacheLimit(1 * 1024 * 1024);
    CloneData* cloneData = static_cast<CloneData*>(data);

    const SkRect* tileRect;
    while ((tileRect = cloneData->nextTile()) != NULL) {
        DrawTileToCanvas(cloneData->fCanvas, *tileRect, cloneData->fClone);
    }
    SkDELETE(cloneData);
}

///////////////////////////////////////////////////////////////////////////////////////////////

void TiledPictureRenderer::setup() {
    if (this->multiThreaded()) {
        // Reset to zero so we start with the first tile.
        fTileCounter = 0;
        if (fUsePipe) {
            // Record the picture into the pipe controller. It is done here because unlike
            // SkPicture, the pipe is modified (bitmaps can be removed) by drawing.
            // fPipeController is deleted here after each call to render() except the last one and
            // in end() for the last one.
            if (fPipeController != NULL) {
                SkDELETE(fPipeController);
            }
            fPipeController = SkNEW_ARGS(ThreadSafePipeController, (fTileRects.count()));
            SkGPipeWriter writer;
            SkCanvas* pipeCanvas = writer.startRecording(fPipeController,
                                                         SkGPipeWriter::kSimultaneousReaders_Flag);
            SkASSERT(fPicture != NULL);
            fPicture->draw(pipeCanvas);
            writer.endRecording();
        }
    }
}

bool TiledPictureRenderer::render(const SkString* path) {
    SkASSERT(fPicture != NULL);
    if (NULL == fPicture) {
        return false;
    }

    if (this->multiThreaded()) {
        SkASSERT(fCanvasPool.count() == fNumThreads);
        SkTDArray<SkThread*> threads;
        SkThread::entryPointProc proc = fUsePipe ? DrawTile : DrawClonedTiles;
        for (int i = 0; i < fNumThreads; ++i) {
            // data will be deleted by the entryPointProc.
            ThreadData* data;
            if (fUsePipe) {
                data = SkNEW_ARGS(TileData,
                                  (fPipeController, fCanvasPool[i], &fTileCounter, &fTileRects));
            } else {
                SkPicture* pic = (0 == i) ? fPicture : &fPictureClones[i-1];
                data = SkNEW_ARGS(CloneData, (pic, fCanvasPool[i], &fTileCounter, &fTileRects));
            }
            SkThread* thread = SkNEW_ARGS(SkThread, (proc, data));
            if (!thread->start()) {
                SkDebugf("Could not start %s thread %i.\n", (fUsePipe ? "pipe" : "picture"), i);
            }
            *threads.append() = thread;
        }
        SkASSERT(threads.count() == fNumThreads);
        for (int i = 0; i < fNumThreads; ++i) {
            SkThread* thread = threads[i];
            thread->join();
            SkDELETE(thread);
        }
        threads.reset();
        // Currently multithreaded is not an option for render_pictures
        return false;
    } else {
        // For single thread, we really only need one canvas total.
        SkCanvas* canvas = this->setupCanvas(fTileWidth, fTileHeight);
        SkAutoUnref aur(canvas);

        for (int i = 0; i < fTileRects.count(); ++i) {
            DrawTileToCanvas(canvas, fTileRects[i], fPicture);
            if (path != NULL) {
                SkString tilePath(*path);
                tilePath.appendf("%i", i);
                if (!this->write(canvas, tilePath)) {
                    return false;
                }
            }
        }
        return path != NULL;
    }
}

SkCanvas* TiledPictureRenderer::setupCanvas(int width, int height) {
    SkCanvas* canvas = this->INHERITED::setupCanvas(width, height);
    SkASSERT(fPicture != NULL);
    // Clip the tile to an area that is completely in what the SkPicture says is the
    // drawn-to area. This is mostly important for tiles on the right and bottom edges
    // as they may go over this area and the picture may have some commands that
    // draw outside of this area and so should not actually be written.
    SkRect clip = SkRect::MakeWH(SkIntToScalar(fPicture->width()),
                                 SkIntToScalar(fPicture->height()));
    canvas->clipRect(clip);
    return canvas;
}

///////////////////////////////////////////////////////////////////////////////////////////////

void PlaybackCreationRenderer::setup() {
    SkCanvas* recorder = fReplayer.beginRecording(fPicture->width(), fPicture->height());
    fPicture->draw(recorder);
}

bool PlaybackCreationRenderer::render(const SkString*) {
    fReplayer.endRecording();
    // Since this class does not actually render, return false.
    return false;
}

}
