/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "PictureRenderer.h"
#include "picture_utils.h"
#include "SamplePipeControllers.h"
#include "SkBitmapHasher.h"
#include "SkCanvas.h"
#include "SkData.h"
#include "SkDevice.h"
#include "SkDiscardableMemoryPool.h"
#include "SkGPipe.h"
#if SK_SUPPORT_GPU
#include "gl/GrGLDefines.h"
#include "SkGpuDevice.h"
#endif
#include "SkGraphics.h"
#include "SkImageEncoder.h"
#include "SkMaskFilter.h"
#include "SkMatrix.h"
#include "SkMultiPictureDraw.h"
#include "SkOSFile.h"
#include "SkPaintFilterCanvas.h"
#include "SkPicture.h"
#include "SkPictureRecorder.h"
#include "SkPictureUtils.h"
#include "SkPixelRef.h"
#include "SkPixelSerializer.h"
#include "SkScalar.h"
#include "SkStream.h"
#include "SkString.h"
#include "SkSurface.h"
#include "SkTemplates.h"
#include "SkTDArray.h"
#include "SkThreadUtils.h"
#include "SkTypes.h"
#include "sk_tool_utils.h"

static inline SkScalar scalar_log2(SkScalar x) {
    static const SkScalar log2_conversion_factor = SkScalarInvert(SkScalarLog(2));

    return SkScalarLog(x) * log2_conversion_factor;
}

namespace sk_tools {

enum {
    kDefaultTileWidth = 256,
    kDefaultTileHeight = 256
};

void PictureRenderer::init(const SkPicture* pict,
                           const SkString* writePath,
                           const SkString* mismatchPath,
                           const SkString* inputFilename,
                           bool useChecksumBasedFilenames,
                           bool useMultiPictureDraw) {
    this->CopyString(&fWritePath, writePath);
    this->CopyString(&fMismatchPath, mismatchPath);
    this->CopyString(&fInputFilename, inputFilename);
    fUseChecksumBasedFilenames = useChecksumBasedFilenames;
    fUseMultiPictureDraw = useMultiPictureDraw;

    SkASSERT(NULL == fPicture);
    SkASSERT(NULL == fCanvas.get());
    if (fPicture || fCanvas.get()) {
        return;
    }

    SkASSERT(pict != NULL);
    if (NULL == pict) {
        return;
    }

    fPicture.reset(pict)->ref();
    fCanvas.reset(this->setupCanvas());
}

void PictureRenderer::CopyString(SkString* dest, const SkString* src) {
    if (src) {
        dest->set(*src);
    } else {
        dest->reset();
    }
}

class FlagsFilterCanvas : public SkPaintFilterCanvas {
public:
    FlagsFilterCanvas(SkCanvas* canvas, PictureRenderer::DrawFilterFlags* flags)
        : INHERITED(canvas->imageInfo().width(), canvas->imageInfo().height())
        , fFlags(flags) {
        this->addCanvas(canvas);
    }

protected:
    void onFilterPaint(SkPaint* paint, Type t) const override {
        paint->setFlags(paint->getFlags() & ~fFlags[t] & SkPaint::kAllFlags);
        if (PictureRenderer::kMaskFilter_DrawFilterFlag & fFlags[t]) {
            SkMaskFilter* maskFilter = paint->getMaskFilter();
            if (maskFilter) {
                paint->setMaskFilter(NULL);
            }
        }
        if (PictureRenderer::kHinting_DrawFilterFlag & fFlags[t]) {
            paint->setHinting(SkPaint::kNo_Hinting);
        } else if (PictureRenderer::kSlightHinting_DrawFilterFlag & fFlags[t]) {
            paint->setHinting(SkPaint::kSlight_Hinting);
        }
    }

private:
    const PictureRenderer::DrawFilterFlags* fFlags;

