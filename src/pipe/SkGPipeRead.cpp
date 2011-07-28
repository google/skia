
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkGPipe.h"
#include "SkGPipePriv.h"
#include "SkReader32.h"
#include "SkStream.h"

#include "SkColorFilter.h"
#include "SkDrawLooper.h"
#include "SkMaskFilter.h"
#include "SkPathEffect.h"
#include "SkRasterizer.h"
#include "SkShader.h"
#include "SkTypeface.h"
#include "SkXfermode.h"

static void set_paintflat(SkPaint* paint, SkFlattenable* obj, unsigned paintFlat) {
    SkASSERT(paintFlat < kCount_PaintFlats);
    switch (paintFlat) {
        case kColorFilter_PaintFlat:
            paint->setColorFilter((SkColorFilter*)obj);
            break;
        case kDrawLooper_PaintFlat:
            paint->setLooper((SkDrawLooper*)obj);
            break;
        case kMaskFilter_PaintFlat:
            paint->setMaskFilter((SkMaskFilter*)obj);
            break;
        case kPathEffect_PaintFlat:
            paint->setPathEffect((SkPathEffect*)obj);
            break;
        case kRasterizer_PaintFlat:
            paint->setRasterizer((SkRasterizer*)obj);
            break;
        case kShader_PaintFlat:
            paint->setShader((SkShader*)obj);
            break;
        case kXfermode_PaintFlat:
            paint->setXfermode((SkXfermode*)obj);
            break;
        default:
            SkASSERT(!"never gets here");
    }
}

template <typename T> class SkRefCntTDArray : public SkTDArray<T> {
public:
    ~SkRefCntTDArray() { this->unrefAll(); }
};

class SkGPipeState {
public:
    SkGPipeState();
    ~SkGPipeState();

    void setReader(SkFlattenableReadBuffer* reader) {
        fReader = reader;
        fReader->setFactoryArray(&fFactoryArray);
    }

    const SkPaint& paint() const { return fPaint; }
    SkPaint* editPaint() { return &fPaint; }
    
    SkFlattenable* getFlat(unsigned index) const {
        if (0 == index) {
            return NULL;
        }
        return fFlatArray[index - 1];
    }

    void defFlattenable(PaintFlats pf, int index) {
        SkASSERT(index == fFlatArray.count() + 1);
        SkFlattenable* obj = fReader->readFlattenable();
        *fFlatArray.append() = obj;
    }

    void addTypeface() {
        size_t size = fReader->readU32();
        const void* data = fReader->skip(SkAlign4(size));
        SkMemoryStream stream(data, size, false);
        *fTypefaces.append() = SkTypeface::Deserialize(&stream);
    }    
    void setTypeface(SkPaint* paint, unsigned id) {
        paint->setTypeface(id ? fTypefaces[id - 1] : NULL);
    }
    
    SkFlattenableReadBuffer* fReader;

private:
    SkPaint                   fPaint;
    SkTDArray<SkFlattenable*> fFlatArray;
    SkTDArray<SkTypeface*>    fTypefaces;
    SkTDArray<SkFlattenable::Factory> fFactoryArray;
};

///////////////////////////////////////////////////////////////////////////////

template <typename T> const T* skip(SkReader32* reader, int count = 1) {
    SkASSERT(count >= 0);
    size_t size = sizeof(T) * count;
    SkASSERT(SkAlign4(size) == size);
    return reinterpret_cast<const T*>(reader->skip(size));
}

template <typename T> const T* skipAlign(SkReader32* reader, int count = 1) {
    SkASSERT(count >= 0);
    size_t size = SkAlign4(sizeof(T) * count);
    return reinterpret_cast<const T*>(reader->skip(size));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void clipPath_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                        SkGPipeState* state) {
    SkPath path;
    path.unflatten(*reader);
    canvas->clipPath(path, (SkRegion::Op)DrawOp_unpackData(op32));
}

static void clipRegion_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                          SkGPipeState* state) {
    SkRegion rgn;
    SkReadRegion(reader, &rgn);
    canvas->clipRegion(rgn, (SkRegion::Op)DrawOp_unpackData(op32));
}

static void clipRect_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                        SkGPipeState* state) {
    canvas->clipRect(*skip<SkRect>(reader), (SkRegion::Op)DrawOp_unpackData(op32));
}

///////////////////////////////////////////////////////////////////////////////

