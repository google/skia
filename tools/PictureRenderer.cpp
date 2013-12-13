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
#include "SkGPipe.h"
#if SK_SUPPORT_GPU
#include "gl/GrGLDefines.h"
#include "SkGpuDevice.h"
#endif
#include "SkGraphics.h"
#include "SkImageEncoder.h"
#include "SkMaskFilter.h"
#include "SkMatrix.h"
#include "SkPicture.h"
#include "SkPictureUtils.h"
#include "SkPixelRef.h"
#include "SkRTree.h"
#include "SkScalar.h"
#include "SkStream.h"
#include "SkString.h"
#include "SkTemplates.h"
#include "SkTileGridPicture.h"
#include "SkTDArray.h"
#include "SkThreadUtils.h"
#include "SkTypes.h"

namespace sk_tools {

enum {
    kDefaultTileWidth = 256,
    kDefaultTileHeight = 256
};

/* TODO(epoger): These constants are already maintained in 2 other places:
 * gm/gm_json.py and gm/gm_expectations.cpp.  We shouldn't add yet a third place.
 * Figure out a way to share the definitions instead.
 */
const static char kJsonKey_ActualResults[]   = "actual-results";
const static char kJsonKey_ActualResults_Failed[]        = "failed";
const static char kJsonKey_ActualResults_FailureIgnored[]= "failure-ignored";
const static char kJsonKey_ActualResults_NoComparison[]  = "no-comparison";
const static char kJsonKey_ActualResults_Succeeded[]     = "succeeded";
const static char kJsonKey_ExpectedResults[] = "expected-results";
const static char kJsonKey_ExpectedResults_AllowedDigests[] = "allowed-digests";
const static char kJsonKey_ExpectedResults_IgnoreFailure[]  = "ignore-failure";
const static char kJsonKey_Hashtype_Bitmap_64bitMD5[]  = "bitmap-64bitMD5";

void ImageResultsSummary::add(const char *testName, const SkBitmap& bitmap) {
    uint64_t hash;
    SkAssertResult(SkBitmapHasher::ComputeDigest(bitmap, &hash));
    Json::Value jsonTypeValuePair;
    jsonTypeValuePair.append(Json::Value(kJsonKey_Hashtype_Bitmap_64bitMD5));
    jsonTypeValuePair.append(Json::UInt64(hash));
    fActualResultsNoComparison[testName] = jsonTypeValuePair;
}

void ImageResultsSummary::writeToFile(const char *filename) {
    Json::Value actualResults;
    actualResults[kJsonKey_ActualResults_NoComparison] = fActualResultsNoComparison;
    Json::Value root;
    root[kJsonKey_ActualResults] = actualResults;
    std::string jsonStdString = root.toStyledString();
    SkFILEWStream stream(filename);
    stream.write(jsonStdString.c_str(), jsonStdString.length());
}

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
    fPicture->ref();
    fCanvas.reset(this->setupCanvas());
}

class FlagsDrawFilter : public SkDrawFilter {
public:
    FlagsDrawFilter(PictureRenderer::DrawFilterFlags* flags) :
        fFlags(flags) {}

    virtual bool filter(SkPaint* paint, Type t) {
        paint->setFlags(paint->getFlags() & ~fFlags[t] & SkPaint::kAllFlags);
        if (PictureRenderer::kMaskFilter_DrawFilterFlag & fFlags[t]) {
            SkMaskFilter* maskFilter = paint->getMaskFilter();
            if (NULL != maskFilter) {
                paint->setMaskFilter(NULL);
            }
        }
        if (PictureRenderer::kHinting_DrawFilterFlag & fFlags[t]) {
            paint->setHinting(SkPaint::kNo_Hinting);
        } else if (PictureRenderer::kSlightHinting_DrawFilterFlag & fFlags[t]) {
            paint->setHinting(SkPaint::kSlight_Hinting);
        }
        return true;
    }

private:
    PictureRenderer::DrawFilterFlags* fFlags;
};

static void setUpFilter(SkCanvas* canvas, PictureRenderer::DrawFilterFlags* drawFilters) {
    if (drawFilters && !canvas->getDrawFilter()) {
        canvas->setDrawFilter(SkNEW_ARGS(FlagsDrawFilter, (drawFilters)))->unref();
        if (drawFilters[0] & PictureRenderer::kAAClip_DrawFilterFlag) {
            canvas->setAllowSoftClip(false);
        }
    }
}

