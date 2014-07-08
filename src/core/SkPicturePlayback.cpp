/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkPictureData.h"
#include "SkPicturePlayback.h"
#include "SkPictureRecord.h"
#include "SkPictureStateTree.h"
#include "SkReader32.h"
#include "SkTDArray.h"
#include "SkTypes.h"

SkPicturePlayback::PlaybackReplacements::ReplacementInfo*
SkPicturePlayback::PlaybackReplacements::push() {
    SkDEBUGCODE(this->validate());
    return fReplacements.push();
}

void SkPicturePlayback::PlaybackReplacements::freeAll() {
    for (int i = 0; i < fReplacements.count(); ++i) {
        SkDELETE(fReplacements[i].fBM);
    }
    fReplacements.reset();
}

#ifdef SK_DEBUG
void SkPicturePlayback::PlaybackReplacements::validate() const {
    // Check that the ranges are monotonically increasing and non-overlapping
    if (fReplacements.count() > 0) {
        SkASSERT(fReplacements[0].fStart < fReplacements[0].fStop);

        for (int i = 1; i < fReplacements.count(); ++i) {
            SkASSERT(fReplacements[i].fStart < fReplacements[i].fStop);
            SkASSERT(fReplacements[i - 1].fStop < fReplacements[i].fStart);
        }
    }
}
#endif

// TODO: Replace with hash or pass in "lastLookedUp" hint
SkPicturePlayback::PlaybackReplacements::ReplacementInfo*
SkPicturePlayback::PlaybackReplacements::lookupByStart(size_t start) {
    SkDEBUGCODE(this->validate());
    for (int i = 0; i < fReplacements.count(); ++i) {
        if (start == fReplacements[i].fStart) {
            return &fReplacements[i];
        } else if (start < fReplacements[i].fStart) {
            return NULL;  // the ranges are monotonically increasing and non-overlapping
        }
    }

    return NULL;
}

/*
 * Read the next op code and chunk size from 'reader'. The returned size
 * is the entire size of the chunk (including the opcode). Thus, the
 * offset just prior to calling ReadOpAndSize + 'size' is the offset
 * to the next chunk's op code. This also means that the size of a chunk
 * with no arguments (just an opcode) will be 4.
 */
DrawType SkPicturePlayback::ReadOpAndSize(SkReader32* reader, uint32_t* size) {
    uint32_t temp = reader->readInt();
    uint32_t op;
    if (((uint8_t)temp) == temp) {
        // old skp file - no size information
        op = temp;
        *size = 0;
    } else {
        UNPACK_8_24(temp, op, *size);
        if (MASK_24 == *size) {
            *size = reader->readInt();
        }
    }
    return (DrawType)op;
}


static const SkRect* get_rect_ptr(SkReader32* reader) {
    if (reader->readBool()) {
        return &reader->skipT<SkRect>();
    } else {
        return NULL;
    }
}

class TextContainer {
public:
    size_t length() { return fByteLength; }
    const void* text() { return (const void*)fText; }
    size_t fByteLength;
    const char* fText;
};

void get_text(SkReader32* reader, TextContainer* text) {
    size_t length = text->fByteLength = reader->readInt();
    text->fText = (const char*)reader->skip(length);
}

// FIXME: SkBitmaps are stateful, so we need to copy them to play back in multiple threads.
static SkBitmap shallow_copy(const SkBitmap& bitmap) {
    return bitmap;
}