static void setMatrix_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                      SkGPipeState* state) {
    SkMatrix matrix;
    SkReadMatrix(reader, &matrix);
    canvas->setMatrix(matrix);
}

static void concat_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                      SkGPipeState* state) {
    SkMatrix matrix;
    SkReadMatrix(reader, &matrix);
    canvas->concat(matrix);
}

static void scale_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                      SkGPipeState* state) {
    const SkScalar* param = skip<SkScalar>(reader, 2);
    canvas->scale(param[0], param[1]);
}

static void skew_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                      SkGPipeState* state) {
    const SkScalar* param = skip<SkScalar>(reader, 2);
    canvas->skew(param[0], param[1]);
}

static void rotate_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                      SkGPipeState* state) {
    canvas->rotate(reader->readScalar());
}

static void translate_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                      SkGPipeState* state) {
    const SkScalar* param = skip<SkScalar>(reader, 2);
    canvas->translate(param[0], param[1]);
}

///////////////////////////////////////////////////////////////////////////////

static void save_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                    SkGPipeState* state) {
    canvas->save((SkCanvas::SaveFlags)DrawOp_unpackData(op32));
}

static void saveLayer_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                         SkGPipeState* state) {
    unsigned flags = DrawOp_unpackFlags(op32);
    SkCanvas::SaveFlags saveFlags = (SkCanvas::SaveFlags)DrawOp_unpackData(op32);

    const SkRect* bounds = NULL;
    if (flags & kSaveLayer_HasBounds_DrawOpFlag) {
        bounds = skip<SkRect>(reader);
    }
    const SkPaint* paint = NULL;
    if (flags & kSaveLayer_HasPaint_DrawOpFlag) {
        paint = &state->paint();
    }
    canvas->saveLayer(bounds, paint, saveFlags);
}

static void restore_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                       SkGPipeState* state) {
    canvas->restore();
}

///////////////////////////////////////////////////////////////////////////////

static void drawClear_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                         SkGPipeState* state) {
    SkColor color = 0;
    if (DrawOp_unpackFlags(op32) & kClear_HasColor_DrawOpFlag) {
        color = reader->readU32();
    }
    canvas->clear(color);
}

static void drawPaint_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                         SkGPipeState* state) {
    canvas->drawPaint(state->paint());
}

static void drawPoints_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                          SkGPipeState* state) {
    SkCanvas::PointMode mode = (SkCanvas::PointMode)DrawOp_unpackFlags(op32);
    size_t count = reader->readU32();
    const SkPoint* pts = skip<SkPoint>(reader, count);
    canvas->drawPoints(mode, count, pts, state->paint());
}

static void drawRect_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                        SkGPipeState* state) {
    canvas->drawRect(*skip<SkRect>(reader), state->paint());
}

static void drawPath_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                        SkGPipeState* state) {
    SkPath path;
    path.unflatten(*reader);
    canvas->drawPath(path, state->paint());
}

static void drawVertices_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                            SkGPipeState* state) {
    unsigned flags = DrawOp_unpackFlags(op32);

    SkCanvas::VertexMode mode = (SkCanvas::VertexMode)reader->readU32();
    int vertexCount = reader->readU32();
    const SkPoint* verts = skip<SkPoint>(reader, vertexCount);

    const SkPoint* texs = NULL;
    if (flags & kDrawVertices_HasTexs_DrawOpFlag) {
        texs = skip<SkPoint>(reader, vertexCount);
    }

    const SkColor* colors = NULL;
    if (flags & kDrawVertices_HasColors_DrawOpFlag) {
        colors = skip<SkColor>(reader, vertexCount);
    }

    // TODO: flatten/unflatten xfermodes
    SkXfermode* xfer = NULL;

    int indexCount = 0;
    const uint16_t* indices = NULL;
    if (flags & kDrawVertices_HasIndices_DrawOpFlag) {
        indexCount = reader->readU32();
        indices = skipAlign<uint16_t>(reader, indexCount);
    }

    canvas->drawVertices(mode, vertexCount, verts, texs, colors, xfer,
                         indices, indexCount, state->paint());
}

///////////////////////////////////////////////////////////////////////////////

static void drawText_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                        SkGPipeState* state) {
    size_t len = reader->readU32();
    const void* text = reader->skip(SkAlign4(len));
    const SkScalar* xy = skip<SkScalar>(reader, 2);
    canvas->drawText(text, len, xy[0], xy[1], state->paint());
}