SkCanvas* PictureRenderer::setupCanvas() {
    const int width = this->getViewWidth();
    const int height = this->getViewHeight();
    return this->setupCanvas(width, height);
}

SkCanvas* PictureRenderer::setupCanvas(int width, int height) {
    SkCanvas* canvas;
    switch(fDeviceType) {
        case kBitmap_DeviceType: {
            SkBitmap bitmap;
            sk_tools::setup_bitmap(&bitmap, width, height);
            canvas = SkNEW_ARGS(SkCanvas, (bitmap));
        }
        break;
#if SK_SUPPORT_GPU
#if SK_ANGLE
        case kAngle_DeviceType:
            // fall through
#endif
        case kGPU_DeviceType: {
            SkAutoTUnref<GrSurface> target;
            if (fGrContext) {
                // create a render target to back the device
                GrTextureDesc desc;
                desc.fConfig = kSkia8888_GrPixelConfig;
                desc.fFlags = kRenderTarget_GrTextureFlagBit;
                desc.fWidth = width;
                desc.fHeight = height;
                desc.fSampleCnt = fSampleCount;
                target.reset(fGrContext->createUncachedTexture(desc, NULL, 0));
            }
            if (NULL == target.get()) {
                SkASSERT(0);
                return NULL;
            }

            SkAutoTUnref<SkGpuDevice> device(SkGpuDevice::Create(target));
            canvas = SkNEW_ARGS(SkCanvas, (device.get()));
            break;
        }
#endif
        default:
            SkASSERT(0);
            return NULL;
    }
    setUpFilter(canvas, fDrawFilters);
    this->scaleToScaleFactor(canvas);
    return canvas;
}

void PictureRenderer::scaleToScaleFactor(SkCanvas* canvas) {
    SkASSERT(canvas != NULL);
    if (fScaleFactor != SK_Scalar1) {
        canvas->scale(fScaleFactor, fScaleFactor);
    }
}

void PictureRenderer::end() {
    this->resetState(true);
    SkSafeUnref(fPicture);
    fPicture = NULL;
    fCanvas.reset(NULL);
}

int PictureRenderer::getViewWidth() {
    SkASSERT(fPicture != NULL);
    int width = SkScalarCeilToInt(fPicture->width() * fScaleFactor);
    if (fViewport.width() > 0) {
        width = SkMin32(width, fViewport.width());
    }
    return width;
}

int PictureRenderer::getViewHeight() {
    SkASSERT(fPicture != NULL);
    int height = SkScalarCeilToInt(fPicture->height() * fScaleFactor);
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
    SkASSERT(NULL != fPicture);
    if (kNone_BBoxHierarchyType != fBBoxHierarchyType && NULL != fPicture) {
        SkPicture* newPicture = this->createPicture();
        SkCanvas* recorder = newPicture->beginRecording(fPicture->width(), fPicture->height(),
                                                        this->recordFlags());
        fPicture->draw(recorder);
        newPicture->endRecording();
        fPicture->unref();
        fPicture = newPicture;
    }
}

void PictureRenderer::resetState(bool callFinish) {
#if SK_SUPPORT_GPU
    SkGLContextHelper* glContext = this->getGLContext();
    if (NULL == glContext) {
        SkASSERT(kBitmap_DeviceType == fDeviceType);
        return;
    }

    fGrContext->flush();
    if (callFinish) {
        SK_GL(*glContext, Finish());
    }
#endif
}

uint32_t PictureRenderer::recordFlags() {
    return ((kNone_BBoxHierarchyType == fBBoxHierarchyType) ? 0 :
        SkPicture::kOptimizeForClippedPlayback_RecordingFlag) |
        SkPicture::kUsePathBoundsForClip_RecordingFlag;
}

/**
 * Write the canvas to the specified path.
 * @param canvas Must be non-null. Canvas to be written to a file.
 * @param path Path for the file to be written. Should have no extension; write() will append
 *             an appropriate one. Passed in by value so it can be modified.
 * @param jsonSummaryPtr If not null, add image results to this summary.
 * @return bool True if the Canvas is written to a file.
 *
 * TODO(epoger): Right now, all canvases must pass through this function in order to be appended
 * to the ImageResultsSummary.  We need some way to add bitmaps to the ImageResultsSummary
 * even if --writePath has not been specified (and thus this function is not called).
 *
 * One fix would be to pass in these path elements separately, and allow this function to be
 * called even if --writePath was not specified...
 *  const char *outputDir   // NULL if we don't want to write image files to disk
 *  const char *filename    // name we use within JSON summary, and as the filename within outputDir
 */