void SkPicturePlayback::draw(SkCanvas* canvas, SkDrawPictureCallback* callback) {
    AutoResetOpID aroi(this);
    SkASSERT(0 == fCurOffset);

    // kDrawComplete will be the signal that we have reached the end of
    // the command stream
    static const uint32_t kDrawComplete = SK_MaxU32;

    SkReader32 reader(fPictureData->fOpData->bytes(), fPictureData->fOpData->size());
    SkAutoTDelete<const SkPicture::OperationList> activeOpsList;
    const SkTDArray<void*>* activeOps = NULL;

    if (fUseBBH && NULL != fPictureData->fStateTree && NULL != fPictureData->fBoundingHierarchy) {
        SkRect clipBounds;
        if (canvas->getClipBounds(&clipBounds)) {
            SkIRect query;
            clipBounds.roundOut(&query);

            activeOpsList.reset(fPictureData->getActiveOps(query));
            if (NULL != activeOpsList.get()) {
                if (0 == activeOpsList->numOps()) {
                    return;     // nothing to draw
                }

                activeOps = &(activeOpsList.get()->fOps);
            }
        }
    }

    SkPictureStateTree::Iterator it = (NULL == activeOps) ?
        SkPictureStateTree::Iterator() :
        fPictureData->fStateTree->getIterator(*activeOps, canvas);

    if (it.isValid()) {
        uint32_t skipTo = it.nextDraw();
        if (kDrawComplete == skipTo) {
            return;
        }
        reader.setOffset(skipTo);
    }

    // Record this, so we can concat w/ it if we encounter a setMatrix()
    SkMatrix initialMatrix = canvas->getTotalMatrix();

    SkAutoCanvasRestore acr(canvas, false);

#ifdef SK_DEVELOPER
    int opIndex = -1;
#endif

    while (!reader.eof()) {
        if (callback && callback->abortDrawing()) {
            return;
        }

        if (NULL != fReplacements) {
            // Potentially replace a block of operations with a single drawBitmap call
            SkPicturePlayback::PlaybackReplacements::ReplacementInfo* temp =
                fReplacements->lookupByStart(reader.offset());
            if (NULL != temp) {
                SkASSERT(NULL != temp->fBM);
                SkASSERT(NULL != temp->fPaint);
                canvas->save();
                canvas->setMatrix(initialMatrix);
                SkRect src = SkRect::Make(temp->fSrcRect);
                SkRect dst = SkRect::MakeXYWH(temp->fPos.fX, temp->fPos.fY,
                                              temp->fSrcRect.width(),
                                              temp->fSrcRect.height());
                canvas->drawBitmapRectToRect(*temp->fBM, &src, dst, temp->fPaint);
                canvas->restore();

                if (it.isValid()) {
                    // This save is needed since the BBH will automatically issue
                    // a restore to balanced the saveLayer we're skipping
                    canvas->save();

                    // At this point we know that the PictureStateTree was aiming
                    // for some draw op within temp's saveLayer (although potentially
                    // in a separate saveLayer nested inside it).
                    // We need to skip all the operations inside temp's range
                    // along with all the associated state changes but update
                    // the state tree to the first operation outside temp's range.

                    uint32_t skipTo;
                    do {
                        skipTo = it.nextDraw();
                        if (kDrawComplete == skipTo) {
                            break;
                        }

                        if (skipTo <= temp->fStop) {
                            reader.setOffset(skipTo);
                            uint32_t size;
                            DrawType op = ReadOpAndSize(&reader, &size);
                            // Since we are relying on the normal SkPictureStateTree
                            // playback we need to convert any nested saveLayer calls
                            // it may issue into saves (so that all its internal
                            // restores will be balanced).
                            if (SAVE_LAYER == op) {
                                canvas->save();
                            }
                        }
                    } while (skipTo <= temp->fStop);

                    if (kDrawComplete == skipTo) {
                        break;
                    }

                    reader.setOffset(skipTo);
                } else {
                    reader.setOffset(temp->fStop);
                    uint32_t size;
                    SkDEBUGCODE(DrawType op = ) ReadOpAndSize(&reader, &size);
                    SkASSERT(RESTORE == op);
                }
                continue;
            }
        }

        fCurOffset = reader.offset();
        uint32_t size;
        DrawType op = ReadOpAndSize(&reader, &size);
        size_t skipTo = 0;
        if (NOOP == op) {
            // NOOPs are to be ignored - do not propagate them any further
            skipTo = fCurOffset + size;
#ifdef SK_DEVELOPER
        } else {
            opIndex++;
            if (this->preDraw(opIndex, op)) {
                skipTo = fCurOffset + size;
            }
#endif
        }

        if (0 != skipTo) {
            if (it.isValid()) {
                // If using a bounding box hierarchy, advance the state tree
                // iterator until at or after skipTo
                uint32_t adjustedSkipTo;
                do {
                    adjustedSkipTo = it.nextDraw();
                } while (adjustedSkipTo < skipTo);
                skipTo = adjustedSkipTo;
            }
            if (kDrawComplete == skipTo) {
                break;
            }
            reader.setOffset(skipTo);
            continue;
        }

        this->handleOp(&reader, op, size, canvas, initialMatrix);

#ifdef SK_DEVELOPER
        this->postDraw(opIndex);
#endif

        if (it.isValid()) {
            uint32_t skipTo = it.nextDraw();
            if (kDrawComplete == skipTo) {
                break;
            }
            reader.setOffset(skipTo);
        }
    }
}