static void drawPosText_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                        SkGPipeState* state) {
    size_t len = reader->readU32();
    const void* text = reader->skip(SkAlign4(len));
    size_t posCount = reader->readU32();    // compute by our writer
    const SkPoint* pos = skip<SkPoint>(reader, posCount);
    canvas->drawPosText(text, len, pos, state->paint());
}

static void drawPosTextH_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                        SkGPipeState* state) {
    size_t len = reader->readU32();
    const void* text = reader->skip(SkAlign4(len));
    size_t posCount = reader->readU32();    // compute by our writer
    const SkScalar* xpos = skip<SkScalar>(reader, posCount);
    SkScalar constY = reader->readScalar();
    canvas->drawPosTextH(text, len, xpos, constY, state->paint());
}

static void drawTextOnPath_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                              SkGPipeState* state) {
    size_t len = reader->readU32();
    const void* text = reader->skip(SkAlign4(len));

    SkPath path;
    path.unflatten(*reader);

    SkMatrix matrixStorage;
    const SkMatrix* matrix = NULL;
    if (DrawOp_unpackFlags(op32) & kDrawTextOnPath_HasMatrix_DrawOpFlag) {
        SkReadMatrix(reader, &matrixStorage);
        matrix = &matrixStorage;
    }

    canvas->drawTextOnPath(text, len, path, matrix, state->paint());
}

///////////////////////////////////////////////////////////////////////////////

static void drawBitmap_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                          SkGPipeState* state) {
    UNIMPLEMENTED
}

static void drawBitmapMatrix_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                                SkGPipeState* state) {
    UNIMPLEMENTED
}

static void drawBitmapRect_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                              SkGPipeState* state) {
    UNIMPLEMENTED
}

static void drawSprite_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                          SkGPipeState* state) {
    UNIMPLEMENTED
}

///////////////////////////////////////////////////////////////////////////////

static void drawData_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                        SkGPipeState* state) {
    // since we don't have a paint, we can use data for our (small) sizes
    size_t size = DrawOp_unpackData(op32);
    if (0 == size) {
        size = reader->readU32();
    }
    const void* data = reader->skip(SkAlign4(size));
    canvas->drawData(data, size);
}

static void drawPicture_rp(SkCanvas* canvas, SkReader32* reader, uint32_t op32,
                           SkGPipeState* state) {
    UNIMPLEMENTED
}

///////////////////////////////////////////////////////////////////////////////

static void paintOp_rp(SkCanvas*, SkReader32* reader, uint32_t op32,
                       SkGPipeState* state) {
    size_t offset = reader->offset();
    size_t stop = offset + PaintOp_unpackData(op32);
    SkPaint* p = state->editPaint();

    do {
        uint32_t p32 = reader->readU32();
        unsigned op = PaintOp_unpackOp(p32);
        unsigned data = PaintOp_unpackData(p32);

//        SkDebugf(" read %08X op=%d flags=%d data=%d\n", p32, op, done, data);

        switch (op) {
            case kReset_PaintOp: p->reset(); break;
            case kFlags_PaintOp: p->setFlags(data); break;
            case kColor_PaintOp: p->setColor(reader->readU32()); break;
            case kStyle_PaintOp: p->setStyle((SkPaint::Style)data); break;
            case kJoin_PaintOp: p->setStrokeJoin((SkPaint::Join)data); break;
            case kCap_PaintOp: p->setStrokeCap((SkPaint::Cap)data); break;
            case kWidth_PaintOp: p->setStrokeWidth(reader->readScalar()); break;
            case kMiter_PaintOp: p->setStrokeMiter(reader->readScalar()); break;
            case kEncoding_PaintOp:
                p->setTextEncoding((SkPaint::TextEncoding)data);
                break;
            case kHinting_PaintOp: p->setHinting((SkPaint::Hinting)data); break;
            case kAlign_PaintOp: p->setTextAlign((SkPaint::Align)data); break;
            case kTextSize_PaintOp: p->setTextSize(reader->readScalar()); break;
            case kTextScaleX_PaintOp: p->setTextScaleX(reader->readScalar()); break;
            case kTextSkewX_PaintOp: p->setTextSkewX(reader->readScalar()); break;

            case kFlatIndex_PaintOp: {
                PaintFlats pf = (PaintFlats)PaintOp_unpackFlags(p32);
                unsigned index = data;
                set_paintflat(p, state->getFlat(index), pf);
                break;
            }

            case kTypeface_PaintOp: state->setTypeface(p, data); break;
            default: SkASSERT(!"bad paintop"); return;
        }
        SkASSERT(reader->offset() <= stop);
    } while (reader->offset() < stop);
}