static bool write(SkCanvas* canvas, const SkString* path, ImageResultsSummary *jsonSummaryPtr) {
    SkASSERT(canvas != NULL);
    if (NULL == canvas) {
        return false;
    }

    SkASSERT(path != NULL);  // TODO(epoger): we want to remove this constraint, as noted above
    SkString fullPathname(*path);
    fullPathname.append(".png");

    SkBitmap bitmap;
    SkISize size = canvas->getDeviceSize();
    sk_tools::setup_bitmap(&bitmap, size.width(), size.height());

    canvas->readPixels(&bitmap, 0, 0);
    sk_tools::force_all_opaque(bitmap);

    if (NULL != jsonSummaryPtr) {
        // EPOGER: This is a hacky way of constructing the filename associated with the
        // image checksum; we assume that outputDir is not NULL, and we remove outputDir
        // from fullPathname.
        //
        // EPOGER: what about including the config type within hashFilename?  That way,
        // we could combine results of different config types without conflicting filenames.
        SkString hashFilename;
        sk_tools::get_basename(&hashFilename, fullPathname);
        jsonSummaryPtr->add(hashFilename.c_str(), bitmap);
    }

    return SkImageEncoder::EncodeFile(fullPathname.c_str(), bitmap, SkImageEncoder::kPNG_Type, 100);
}

/**
 * If path is non NULL, append number to it, and call write() to write the
 * provided canvas to a file. Returns true if path is NULL or if write() succeeds.
 */
static bool writeAppendNumber(SkCanvas* canvas, const SkString* path, int number,
                              ImageResultsSummary *jsonSummaryPtr) {
    if (NULL == path) {
        return true;
    }
    SkString pathWithNumber(*path);
    pathWithNumber.appendf("%i", number);
    return write(canvas, &pathWithNumber, jsonSummaryPtr);
}

///////////////////////////////////////////////////////////////////////////////////////////////

SkCanvas* RecordPictureRenderer::setupCanvas(int width, int height) {
    // defer the canvas setup until the render step
    return NULL;
}

static SkData* encode_bitmap_to_data(size_t* offset, const SkBitmap& bm) {
    SkPixelRef* pr = bm.pixelRef();
    if (pr != NULL) {
        SkData* data = pr->refEncodedData();
        if (data != NULL) {
            *offset = bm.pixelRefOffset();
            return data;
        }
    }
    *offset = 0;
    return SkImageEncoder::EncodeData(bm, SkImageEncoder::kPNG_Type, 100);
}

bool RecordPictureRenderer::render(const SkString* path, SkBitmap** out) {
    SkAutoTUnref<SkPicture> replayer(this->createPicture());
    SkCanvas* recorder = replayer->beginRecording(this->getViewWidth(), this->getViewHeight(),
                                                  this->recordFlags());
    this->scaleToScaleFactor(recorder);
    fPicture->draw(recorder);
    replayer->endRecording();
    if (path != NULL) {
        // Record the new picture as a new SKP with PNG encoded bitmaps.
        SkString skpPath(*path);
        // ".skp" was removed from 'path' before being passed in here.
        skpPath.append(".skp");
        SkFILEWStream stream(skpPath.c_str());
        replayer->serialize(&stream, &encode_bitmap_to_data);
        return true;
    }
    return false;
}

SkString RecordPictureRenderer::getConfigNameInternal() {
    return SkString("record");
}

///////////////////////////////////////////////////////////////////////////////////////////////

bool PipePictureRenderer::render(const SkString* path, SkBitmap** out) {
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
    if (NULL != path) {
        return write(fCanvas, path, fJsonSummaryPtr);
    }
    if (NULL != out) {
        *out = SkNEW(SkBitmap);
        setup_bitmap(*out, fPicture->width(), fPicture->height());
        fCanvas->readPixels(*out, 0, 0);
    }
    return true;
}

SkString PipePictureRenderer::getConfigNameInternal() {
    return SkString("pipe");
}

///////////////////////////////////////////////////////////////////////////////////////////////

void SimplePictureRenderer::init(SkPicture* picture) {
    INHERITED::init(picture);
    this->buildBBoxHierarchy();
}