void SkPicturePlayback::handleOp(SkReader32* reader, 
                                 DrawType op, 
                                 uint32_t size, 
                                 SkCanvas* canvas,
                                 const SkMatrix& initialMatrix) {
    switch (op) {
        case CLIP_PATH: {
            const SkPath& path = fPictureData->getPath(reader);
            uint32_t packed = reader->readInt();
            SkRegion::Op regionOp = ClipParams_unpackRegionOp(packed);
            bool doAA = ClipParams_unpackDoAA(packed);
            size_t offsetToRestore = reader->readInt();
            SkASSERT(!offsetToRestore || offsetToRestore >= reader->offset());
            canvas->clipPath(path, regionOp, doAA);
            if (canvas->isClipEmpty() && offsetToRestore) {
                reader->setOffset(offsetToRestore);
            }
        } break;
        case CLIP_REGION: {
            SkRegion region;
            reader->readRegion(&region);
            uint32_t packed = reader->readInt();
            SkRegion::Op regionOp = ClipParams_unpackRegionOp(packed);
            size_t offsetToRestore = reader->readInt();
            SkASSERT(!offsetToRestore || offsetToRestore >= reader->offset());
            canvas->clipRegion(region, regionOp);
            if (canvas->isClipEmpty() && offsetToRestore) {
                reader->setOffset(offsetToRestore);
            }
        } break;
        case CLIP_RECT: {
            const SkRect& rect = reader->skipT<SkRect>();
            uint32_t packed = reader->readInt();
            SkRegion::Op regionOp = ClipParams_unpackRegionOp(packed);
            bool doAA = ClipParams_unpackDoAA(packed);
            size_t offsetToRestore = reader->readInt();
            SkASSERT(!offsetToRestore || offsetToRestore >= reader->offset());
            canvas->clipRect(rect, regionOp, doAA);
            if (canvas->isClipEmpty() && offsetToRestore) {
                reader->setOffset(offsetToRestore);
            }
        } break;
        case CLIP_RRECT: {
            SkRRect rrect;
            reader->readRRect(&rrect);
            uint32_t packed = reader->readInt();
            SkRegion::Op regionOp = ClipParams_unpackRegionOp(packed);
            bool doAA = ClipParams_unpackDoAA(packed);
            size_t offsetToRestore = reader->readInt();
            SkASSERT(!offsetToRestore || offsetToRestore >= reader->offset());
            canvas->clipRRect(rrect, regionOp, doAA);
            if (canvas->isClipEmpty() && offsetToRestore) {
                reader->setOffset(offsetToRestore);
            }
        } break;
        case PUSH_CULL: {
            const SkRect& cullRect = reader->skipT<SkRect>();
            size_t offsetToRestore = reader->readInt();
            if (offsetToRestore && canvas->quickReject(cullRect)) {
                reader->setOffset(offsetToRestore);
            } else {
                canvas->pushCull(cullRect);
            }
        } break;
        case POP_CULL:
            canvas->popCull();
            break;
        case CONCAT: {
            SkMatrix matrix;
            reader->readMatrix(&matrix);
            canvas->concat(matrix);
            break;
        }
        case DRAW_BITMAP: {
            const SkPaint* paint = fPictureData->getPaint(reader);
            const SkBitmap bitmap = shallow_copy(fPictureData->getBitmap(reader));
            const SkPoint& loc = reader->skipT<SkPoint>();
            canvas->drawBitmap(bitmap, loc.fX, loc.fY, paint);
        } break;
        case DRAW_BITMAP_RECT_TO_RECT: {
            const SkPaint* paint = fPictureData->getPaint(reader);
            const SkBitmap bitmap = shallow_copy(fPictureData->getBitmap(reader));
            const SkRect* src = get_rect_ptr(reader);   // may be null
            const SkRect& dst = reader->skipT<SkRect>();     // required
            SkCanvas::DrawBitmapRectFlags flags;
            flags = (SkCanvas::DrawBitmapRectFlags) reader->readInt();
            canvas->drawBitmapRectToRect(bitmap, src, dst, paint, flags);
        } break;
        case DRAW_BITMAP_MATRIX: {
            const SkPaint* paint = fPictureData->getPaint(reader);
            const SkBitmap bitmap = shallow_copy(fPictureData->getBitmap(reader));
            SkMatrix matrix;
            reader->readMatrix(&matrix);
            canvas->drawBitmapMatrix(bitmap, matrix, paint);
        } break;
        case DRAW_BITMAP_NINE: {
            const SkPaint* paint = fPictureData->getPaint(reader);
            const SkBitmap bitmap = shallow_copy(fPictureData->getBitmap(reader));
            const SkIRect& src = reader->skipT<SkIRect>();
            const SkRect& dst = reader->skipT<SkRect>();
            canvas->drawBitmapNine(bitmap, src, dst, paint);
        } break;
        case DRAW_CLEAR:
            canvas->clear(reader->readInt());
            break;
        case DRAW_DATA: {
            size_t length = reader->readInt();
            canvas->drawData(reader->skip(length), length);
            // skip handles padding the read out to a multiple of 4
        } break;
        case DRAW_DRRECT: {
            const SkPaint& paint = *fPictureData->getPaint(reader);
            SkRRect outer, inner;
            reader->readRRect(&outer);
            reader->readRRect(&inner);
            canvas->drawDRRect(outer, inner, paint);
        } break;
        case BEGIN_COMMENT_GROUP: {
            const char* desc = reader->readString();
            canvas->beginCommentGroup(desc);
        } break;
        case COMMENT: {
            const char* kywd = reader->readString();
            const char* value = reader->readString();
            canvas->addComment(kywd, value);
        } break;
        case END_COMMENT_GROUP: {
            canvas->endCommentGroup();
        } break;
        case DRAW_OVAL: {
            const SkPaint& paint = *fPictureData->getPaint(reader);
            canvas->drawOval(reader->skipT<SkRect>(), paint);
        } break;
        case DRAW_PAINT:
            canvas->drawPaint(*fPictureData->getPaint(reader));
            break;
        case DRAW_PATH: {
            const SkPaint& paint = *fPictureData->getPaint(reader);
            canvas->drawPath(fPictureData->getPath(reader), paint);
        } break;
        case DRAW_PICTURE:
            canvas->drawPicture(fPictureData->getPicture(reader));
            break;
        case DRAW_POINTS: {
            const SkPaint& paint = *fPictureData->getPaint(reader);
            SkCanvas::PointMode mode = (SkCanvas::PointMode)reader->readInt();
            size_t count = reader->readInt();
            const SkPoint* pts = (const SkPoint*)reader->skip(sizeof(SkPoint)* count);
            canvas->drawPoints(mode, count, pts, paint);
        } break;
        case DRAW_POS_TEXT: {
            const SkPaint& paint = *fPictureData->getPaint(reader);
            TextContainer text;
            get_text(reader, &text);
            size_t points = reader->readInt();
            const SkPoint* pos = (const SkPoint*)reader->skip(points * sizeof(SkPoint));
            canvas->drawPosText(text.text(), text.length(), pos, paint);
        } break;
        case DRAW_POS_TEXT_TOP_BOTTOM: {
            const SkPaint& paint = *fPictureData->getPaint(reader);
            TextContainer text;
            get_text(reader, &text);
            size_t points = reader->readInt();
            const SkPoint* pos = (const SkPoint*)reader->skip(points * sizeof(SkPoint));
            const SkScalar top = reader->readScalar();
            const SkScalar bottom = reader->readScalar();
            if (!canvas->quickRejectY(top, bottom)) {
                canvas->drawPosText(text.text(), text.length(), pos, paint);
            }
        } break;
        case DRAW_POS_TEXT_H: {
            const SkPaint& paint = *fPictureData->getPaint(reader);
            TextContainer text;
            get_text(reader, &text);
            size_t xCount = reader->readInt();
            const SkScalar constY = reader->readScalar();
            const SkScalar* xpos = (const SkScalar*)reader->skip(xCount * sizeof(SkScalar));
            canvas->drawPosTextH(text.text(), text.length(), xpos, constY, paint);
        } break;
        case DRAW_POS_TEXT_H_TOP_BOTTOM: {
            const SkPaint& paint = *fPictureData->getPaint(reader);
            TextContainer text;
            get_text(reader, &text);
            size_t xCount = reader->readInt();
            const SkScalar* xpos = (const SkScalar*)reader->skip((3 + xCount) * sizeof(SkScalar));
            const SkScalar top = *xpos++;
            const SkScalar bottom = *xpos++;
            const SkScalar constY = *xpos++;
            if (!canvas->quickRejectY(top, bottom)) {
                canvas->drawPosTextH(text.text(), text.length(), xpos, constY, paint);
            }
        } break;
        case DRAW_RECT: {
            const SkPaint& paint = *fPictureData->getPaint(reader);
            canvas->drawRect(reader->skipT<SkRect>(), paint);
        } break;
        case DRAW_RRECT: {
            const SkPaint& paint = *fPictureData->getPaint(reader);
            SkRRect rrect;
            reader->readRRect(&rrect);
            canvas->drawRRect(rrect, paint);
        } break;
        case DRAW_SPRITE: {
            const SkPaint* paint = fPictureData->getPaint(reader);
            const SkBitmap bitmap = shallow_copy(fPictureData->getBitmap(reader));
            int left = reader->readInt();
            int top = reader->readInt();
            canvas->drawSprite(bitmap, left, top, paint);
        } break;
        case DRAW_TEXT: {
            const SkPaint& paint = *fPictureData->getPaint(reader);
            TextContainer text;
            get_text(reader, &text);
            SkScalar x = reader->readScalar();
            SkScalar y = reader->readScalar();
            canvas->drawText(text.text(), text.length(), x, y, paint);
        } break;
        case DRAW_TEXT_TOP_BOTTOM: {
            const SkPaint& paint = *fPictureData->getPaint(reader);
            TextContainer text;
            get_text(reader, &text);
            const SkScalar* ptr = (const SkScalar*)reader->skip(4 * sizeof(SkScalar));
            // ptr[0] == x
            // ptr[1] == y
            // ptr[2] == top
            // ptr[3] == bottom
            if (!canvas->quickRejectY(ptr[2], ptr[3])) {
                canvas->drawText(text.text(), text.length(), ptr[0], ptr[1], paint);
            }
        } break;
        case DRAW_TEXT_ON_PATH: {
            const SkPaint& paint = *fPictureData->getPaint(reader);
            TextContainer text;
            get_text(reader, &text);
            const SkPath& path = fPictureData->getPath(reader);
            SkMatrix matrix;
            reader->readMatrix(&matrix);
            canvas->drawTextOnPath(text.text(), text.length(), path, &matrix, paint);
        } break;
        case DRAW_VERTICES: {
            SkAutoTUnref<SkXfermode> xfer;
            const SkPaint& paint = *fPictureData->getPaint(reader);
            DrawVertexFlags flags = (DrawVertexFlags)reader->readInt();
            SkCanvas::VertexMode vmode = (SkCanvas::VertexMode)reader->readInt();
            int vCount = reader->readInt();
            const SkPoint* verts = (const SkPoint*)reader->skip(vCount * sizeof(SkPoint));
            const SkPoint* texs = NULL;
            const SkColor* colors = NULL;
            const uint16_t* indices = NULL;
            int iCount = 0;
            if (flags & DRAW_VERTICES_HAS_TEXS) {
                texs = (const SkPoint*)reader->skip(vCount * sizeof(SkPoint));
            }
            if (flags & DRAW_VERTICES_HAS_COLORS) {
                colors = (const SkColor*)reader->skip(vCount * sizeof(SkColor));
            }
            if (flags & DRAW_VERTICES_HAS_INDICES) {
                iCount = reader->readInt();
                indices = (const uint16_t*)reader->skip(iCount * sizeof(uint16_t));
            }
            if (flags & DRAW_VERTICES_HAS_XFER) {
                int mode = reader->readInt();
                if (mode < 0 || mode > SkXfermode::kLastMode) {
                    mode = SkXfermode::kModulate_Mode;
                }
                xfer.reset(SkXfermode::Create((SkXfermode::Mode)mode));
            }
            canvas->drawVertices(vmode, vCount, verts, texs, colors, xfer, indices, iCount, paint);
        } break;
        case RESTORE:
            canvas->restore();
            break;
        case ROTATE:
            canvas->rotate(reader->readScalar());
            break;
        case SAVE:
            // SKPs with version < 29 also store a SaveFlags param.
            if (size > 4) {
                SkASSERT(8 == size);
                reader->readInt();
            }
            canvas->save();
            break;
        case SAVE_LAYER: {
            const SkRect* boundsPtr = get_rect_ptr(reader);
            const SkPaint* paint = fPictureData->getPaint(reader);
            canvas->saveLayer(boundsPtr, paint, (SkCanvas::SaveFlags) reader->readInt());
        } break;
        case SCALE: {
            SkScalar sx = reader->readScalar();
            SkScalar sy = reader->readScalar();
            canvas->scale(sx, sy);
        } break;
        case SET_MATRIX: {
            SkMatrix matrix;
            reader->readMatrix(&matrix);
            matrix.postConcat(initialMatrix);
            canvas->setMatrix(matrix);
        } break;
        case SKEW: {
            SkScalar sx = reader->readScalar();
            SkScalar sy = reader->readScalar();
            canvas->skew(sx, sy);
        } break;
        case TRANSLATE: {
            SkScalar dx = reader->readScalar();
            SkScalar dy = reader->readScalar();
            canvas->translate(dx, dy);
        } break;
        default:
            SkASSERT(0);
    }
}