    typedef SkPaintFilterCanvas INHERITED;
};

SkCanvas* PictureRenderer::setupCanvas() {
    const int width = this->getViewWidth();
    const int height = this->getViewHeight();
    return this->setupCanvas(width, height);
}

SkCanvas* PictureRenderer::setupCanvas(int width, int height) {
    SkAutoTUnref<SkCanvas> canvas;

    switch(fDeviceType) {
        case kBitmap_DeviceType: {
            SkBitmap bitmap;
            sk_tools::setup_bitmap(&bitmap, width, height);
            canvas.reset(SkNEW_ARGS(SkCanvas, (bitmap)));
        }
        break;
#if SK_SUPPORT_GPU
#if SK_ANGLE
        case kAngle_DeviceType:
            // fall through
#endif
#if SK_MESA
        case kMesa_DeviceType:
            // fall through
#endif
        case kGPU_DeviceType:
        case kNVPR_DeviceType: {
            SkAutoTUnref<GrSurface> target;
            if (fGrContext) {
                // create a render target to back the device
                GrSurfaceDesc desc;
                desc.fConfig = kSkia8888_GrPixelConfig;
                desc.fFlags = kRenderTarget_GrSurfaceFlag;
                desc.fWidth = width;
                desc.fHeight = height;
                desc.fSampleCnt = fSampleCount;
                target.reset(fGrContext->textureProvider()->createTexture(desc, false, NULL, 0));
            }

            uint32_t flags = fUseDFText ? SkSurfaceProps::kUseDistanceFieldFonts_Flag : 0;
            SkSurfaceProps props(flags, SkSurfaceProps::kLegacyFontHost_InitType);
            SkAutoTUnref<SkGpuDevice> device(
                SkGpuDevice::Create(target->asRenderTarget(), &props,
                                    SkGpuDevice::kUninit_InitContents));
            if (!device) {
                return NULL;
            }
            canvas.reset(SkNEW_ARGS(SkCanvas, (device)));
            break;
        }
#endif
        default:
            SkASSERT(0);
            return NULL;
    }

    if (fHasDrawFilters) {
        if (fDrawFilters[0] & PictureRenderer::kAAClip_DrawFilterFlag) {
            canvas->setAllowSoftClip(false);
        }

        canvas.reset(SkNEW_ARGS(FlagsFilterCanvas, (canvas.get(), fDrawFilters)));
    }

    this->scaleToScaleFactor(canvas);

    // Pictures often lie about their extent (i.e., claim to be 100x100 but
    // only ever draw to 90x100). Clear here so the undrawn portion will have
    // a consistent color
    canvas->clear(SK_ColorTRANSPARENT);
    return canvas.detach();
}

void PictureRenderer::scaleToScaleFactor(SkCanvas* canvas) {
    SkASSERT(canvas != NULL);
    if (fScaleFactor != SK_Scalar1) {
        canvas->scale(fScaleFactor, fScaleFactor);
    }
}

void PictureRenderer::end() {
    this->resetState(true);
    fPicture.reset(NULL);
    fCanvas.reset(NULL);
}

int PictureRenderer::getViewWidth() {
    SkASSERT(fPicture != NULL);
    int width = SkScalarCeilToInt(fPicture->cullRect().width() * fScaleFactor);
    if (fViewport.width() > 0) {
        width = SkMin32(width, fViewport.width());
    }
    return width;
}

int PictureRenderer::getViewHeight() {
    SkASSERT(fPicture != NULL);
    int height = SkScalarCeilToInt(fPicture->cullRect().height() * fScaleFactor);
    if (fViewport.height() > 0) {
        height = SkMin32(height, fViewport.height());
    }
    return height;
}

/** Converts fPicture to a picture that uses a BBoxHierarchy.
 *  PictureRenderer subclasses that are used to test picture playback
 *  should call this method during init.
 */
void PictureRenderer::buildBBoxHierarchy() {
    SkASSERT(fPicture);
    if (kNone_BBoxHierarchyType != fBBoxHierarchyType && fPicture) {
        SkAutoTDelete<SkBBHFactory> factory(this->getFactory());
        SkPictureRecorder recorder;
        uint32_t flags = this->recordFlags();
        if (fUseMultiPictureDraw) {
            flags |= SkPictureRecorder::kComputeSaveLayerInfo_RecordFlag;
        }
        SkCanvas* canvas = recorder.beginRecording(fPicture->cullRect().width(),
                                                   fPicture->cullRect().height(),
                                                   factory.get(),
                                                   flags);
        fPicture->playback(canvas);
        fPicture.reset(recorder.endRecording());
    }
}

void PictureRenderer::resetState(bool callFinish) {
#if SK_SUPPORT_GPU
    SkGLContext* glContext = this->getGLContext();
    if (NULL == glContext) {
        SkASSERT(kBitmap_DeviceType == fDeviceType);
        return;
    }

    fGrContext->flush();
    glContext->swapBuffers();
    if (callFinish) {
        SK_GL(*glContext, Finish());
    }
#endif
}

void PictureRenderer::purgeTextures() {
    SkDiscardableMemoryPool* pool = SkGetGlobalDiscardableMemoryPool();

    pool->dumpPool();

#if SK_SUPPORT_GPU
    SkGLContext* glContext = this->getGLContext();
    if (NULL == glContext) {
        SkASSERT(kBitmap_DeviceType == fDeviceType);
        return;
    }

    // resetState should've already done this
    fGrContext->flush();

    fGrContext->purgeAllUnlockedResources();
#endif
}

/**
 * Write the canvas to an image file and/or JSON summary.
 *
 * @param canvas Must be non-null. Canvas to be written to a file.
 * @param writePath If nonempty, write the binary image to a file within this directory.
 * @param mismatchPath If nonempty, write the binary image to a file within this directory,
 *     but only if the image does not match expectations.
 * @param inputFilename If we are writing out a binary image, use this to build its filename.
 * @param jsonSummaryPtr If not null, add image results (checksum) to this summary.
 * @param useChecksumBasedFilenames If true, use checksum-based filenames when writing to disk.
 * @param tileNumberPtr If not null, which tile number this image contains.
 *
 * @return bool True if the operation completed successfully.
 */
static bool write(SkCanvas* canvas, const SkString& writePath, const SkString& mismatchPath,
                  const SkString& inputFilename, ImageResultsAndExpectations *jsonSummaryPtr,
                  bool useChecksumBasedFilenames, const int* tileNumberPtr=NULL) {
    SkASSERT(canvas != NULL);
    if (NULL == canvas) {
        return false;
    }

    SkBitmap bitmap;
    SkISize size = canvas->getDeviceSize();
    setup_bitmap(&bitmap, size.width(), size.height());

    canvas->readPixels(&bitmap, 0, 0);
    force_all_opaque(bitmap);
    BitmapAndDigest bitmapAndDigest(bitmap);

    SkString escapedInputFilename(inputFilename);
    replace_char(&escapedInputFilename, '.', '_');

    // TODO(epoger): what about including the config type within outputFilename?  That way,
    // we could combine results of different config types without conflicting filenames.
    SkString outputFilename;
    const char *outputSubdirPtr = NULL;
    if (useChecksumBasedFilenames) {
        ImageDigest *imageDigestPtr = bitmapAndDigest.getImageDigestPtr();
        outputSubdirPtr = escapedInputFilename.c_str();
        outputFilename.set(imageDigestPtr->getHashType());
        outputFilename.append("_");
        outputFilename.appendU64(imageDigestPtr->getHashValue());
    } else {
        outputFilename.set(escapedInputFilename);
        if (tileNumberPtr) {
            outputFilename.append("-tile");
            outputFilename.appendS32(*tileNumberPtr);
        }
    }
    outputFilename.append(".png");

    if (jsonSummaryPtr) {
        ImageDigest *imageDigestPtr = bitmapAndDigest.getImageDigestPtr();
        SkString outputRelativePath;
        if (outputSubdirPtr) {
            outputRelativePath.set(outputSubdirPtr);
            outputRelativePath.append("/");  // always use "/", even on Windows
            outputRelativePath.append(outputFilename);
        } else {
            outputRelativePath.set(outputFilename);
        }

        jsonSummaryPtr->add(inputFilename.c_str(), outputRelativePath.c_str(),
                            *imageDigestPtr, tileNumberPtr);
        if (!mismatchPath.isEmpty() &&
            !jsonSummaryPtr->getExpectation(inputFilename.c_str(),
                                            tileNumberPtr).matches(*imageDigestPtr)) {
            if (!write_bitmap_to_disk(bitmap, mismatchPath, outputSubdirPtr, outputFilename)) {
                return false;
            }
        }
    }

    if (writePath.isEmpty()) {
        return true;
    } else {
        return write_bitmap_to_disk(bitmap, writePath, outputSubdirPtr, outputFilename);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////

SkCanvas* RecordPictureRenderer::setupCanvas(int width, int height) {
    // defer the canvas setup until the render step
    return NULL;
}

bool RecordPictureRenderer::render(SkBitmap** out) {
    SkAutoTDelete<SkBBHFactory> factory(this->getFactory());
    SkPictureRecorder recorder;
    SkCanvas* canvas = recorder.beginRecording(SkIntToScalar(this->getViewWidth()),
                                               SkIntToScalar(this->getViewHeight()),
                                               factory.get(),
                                               this->recordFlags());
    this->scaleToScaleFactor(canvas);
    fPicture->playback(canvas);
    SkAutoTUnref<SkPicture> picture(recorder.endRecording());
    if (!fWritePath.isEmpty()) {
        // Record the new picture as a new SKP with PNG encoded bitmaps.
        SkString skpPath = SkOSPath::Join(fWritePath.c_str(), fInputFilename.c_str());
        SkFILEWStream stream(skpPath.c_str());
        sk_tool_utils::PngPixelSerializer serializer;
        picture->serialize(&stream, &serializer);
        return true;
    }
    return false;
}

SkString RecordPictureRenderer::getConfigNameInternal() {
    return SkString("record");
}

///////////////////////////////////////////////////////////////////////////////////////////////

bool PipePictureRenderer::render(SkBitmap** out) {
    SkASSERT(fCanvas.get() != NULL);
    SkASSERT(fPicture != NULL);
    if (NULL == fCanvas.get() || NULL == fPicture) {
        return false;
    }

    PipeController pipeController(fCanvas.get());
    SkGPipeWriter writer;
    SkCanvas* pipeCanvas = writer.startRecording(&pipeController);
    pipeCanvas->drawPicture(fPicture);
    writer.endRecording();
    fCanvas->flush();
    if (out) {
        *out = SkNEW(SkBitmap);
        setup_bitmap(*out, SkScalarCeilToInt(fPicture->cullRect().width()),
                           SkScalarCeilToInt(fPicture->cullRect().height()));
        fCanvas->readPixels(*out, 0, 0);
    }
    if (fEnableWrites) {
        return write(fCanvas, fWritePath, fMismatchPath, fInputFilename, fJsonSummaryPtr,
                     fUseChecksumBasedFilenames);
    } else {
        return true;
    }
}

SkString PipePictureRenderer::getConfigNameInternal() {
    return SkString("pipe");
}

///////////////////////////////////////////////////////////////////////////////////////////////

void SimplePictureRenderer::init(const SkPicture* picture, const SkString* writePath,
                                 const SkString* mismatchPath, const SkString* inputFilename,
                                 bool useChecksumBasedFilenames, bool useMultiPictureDraw) {
    INHERITED::init(picture, writePath, mismatchPath, inputFilename,
                    useChecksumBasedFilenames, useMultiPictureDraw);
    this->buildBBoxHierarchy();
}

bool SimplePictureRenderer::render(SkBitmap** out) {
    SkASSERT(fCanvas.get() != NULL);
    SkASSERT(fPicture);
    if (NULL == fCanvas.get() || NULL == fPicture) {
        return false;
    }

    if (fUseMultiPictureDraw) {
        SkMultiPictureDraw mpd;

        mpd.add(fCanvas, fPicture);

        mpd.draw();
    } else {
        fCanvas->drawPicture(fPicture);
    }
    fCanvas->flush();
    if (out) {
        *out = SkNEW(SkBitmap);
        setup_bitmap(*out, SkScalarCeilToInt(fPicture->cullRect().width()),
                           SkScalarCeilToInt(fPicture->cullRect().height()));
        fCanvas->readPixels(*out, 0, 0);
    }
    if (fEnableWrites) {
        return write(fCanvas, fWritePath, fMismatchPath, fInputFilename, fJsonSummaryPtr,
                     fUseChecksumBasedFilenames);
    } else {
        return true;
    }
}

SkString SimplePictureRenderer::getConfigNameInternal() {
    return SkString("simple");
}

///////////////////////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU
TiledPictureRenderer::TiledPictureRenderer(const GrContextOptions& opts)
    : INHERITED(opts)
    , fTileWidth(kDefaultTileWidth)
#else
TiledPictureRenderer::TiledPictureRenderer()
    : fTileWidth(kDefaultTileWidth)
#endif
    , fTileHeight(kDefaultTileHeight)
    , fTileWidthPercentage(0.0)
    , fTileHeightPercentage(0.0)
    , fTileMinPowerOf2Width(0)
    , fCurrentTileOffset(-1)
    , fTilesX(0)
    , fTilesY(0) { }

void TiledPictureRenderer::init(const SkPicture* pict, const SkString* writePath,
                                const SkString* mismatchPath, const SkString* inputFilename,
                                bool useChecksumBasedFilenames, bool useMultiPictureDraw) {
    SkASSERT(pict);
    SkASSERT(0 == fTileRects.count());
    if (NULL == pict || fTileRects.count() != 0) {
        return;
    }

    // Do not call INHERITED::init(), which would create a (potentially large) canvas which is not
    // used by bench_pictures.
    fPicture.reset(pict)->ref();
    this->CopyString(&fWritePath, writePath);
    this->CopyString(&fMismatchPath, mismatchPath);
    this->CopyString(&fInputFilename, inputFilename);
    fUseChecksumBasedFilenames = useChecksumBasedFilenames;
    fUseMultiPictureDraw = useMultiPictureDraw;
    this->buildBBoxHierarchy();

    if (fTileWidthPercentage > 0) {
        fTileWidth = SkScalarCeilToInt(float(fTileWidthPercentage * fPicture->cullRect().width() / 100));
    }
    if (fTileHeightPercentage > 0) {
        fTileHeight = SkScalarCeilToInt(float(fTileHeightPercentage * fPicture->cullRect().height() / 100));
    }

    if (fTileMinPowerOf2Width > 0) {
        this->setupPowerOf2Tiles();
    } else {
        this->setupTiles();
    }
    fCanvas.reset(this->setupCanvas(fTileWidth, fTileHeight));
    // Initialize to -1 so that the first call to nextTile will set this up to draw tile 0 on the
    // first call to drawCurrentTile.
    fCurrentTileOffset = -1;
}

void TiledPictureRenderer::end() {
    fTileRects.reset();
    this->INHERITED::end();
}

void TiledPictureRenderer::setupTiles() {
    // Only use enough tiles to cover the viewport
    const int width = this->getViewWidth();
    const int height = this->getViewHeight();

    fTilesX = fTilesY = 0;
    for (int tile_y_start = 0; tile_y_start < height; tile_y_start += fTileHeight) {
        fTilesY++;
        for (int tile_x_start = 0; tile_x_start < width; tile_x_start += fTileWidth) {
            if (0 == tile_y_start) {
                // Only count tiles in the X direction on the first pass.
                fTilesX++;
            }
            *fTileRects.append() = SkIRect::MakeXYWH(tile_x_start, tile_y_start,
                                                     fTileWidth, fTileHeight);
        }
    }
}

bool TiledPictureRenderer::tileDimensions(int &x, int &y) {
    if (fTileRects.count() == 0 || NULL == fPicture) {
        return false;
    }
    x = fTilesX;
    y = fTilesY;
    return true;
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
    // Only use enough tiles to cover the viewport
    const int width = this->getViewWidth();
    const int height = this->getViewHeight();

    int rounded_value = width;
    if (width % fTileMinPowerOf2Width != 0) {
        rounded_value = width - (width % fTileMinPowerOf2Width) + fTileMinPowerOf2Width;
    }

    int num_bits = SkScalarCeilToInt(scalar_log2(SkIntToScalar(width)));
    int largest_possible_tile_size = 1 << num_bits;

    fTilesX = fTilesY = 0;
    // The tile height is constant for a particular picture.
    for (int tile_y_start = 0; tile_y_start < height; tile_y_start += fTileHeight) {
        fTilesY++;
        int tile_x_start = 0;
        int current_width = largest_possible_tile_size;
        // Set fTileWidth to be the width of the widest tile, so that each canvas is large enough
        // to draw each tile.
        fTileWidth = current_width;

        while (current_width >= fTileMinPowerOf2Width) {
            // It is very important this is a bitwise AND.
            if (current_width & rounded_value) {
                if (0 == tile_y_start) {
                    // Only count tiles in the X direction on the first pass.
                    fTilesX++;
                }
                *fTileRects.append() = SkIRect::MakeXYWH(tile_x_start, tile_y_start,
                                                         current_width, fTileHeight);
                tile_x_start += current_width;
            }

            current_width >>= 1;
        }
    }
}

/**
 * Draw the specified picture to the canvas translated to rectangle provided, so that this mini
 * canvas represents the rectangle's portion of the overall picture.
 * Saves and restores so that the initial clip and matrix return to their state before this function
 * is called.
 */
static void draw_tile_to_canvas(SkCanvas* canvas,
                                const SkIRect& tileRect,
                                const SkPicture* picture) {
    int saveCount = canvas->save();
    // Translate so that we draw the correct portion of the picture.
    // Perform a postTranslate so that the scaleFactor does not interfere with the positioning.
    SkMatrix mat(canvas->getTotalMatrix());
    mat.postTranslate(-SkIntToScalar(tileRect.fLeft), -SkIntToScalar(tileRect.fTop));
    canvas->setMatrix(mat);
    canvas->clipRect(SkRect::Make(tileRect));
    canvas->clear(SK_ColorTRANSPARENT); // Not every picture covers the entirety of every tile
    canvas->drawPicture(picture);
    canvas->restoreToCount(saveCount);
    canvas->flush();
}

///////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Copies the entirety of the src bitmap (typically a tile) into a portion of the dst bitmap.
 * If the src bitmap is too large to fit within the dst bitmap after the x and y
 * offsets have been applied, any excess will be ignored (so only the top-left portion of the
 * src bitmap will be copied).
 *
 * @param src source bitmap
 * @param dst destination bitmap
 * @param xOffset x-offset within destination bitmap
 * @param yOffset y-offset within destination bitmap
 */
static void bitmapCopyAtOffset(const SkBitmap& src, SkBitmap* dst,
                               int xOffset, int yOffset) {
    for (int y = 0; y <src.height() && y + yOffset < dst->height() ; y++) {
        for (int x = 0; x < src.width() && x + xOffset < dst->width() ; x++) {
            *dst->getAddr32(xOffset + x, yOffset + y) = *src.getAddr32(x, y);
        }
    }
}

bool TiledPictureRenderer::nextTile(int &i, int &j) {
    if (++fCurrentTileOffset < fTileRects.count()) {
        i = fCurrentTileOffset % fTilesX;
        j = fCurrentTileOffset / fTilesX;
        return true;
    }
    return false;
}

void TiledPictureRenderer::drawCurrentTile() {
    SkASSERT(fCurrentTileOffset >= 0 && fCurrentTileOffset < fTileRects.count());
    draw_tile_to_canvas(fCanvas, fTileRects[fCurrentTileOffset], fPicture);
}

bool TiledPictureRenderer::postRender(SkCanvas* canvas, const SkIRect& tileRect,
                                      SkBitmap* tempBM, SkBitmap** out,
                                      int tileNumber) {
    bool success = true;

    if (fEnableWrites) {
        success &= write(canvas, fWritePath, fMismatchPath, fInputFilename, fJsonSummaryPtr,
                         fUseChecksumBasedFilenames, &tileNumber);
    }
    if (out) {
        if (canvas->readPixels(tempBM, 0, 0)) {
            // Add this tile to the entire bitmap.
            bitmapCopyAtOffset(*tempBM, *out, tileRect.left(), tileRect.top());
        } else {
            success = false;
        }
    }

    return success;
}

bool TiledPictureRenderer::render(SkBitmap** out) {
    SkASSERT(fPicture != NULL);
    if (NULL == fPicture) {
        return false;
    }

    SkBitmap bitmap;
    if (out) {
        *out = SkNEW(SkBitmap);
        setup_bitmap(*out, SkScalarCeilToInt(fPicture->cullRect().width()),
                           SkScalarCeilToInt(fPicture->cullRect().height()));
        setup_bitmap(&bitmap, fTileWidth, fTileHeight);
    }
    bool success = true;

    if (fUseMultiPictureDraw) {
        SkMultiPictureDraw mpd;
        SkTDArray<SkSurface*> surfaces;
        surfaces.setReserve(fTileRects.count());

        // Create a separate SkSurface/SkCanvas for each tile along with a
        // translated version of the skp (to mimic Chrome's behavior) and
        // feed all such pairs to the MultiPictureDraw.
        for (int i = 0; i < fTileRects.count(); ++i) {
            SkImageInfo ii = fCanvas->imageInfo().makeWH(fTileRects[i].width(),
                                                         fTileRects[i].height());
            *surfaces.append() = fCanvas->newSurface(ii);
            surfaces[i]->getCanvas()->setMatrix(fCanvas->getTotalMatrix());

            SkPictureRecorder recorder;
            SkRTreeFactory bbhFactory;

            SkCanvas* c = recorder.beginRecording(SkIntToScalar(fTileRects[i].width()),
                                                  SkIntToScalar(fTileRects[i].height()),
                                                  &bbhFactory,
                                                  SkPictureRecorder::kComputeSaveLayerInfo_RecordFlag);
            c->save();
            SkMatrix mat;
            mat.setTranslate(-SkIntToScalar(fTileRects[i].fLeft),
                             -SkIntToScalar(fTileRects[i].fTop));
            c->setMatrix(mat);
            c->drawPicture(fPicture);
            c->restore();

            SkAutoTUnref<SkPicture> xlatedPicture(recorder.endRecording());

            mpd.add(surfaces[i]->getCanvas(), xlatedPicture);
        }

        // Render all the buffered SkCanvases/SkPictures
        mpd.draw();

        // Sort out the results and cleanup the allocated surfaces
        for (int i = 0; i < fTileRects.count(); ++i) {
            success &= this->postRender(surfaces[i]->getCanvas(), fTileRects[i], &bitmap, out, i);
            surfaces[i]->unref();
        }
    } else {
        for (int i = 0; i < fTileRects.count(); ++i) {
            draw_tile_to_canvas(fCanvas, fTileRects[i], fPicture);
            success &= this->postRender(fCanvas, fTileRects[i], &bitmap, out, i);
        }
    }

    return success;
}

SkCanvas* TiledPictureRenderer::setupCanvas(int width, int height) {
    SkCanvas* canvas = this->INHERITED::setupCanvas(width, height);
    SkASSERT(fPicture);
    // Clip the tile to an area that is completely inside both the SkPicture and the viewport. This
    // is mostly important for tiles on the right and bottom edges as they may go over this area and
    // the picture may have some commands that draw outside of this area and so should not actually
    // be written.
    // Uses a clipRegion so that it will be unaffected by the scale factor, which may have been set
    // by INHERITED::setupCanvas.
    SkRegion clipRegion;
    clipRegion.setRect(0, 0, this->getViewWidth(), this->getViewHeight());
    canvas->clipRegion(clipRegion);
    return canvas;
}

SkString TiledPictureRenderer::getConfigNameInternal() {
    SkString name;
    if (fTileMinPowerOf2Width > 0) {
        name.append("pow2tile_");
        name.appendf("%i", fTileMinPowerOf2Width);
    } else {
        name.append("tile_");
        if (fTileWidthPercentage > 0) {
            name.appendf("%.f%%", fTileWidthPercentage);
        } else {
            name.appendf("%i", fTileWidth);
        }
    }
    name.append("x");
    if (fTileHeightPercentage > 0) {
        name.appendf("%.f%%", fTileHeightPercentage);
    } else {
        name.appendf("%i", fTileHeight);
    }
    return name;
}

///////////////////////////////////////////////////////////////////////////////////////////////

void PlaybackCreationRenderer::setup() {
    SkAutoTDelete<SkBBHFactory> factory(this->getFactory());
    fRecorder.reset(SkNEW(SkPictureRecorder));
    SkCanvas* canvas = fRecorder->beginRecording(SkIntToScalar(this->getViewWidth()),
                                                 SkIntToScalar(this->getViewHeight()),
                                                 factory.get(),
                                                 this->recordFlags());
    this->scaleToScaleFactor(canvas);
    canvas->drawPicture(fPicture);
}

bool PlaybackCreationRenderer::render(SkBitmap** out) {
    fPicture.reset(fRecorder->endRecording());
    // Since this class does not actually render, return false.
    return false;
}

SkString PlaybackCreationRenderer::getConfigNameInternal() {
    return SkString("playback_creation");
}

///////////////////////////////////////////////////////////////////////////////////////////////
// SkPicture variants for each BBoxHierarchy type

SkBBHFactory* PictureRenderer::getFactory() {
    switch (fBBoxHierarchyType) {
        case kNone_BBoxHierarchyType:
            return NULL;
        case kRTree_BBoxHierarchyType:
            return SkNEW(SkRTreeFactory);
    }
    SkASSERT(0); // invalid bbhType
    return NULL;
}

} // namespace sk_tools