bool SimplePictureRenderer::render(const SkString* path, SkBitmap** out) {
    SkASSERT(fCanvas.get() != NULL);
    SkASSERT(fPicture != NULL);
    if (NULL == fCanvas.get() || NULL == fPicture) {
        return false;
    }

    fCanvas->drawPicture(*fPicture);
    fCanvas->flush();
    if (NULL != path) {
        return write(fCanvas, path, fJsonSummaryPtr);
    }

    if (NULL != out) {
        *out = SkNEW(SkBitmap);
        setup_bitmap(*out, fPicture->width(), fPicture->height());
        fCanvas->readPixels(*out, 0, 0);
    }

    return true;
}

SkString SimplePictureRenderer::getConfigNameInternal() {
    return SkString("simple");
}

///////////////////////////////////////////////////////////////////////////////////////////////

TiledPictureRenderer::TiledPictureRenderer()
    : fTileWidth(kDefaultTileWidth)
    , fTileHeight(kDefaultTileHeight)
    , fTileWidthPercentage(0.0)
    , fTileHeightPercentage(0.0)
    , fTileMinPowerOf2Width(0)
    , fCurrentTileOffset(-1)
    , fTilesX(0)
    , fTilesY(0) { }

void TiledPictureRenderer::init(SkPicture* pict) {
    SkASSERT(pict != NULL);
    SkASSERT(0 == fTileRects.count());
    if (NULL == pict || fTileRects.count() != 0) {
        return;
    }

    // Do not call INHERITED::init(), which would create a (potentially large) canvas which is not
    // used by bench_pictures.
    fPicture = pict;
    fPicture->ref();
    this->buildBBoxHierarchy();

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
            *fTileRects.append() = SkRect::MakeXYWH(SkIntToScalar(tile_x_start),
                                                    SkIntToScalar(tile_y_start),
                                                    SkIntToScalar(fTileWidth),
                                                    SkIntToScalar(fTileHeight));
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

    int num_bits = SkScalarCeilToInt(SkScalarLog2(SkIntToScalar(width)));
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
    // Translate so that we draw the correct portion of the picture.
    // Perform a postTranslate so that the scaleFactor does not interfere with the positioning.
    SkMatrix mat(canvas->getTotalMatrix());
    mat.postTranslate(-tileRect.fLeft, -tileRect.fTop);
    canvas->setMatrix(mat);
    playback->draw(canvas);
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
    DrawTileToCanvas(fCanvas, fTileRects[fCurrentTileOffset], fPicture);
}

bool TiledPictureRenderer::render(const SkString* path, SkBitmap** out) {
    SkASSERT(fPicture != NULL);
    if (NULL == fPicture) {
        return false;
    }

    SkBitmap bitmap;
    if (out){
        *out = SkNEW(SkBitmap);
        setup_bitmap(*out, fPicture->width(), fPicture->height());
        setup_bitmap(&bitmap, fTileWidth, fTileHeight);
    }
    bool success = true;
    for (int i = 0; i < fTileRects.count(); ++i) {
        DrawTileToCanvas(fCanvas, fTileRects[i], fPicture);
        if (NULL != path) {
            success &= writeAppendNumber(fCanvas, path, i, fJsonSummaryPtr);
        }
        if (NULL != out) {
            if (fCanvas->readPixels(&bitmap, 0, 0)) {
                // Add this tile to the entire bitmap.
                bitmapCopyAtOffset(bitmap, *out, SkScalarFloorToInt(fTileRects[i].left()),
                                   SkScalarFloorToInt(fTileRects[i].top()));
            } else {
                success = false;
            }
        }
    }
    return success;
}