///////////////////////////////////////////////////////////////////////////////

static void def_Typeface_rp(SkCanvas*, SkReader32*, uint32_t, SkGPipeState* state) {
    state->addTypeface();
}

static void def_PaintFlat_rp(SkCanvas*, SkReader32*, uint32_t op32,
                             SkGPipeState* state) {
    PaintFlats pf = (PaintFlats)DrawOp_unpackFlags(op32);
    unsigned index = DrawOp_unpackData(op32);
    state->defFlattenable(pf, index);
}

///////////////////////////////////////////////////////////////////////////////

static void skip_rp(SkCanvas*, SkReader32* reader, uint32_t op32, SkGPipeState*) {
    size_t bytes = DrawOp_unpackData(op32);
    (void)reader->skip(bytes);
}

static void done_rp(SkCanvas*, SkReader32*, uint32_t, SkGPipeState*) {}

typedef void (*ReadProc)(SkCanvas*, SkReader32*, uint32_t op32, SkGPipeState*);

static const ReadProc gReadTable[] = {
    skip_rp,
    clipPath_rp,
    clipRegion_rp,
    clipRect_rp,
    concat_rp,
    drawBitmap_rp,
    drawBitmapMatrix_rp,
    drawBitmapRect_rp,
    drawClear_rp,
    drawData_rp,
    drawPaint_rp,
    drawPath_rp,
    drawPicture_rp,
    drawPoints_rp,
    drawPosText_rp,
    drawPosTextH_rp,
    drawRect_rp,
    drawSprite_rp,
    drawText_rp,
    drawTextOnPath_rp,
    drawVertices_rp,
    restore_rp,
    rotate_rp,
    save_rp,
    saveLayer_rp,
    scale_rp,
    setMatrix_rp,
    skew_rp,
    translate_rp,

    paintOp_rp,
    def_Typeface_rp,
    def_PaintFlat_rp,

    done_rp
};

///////////////////////////////////////////////////////////////////////////////

SkGPipeState::SkGPipeState() {}

SkGPipeState::~SkGPipeState() {
    fTypefaces.safeUnrefAll();
    fFlatArray.safeUnrefAll();
}

///////////////////////////////////////////////////////////////////////////////

#include "SkGPipe.h"

SkGPipeReader::SkGPipeReader(SkCanvas* target) {
    SkSafeRef(target);
    fCanvas = target;
    fState = NULL;
}

SkGPipeReader::~SkGPipeReader() {
    SkSafeUnref(fCanvas);
    delete fState;
}

SkGPipeReader::Status SkGPipeReader::playback(const void* data, size_t length,
                                              size_t* bytesRead, bool readAtom) {
    if (NULL == fCanvas) {
        return kError_Status;
    }

    if (NULL == fState) {
        fState = new SkGPipeState;
    }

    SkASSERT(SK_ARRAY_COUNT(gReadTable) == (kDone_DrawOp + 1));

    const ReadProc* table = gReadTable;
    SkFlattenableReadBuffer reader(data, length);
    SkCanvas* canvas = fCanvas;
    Status status = kEOF_Status;

    fState->setReader(&reader);
    while (!reader.eof()) {
        uint32_t op32 = reader.readU32();
        unsigned op = DrawOp_unpackOp(op32);
        SkDEBUGCODE(DrawOps drawOp = (DrawOps)op;)
        
        if (op >= SK_ARRAY_COUNT(gReadTable)) {
            SkDebugf("---- bad op during GPipeState::playback\n");
            status = kError_Status;
            break;
        }
        if (kDone_DrawOp == op) {
            status = kDone_Status;
            break;
        }
        table[op](canvas, &reader, op32, fState);
        if (readAtom && 
            (table[op] != paintOp_rp &&
             table[op] != def_Typeface_rp &&
             table[op] != def_PaintFlat_rp
             )) {
                status = kReadAtom_Status;
                break;
            }
    }

    if (bytesRead) {
        *bytesRead = reader.offset();
    }
    return status;
}