SkCanvas* TiledPictureRenderer::setupCanvas(int width, int height) {
    SkCanvas* canvas = this->INHERITED::setupCanvas(width, height);
    SkASSERT(fPicture != NULL);
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

// Holds all of the information needed to draw a set of tiles.
class CloneData : public SkRunnable {

public:
    CloneData(SkPicture* clone, SkCanvas* canvas, SkTDArray<SkRect>& rects, int start, int end,
              SkRunnable* done, ImageResultsSummary* jsonSummaryPtr)
        : fClone(clone)
        , fCanvas(canvas)
        , fPath(NULL)
        , fRects(rects)
        , fStart(start)
        , fEnd(end)
        , fSuccess(NULL)
        , fDone(done)
        , fJsonSummaryPtr(jsonSummaryPtr) {
        SkASSERT(fDone != NULL);
    }

    virtual void run() SK_OVERRIDE {
        SkGraphics::SetTLSFontCacheLimit(1024 * 1024);

        SkBitmap bitmap;
        if (fBitmap != NULL) {
            // All tiles are the same size.
            setup_bitmap(&bitmap, SkScalarFloorToInt(fRects[0].width()), SkScalarFloorToInt(fRects[0].height()));
        }

        for (int i = fStart; i < fEnd; i++) {
            DrawTileToCanvas(fCanvas, fRects[i], fClone);
            if ((fPath != NULL) && !writeAppendNumber(fCanvas, fPath, i, fJsonSummaryPtr)
                && fSuccess != NULL) {
                *fSuccess = false;
                // If one tile fails to write to a file, do not continue drawing the rest.
                break;
            }
            if (fBitmap != NULL) {
                if (fCanvas->readPixels(&bitmap, 0, 0)) {
                    SkAutoLockPixels alp(*fBitmap);
                    bitmapCopyAtOffset(bitmap, fBitmap, SkScalarFloorToInt(fRects[i].left()),
                                       SkScalarFloorToInt(fRects[i].top()));
                } else {
                    *fSuccess = false;
                    // If one tile fails to read pixels, do not continue drawing the rest.
                    break;
                }
            }
        }
        fDone->run();
    }

    void setPathAndSuccess(const SkString* path, bool* success) {
        fPath = path;
        fSuccess = success;
    }

    void setBitmap(SkBitmap* bitmap) {
        fBitmap = bitmap;
    }

private:
    // All pointers unowned.
    SkPicture*         fClone;      // Picture to draw from. Each CloneData has a unique one which
                                    // is threadsafe.
    SkCanvas*          fCanvas;     // Canvas to draw to. Reused for each tile.
    const SkString*    fPath;       // If non-null, path to write the result to as a PNG.
    SkTDArray<SkRect>& fRects;      // All tiles of the picture.
    const int          fStart;      // Range of tiles drawn by this thread.
    const int          fEnd;
    bool*              fSuccess;    // Only meaningful if path is non-null. Shared by all threads,
                                    // and only set to false upon failure to write to a PNG.
    SkRunnable*        fDone;
    SkBitmap*          fBitmap;
    ImageResultsSummary* fJsonSummaryPtr;
};

MultiCorePictureRenderer::MultiCorePictureRenderer(int threadCount)
: fNumThreads(threadCount)
, fThreadPool(threadCount)
, fCountdown(threadCount) {
    // Only need to create fNumThreads - 1 clones, since one thread will use the base
    // picture.
    fPictureClones = SkNEW_ARRAY(SkPicture, fNumThreads - 1);
    fCloneData = SkNEW_ARRAY(CloneData*, fNumThreads);
}

void MultiCorePictureRenderer::init(SkPicture *pict) {
    // Set fPicture and the tiles.
    this->INHERITED::init(pict);
    for (int i = 0; i < fNumThreads; ++i) {
        *fCanvasPool.append() = this->setupCanvas(this->getTileWidth(), this->getTileHeight());
    }
    // Only need to create fNumThreads - 1 clones, since one thread will use the base picture.
    fPicture->clone(fPictureClones, fNumThreads - 1);
    // Populate each thread with the appropriate data.
    // Group the tiles into nearly equal size chunks, rounding up so we're sure to cover them all.
    const int chunkSize = (fTileRects.count() + fNumThreads - 1) / fNumThreads;

    for (int i = 0; i < fNumThreads; i++) {
        SkPicture* pic;
        if (i == fNumThreads-1) {
            // The last set will use the original SkPicture.
            pic = fPicture;
        } else {
            pic = &fPictureClones[i];
        }
        const int start = i * chunkSize;
        const int end = SkMin32(start + chunkSize, fTileRects.count());
        fCloneData[i] = SkNEW_ARGS(CloneData,
                                   (pic, fCanvasPool[i], fTileRects, start, end, &fCountdown,
                                    fJsonSummaryPtr));
    }
}

bool MultiCorePictureRenderer::render(const SkString *path, SkBitmap** out) {
    bool success = true;
    if (path != NULL) {
        for (int i = 0; i < fNumThreads-1; i++) {
            fCloneData[i]->setPathAndSuccess(path, &success);
        }
    }

    if (NULL != out) {
        *out = SkNEW(SkBitmap);
        setup_bitmap(*out, fPicture->width(), fPicture->height());
        for (int i = 0; i < fNumThreads; i++) {
            fCloneData[i]->setBitmap(*out);
        }
    } else {
        for (int i = 0; i < fNumThreads; i++) {
            fCloneData[i]->setBitmap(NULL);
        }
    }

    fCountdown.reset(fNumThreads);
    for (int i = 0; i < fNumThreads; i++) {
        fThreadPool.add(fCloneData[i]);
    }
    fCountdown.wait();

    return success;
}

void MultiCorePictureRenderer::end() {
    for (int i = 0; i < fNumThreads - 1; i++) {
        SkDELETE(fCloneData[i]);
        fCloneData[i] = NULL;
    }

    fCanvasPool.unrefAll();

    this->INHERITED::end();
}

MultiCorePictureRenderer::~MultiCorePictureRenderer() {
    // Each individual CloneData was deleted in end.
    SkDELETE_ARRAY(fCloneData);
    SkDELETE_ARRAY(fPictureClones);
}

SkString MultiCorePictureRenderer::getConfigNameInternal() {
    SkString name = this->INHERITED::getConfigNameInternal();
    name.appendf("_multi_%i_threads", fNumThreads);
    return name;
}

///////////////////////////////////////////////////////////////////////////////////////////////

void PlaybackCreationRenderer::setup() {
    fReplayer.reset(this->createPicture());
    SkCanvas* recorder = fReplayer->beginRecording(this->getViewWidth(), this->getViewHeight(),
                                                   this->recordFlags());
    this->scaleToScaleFactor(recorder);
    fPicture->draw(recorder);
}

bool PlaybackCreationRenderer::render(const SkString*, SkBitmap** out) {
    fReplayer->endRecording();
    // Since this class does not actually render, return false.
    return false;
}

SkString PlaybackCreationRenderer::getConfigNameInternal() {
    return SkString("playback_creation");
}

///////////////////////////////////////////////////////////////////////////////////////////////
// SkPicture variants for each BBoxHierarchy type

class RTreePicture : public SkPicture {
public:
    virtual SkBBoxHierarchy* createBBoxHierarchy() const SK_OVERRIDE{
        static const int kRTreeMinChildren = 6;
        static const int kRTreeMaxChildren = 11;
        SkScalar aspectRatio = SkScalarDiv(SkIntToScalar(fWidth),
                                           SkIntToScalar(fHeight));
        bool sortDraws = false;
        return SkRTree::Create(kRTreeMinChildren, kRTreeMaxChildren,
                               aspectRatio, sortDraws);
    }
};

SkPicture* PictureRenderer::createPicture() {
    switch (fBBoxHierarchyType) {
        case kNone_BBoxHierarchyType:
            return SkNEW(SkPicture);
        case kRTree_BBoxHierarchyType:
            return SkNEW(RTreePicture);
        case kTileGrid_BBoxHierarchyType:
            return SkNEW_ARGS(SkTileGridPicture, (fPicture->width(),
                fPicture->height(), fGridInfo));
    }
    SkASSERT(0); // invalid bbhType
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////

class GatherRenderer : public PictureRenderer {
public:
    virtual bool render(const SkString* path, SkBitmap** out = NULL)
            SK_OVERRIDE {
        SkRect bounds = SkRect::MakeWH(SkIntToScalar(fPicture->width()),
                                       SkIntToScalar(fPicture->height()));
        SkData* data = SkPictureUtils::GatherPixelRefs(fPicture, bounds);
        SkSafeUnref(data);

        return NULL == path;    // we don't have anything to write
    }

private:
    virtual SkString getConfigNameInternal() SK_OVERRIDE {
        return SkString("gather_pixelrefs");
    }
};

PictureRenderer* CreateGatherPixelRefsRenderer() {
    return SkNEW(GatherRenderer);
}

///////////////////////////////////////////////////////////////////////////////

class PictureCloneRenderer : public PictureRenderer {
public:
    virtual bool render(const SkString* path, SkBitmap** out = NULL)
            SK_OVERRIDE {
        for (int i = 0; i < 100; ++i) {
            SkPicture* clone = fPicture->clone();
            SkSafeUnref(clone);
        }

        return NULL == path;    // we don't have anything to write
    }

private:
    virtual SkString getConfigNameInternal() SK_OVERRIDE {
        return SkString("picture_clone");
    }
};

PictureRenderer* CreatePictureCloneRenderer() {
    return SkNEW(PictureCloneRenderer);
}

} // namespace sk_tools
