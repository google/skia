/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkPatchUtils.h"
#include "SkPictureData.h"
#include "SkPicturePlayback.h"
#include "SkPictureRecord.h"
#include "SkReadBuffer.h"
#include "SkRSXform.h"
#include "SkTextBlob.h"
#include "SkTDArray.h"
#include "SkTypes.h"

// matches old SkCanvas::SaveFlags
enum LegacySaveFlags {
    kHasAlphaLayer_LegacySaveFlags    = 0x04,
    kClipToLayer_LegacySaveFlags      = 0x10,
};

SkCanvas::SaveLayerFlags SkCanvas::LegacySaveFlagsToSaveLayerFlags(uint32_t flags) {
    uint32_t layerFlags = 0;

    if (0 == (flags & kClipToLayer_LegacySaveFlags)) {
        layerFlags |= SkCanvas::kDontClipToLayer_PrivateSaveLayerFlag;
    }
    if (0 == (flags & kHasAlphaLayer_LegacySaveFlags)) {
        layerFlags |= kIsOpaque_SaveLayerFlag;
    }
    return layerFlags;
}

/*
 * Read the next op code and chunk size from 'reader'. The returned size
 * is the entire size of the chunk (including the opcode). Thus, the
 * offset just prior to calling ReadOpAndSize + 'size' is the offset
 * to the next chunk's op code. This also means that the size of a chunk
 * with no arguments (just an opcode) will be 4.
 */
DrawType SkPicturePlayback::ReadOpAndSize(SkReadBuffer* reader, uint32_t* size) {
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


static const SkRect* get_rect_ptr(SkReadBuffer* reader, SkRect* storage) {
    if (reader->readBool()) {
        reader->readRect(storage);
        return storage;
    } else {
        return nullptr;
    }
}

class TextContainer {
public:
    size_t length() { return fByteLength; }
    const void* text() { return (const void*)fText; }
    size_t fByteLength;
    const char* fText;
};

void get_text(SkReadBuffer* reader, TextContainer* text) {
    size_t length = text->fByteLength = reader->readInt();
    text->fText = (const char*)reader->skip(length);
}

// FIXME: SkBitmaps are stateful, so we need to copy them to play back in multiple threads.
static SkBitmap shallow_copy(const SkBitmap& bitmap) {
    return bitmap;
}

void SkPicturePlayback::draw(SkCanvas* canvas,
                             SkPicture::AbortCallback* callback,
                             const SkReadBuffer* buffer) {
    AutoResetOpID aroi(this);
    SkASSERT(0 == fCurOffset);

    SkAutoTDelete<SkReadBuffer> reader;
    if (buffer) {
        reader.reset(buffer->clone(fPictureData->opData()->bytes(),
                                   fPictureData->opData()->size()));
    } else {
        reader.reset(new SkReadBuffer(fPictureData->opData()->bytes(),
                                      fPictureData->opData()->size()));
    }

    // Record this, so we can concat w/ it if we encounter a setMatrix()
    SkMatrix initialMatrix = canvas->getTotalMatrix();

    SkAutoCanvasRestore acr(canvas, false);

    while (!reader->eof()) {
        if (callback && callback->abort()) {
            return;
        }

        fCurOffset = reader->offset();
        uint32_t size;
        DrawType op = ReadOpAndSize(reader, &size);

        this->handleOp(reader, op, size, canvas, initialMatrix);
    }
}

void SkPicturePlayback::handleOp(SkReadBuffer* reader,
                                 DrawType op,
                                 uint32_t size,
                                 SkCanvas* canvas,
                                 const SkMatrix& initialMatrix) {
    switch (op) {
        case NOOP: {
            SkASSERT(size >= 4);
            reader->skip(size - 4);
        } break;
        case CLIP_PATH: {
            const SkPath& path = fPictureData->getPath(reader);
            uint32_t packed = reader->readInt();
            SkRegion::Op regionOp = ClipParams_unpackRegionOp(packed);
            bool doAA = ClipParams_unpackDoAA(packed);
            size_t offsetToRestore = reader->readInt();
            SkASSERT(!offsetToRestore || offsetToRestore >= reader->offset());
            canvas->clipPath(path, regionOp, doAA);
            if (canvas->isClipEmpty() && offsetToRestore) {
                reader->skip(offsetToRestore - reader->offset());
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
                reader->skip(offsetToRestore - reader->offset());
            }
        } break;
        case CLIP_RECT: {
            SkRect rect;
            reader->readRect(&rect);
            uint32_t packed = reader->readInt();
            SkRegion::Op regionOp = ClipParams_unpackRegionOp(packed);
            bool doAA = ClipParams_unpackDoAA(packed);
            size_t offsetToRestore = reader->readInt();
            SkASSERT(!offsetToRestore || offsetToRestore >= reader->offset());
            canvas->clipRect(rect, regionOp, doAA);
            if (canvas->isClipEmpty() && offsetToRestore) {
                reader->skip(offsetToRestore - reader->offset());
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
                reader->skip(offsetToRestore - reader->offset());
            }
        } break;
        case PUSH_CULL: break;  // Deprecated, safe to ignore both push and pop.
        case POP_CULL:  break;
        case CONCAT: {
            SkMatrix matrix;
            reader->readMatrix(&matrix);
            canvas->concat(matrix);
            break;
        }
        case DRAW_ANNOTATION: {
            SkRect rect;
            reader->readRect(&rect);
            SkString key;
            reader->readString(&key);
            canvas->drawAnnotation(rect, key.c_str(), reader->readByteArrayAsData().get());
        } break;
        case DRAW_ATLAS: {
            const SkPaint* paint = fPictureData->getPaint(reader);
            const SkImage* atlas = fPictureData->getImage(reader);
            const uint32_t flags = reader->readUInt();
            const int count = reader->readUInt();
            const SkRSXform* xform = (const SkRSXform*)reader->skip(count * sizeof(SkRSXform));
            const SkRect* tex = (const SkRect*)reader->skip(count * sizeof(SkRect));
            const SkColor* colors = nullptr;
            SkXfermode::Mode mode = SkXfermode::kDst_Mode;
            if (flags & DRAW_ATLAS_HAS_COLORS) {
                colors = (const SkColor*)reader->skip(count * sizeof(SkColor));
                mode = (SkXfermode::Mode)reader->readUInt();
            }
            const SkRect* cull = nullptr;
            if (flags & DRAW_ATLAS_HAS_CULL) {
                cull = (const SkRect*)reader->skip(sizeof(SkRect));
            }
            canvas->drawAtlas(atlas, xform, tex, colors, count, mode, cull, paint);
        } break;
        case DRAW_BITMAP: {
            const SkPaint* paint = fPictureData->getPaint(reader);
            const SkBitmap bitmap = shallow_copy(fPictureData->getBitmap(reader));
            SkPoint loc;
            reader->readPoint(&loc);
            canvas->drawBitmap(bitmap, loc.fX, loc.fY, paint);
        } break;
        case DRAW_BITMAP_RECT: {
            const SkPaint* paint = fPictureData->getPaint(reader);
            const SkBitmap bitmap = shallow_copy(fPictureData->getBitmap(reader));
            SkRect storage;
            const SkRect* src = get_rect_ptr(reader, &storage);   // may be null
            SkRect dst;
            reader->readRect(&dst);     // required
            SkCanvas::SrcRectConstraint constraint = (SkCanvas::SrcRectConstraint)reader->readInt();
            canvas->legacy_drawBitmapRect(bitmap, src, dst, paint, constraint);
        } break;
        case DRAW_BITMAP_MATRIX: {
            const SkPaint* paint = fPictureData->getPaint(reader);
            const SkBitmap bitmap = shallow_copy(fPictureData->getBitmap(reader));
            SkMatrix matrix;
            reader->readMatrix(&matrix);

            SkAutoCanvasRestore acr(canvas, true);
            canvas->concat(matrix);
            canvas->drawBitmap(bitmap, 0, 0, paint);
        } break;
        case DRAW_BITMAP_NINE: {
            const SkPaint* paint = fPictureData->getPaint(reader);
            const SkBitmap bitmap = shallow_copy(fPictureData->getBitmap(reader));
            SkIRect src;
            reader->readIRect(&src);
            SkRect dst;
            reader->readRect(&dst);
            canvas->drawBitmapNine(bitmap, src, dst, paint);
        } break;
        case DRAW_CLEAR:
            canvas->clear(reader->readInt());
            break;
        case DRAW_DATA: {
            // This opcode is now dead, just need to skip it for backwards compatibility
            size_t length = reader->readInt();
            (void)reader->skip(length);
            // skip handles padding the read out to a multiple of 4
        } break;
        case DRAW_DRAWABLE:
            canvas->drawDrawable(fPictureData->getDrawable(reader));
            break;
        case DRAW_DRAWABLE_MATRIX: {
            SkMatrix matrix;
            reader->readMatrix(&matrix);
            SkDrawable* drawable = fPictureData->getDrawable(reader);
            canvas->drawDrawable(drawable, &matrix);
        } break;
        case DRAW_DRRECT: {
            const SkPaint* paint = fPictureData->getPaint(reader);
            SkRRect outer, inner;
            reader->readRRect(&outer);
            reader->readRRect(&inner);
            if (paint) {
                canvas->drawDRRect(outer, inner, *paint);
            }
        } break;
        case BEGIN_COMMENT_GROUP: {
            SkString tmp;
            reader->readString(&tmp);
            // deprecated (M44)
            break;
        }
        case COMMENT: {
            SkString tmp;
            reader->readString(&tmp);
            reader->readString(&tmp);
            // deprecated (M44)
            break;
        }
        case END_COMMENT_GROUP:
            // deprecated (M44)
            break;
        case DRAW_IMAGE: {
            const SkPaint* paint = fPictureData->getPaint(reader);
            const SkImage* image = fPictureData->getImage(reader);
            SkPoint loc;
            reader->readPoint(&loc);
            canvas->drawImage(image, loc.fX, loc.fY, paint);
        } break;
        case DRAW_IMAGE_NINE: {
            const SkPaint* paint = fPictureData->getPaint(reader);
            const SkImage* image = fPictureData->getImage(reader);
            SkIRect center;
            reader->readIRect(&center);
            SkRect dst;
            reader->readRect(&dst);
            canvas->drawImageNine(image, center, dst, paint);
        } break;
        case DRAW_IMAGE_RECT_STRICT:
        case DRAW_IMAGE_RECT: {
            const SkPaint* paint = fPictureData->getPaint(reader);
            const SkImage* image = fPictureData->getImage(reader);
            SkRect storage;
            const SkRect* src = get_rect_ptr(reader, &storage);   // may be null
            SkRect dst;
            reader->readRect(&dst);     // required
            // DRAW_IMAGE_RECT_STRICT assumes this constraint, and doesn't store it
            SkCanvas::SrcRectConstraint constraint = SkCanvas::kStrict_SrcRectConstraint;
            if (DRAW_IMAGE_RECT == op) {
                // newer op-code stores the constraint explicitly
                constraint = (SkCanvas::SrcRectConstraint)reader->readInt();
            }
            canvas->legacy_drawImageRect(image, src, dst, paint, constraint);
        } break;
        case DRAW_OVAL: {
            const SkPaint* paint = fPictureData->getPaint(reader);
            SkRect rect;
            reader->readRect(&rect);
            if (paint) {
                canvas->drawOval(rect, *paint);
            }
        } break;
        case DRAW_PAINT: {
            const SkPaint* paint = fPictureData->getPaint(reader);
            if (paint) {
                canvas->drawPaint(*paint);
            }
        } break;
        case DRAW_PATCH: {
            const SkPaint* paint = fPictureData->getPaint(reader);

            const SkPoint* cubics = (const SkPoint*)reader->skip(SkPatchUtils::kNumCtrlPts *
                                                                 sizeof(SkPoint));
            uint32_t flag = reader->readInt();
            const SkColor* colors = nullptr;
            if (flag & DRAW_VERTICES_HAS_COLORS) {
                colors = (const SkColor*)reader->skip(SkPatchUtils::kNumCorners * sizeof(SkColor));
            }
            const SkPoint* texCoords = nullptr;
            if (flag & DRAW_VERTICES_HAS_TEXS) {
                texCoords = (const SkPoint*)reader->skip(SkPatchUtils::kNumCorners *
                                                         sizeof(SkPoint));
            }
            sk_sp<SkXfermode> xfer;
            if (flag & DRAW_VERTICES_HAS_XFER) {
                int mode = reader->readInt();
                if (mode < 0 || mode > SkXfermode::kLastMode) {
                    mode = SkXfermode::kModulate_Mode;
                }
                xfer = SkXfermode::Make((SkXfermode::Mode)mode);
            }
            if (paint) {
                canvas->drawPatch(cubics, colors, texCoords, std::move(xfer), *paint);
            }
        } break;
        case DRAW_PATH: {
            const SkPaint* paint = fPictureData->getPaint(reader);
            if (paint) {
                canvas->drawPath(fPictureData->getPath(reader), *paint);
            }
        } break;
        case DRAW_PICTURE:
            canvas->drawPicture(fPictureData->getPicture(reader));
            break;
        case DRAW_PICTURE_MATRIX_PAINT: {
            const SkPaint* paint = fPictureData->getPaint(reader);
            SkMatrix matrix;
            reader->readMatrix(&matrix);
            const SkPicture* pic = fPictureData->getPicture(reader);
            canvas->drawPicture(pic, &matrix, paint);
        } break;
        case DRAW_POINTS: {
            const SkPaint* paint = fPictureData->getPaint(reader);
            SkCanvas::PointMode mode = (SkCanvas::PointMode)reader->readInt();
            size_t count = reader->readInt();
            const SkPoint* pts = (const SkPoint*)reader->skip(sizeof(SkPoint)* count);
            if (paint) {
                canvas->drawPoints(mode, count, pts, *paint);
            }
        } break;
        case DRAW_POS_TEXT: {
            const SkPaint* paint = fPictureData->getPaint(reader);
            TextContainer text;
            get_text(reader, &text);
            size_t points = reader->readInt();
            const SkPoint* pos = (const SkPoint*)reader->skip(points * sizeof(SkPoint));
            if (paint) {
                canvas->drawPosText(text.text(), text.length(), pos, *paint);
            }
        } break;
        case DRAW_POS_TEXT_TOP_BOTTOM: {
            const SkPaint* paint = fPictureData->getPaint(reader);
            TextContainer text;
            get_text(reader, &text);
            size_t points = reader->readInt();
            const SkPoint* pos = (const SkPoint*)reader->skip(points * sizeof(SkPoint));
            const SkScalar top = reader->readScalar();
            const SkScalar bottom = reader->readScalar();
            if (!canvas->quickRejectY(top, bottom) && paint) {
                canvas->drawPosText(text.text(), text.length(), pos, *paint);
            }
        } break;
        case DRAW_POS_TEXT_H: {
            const SkPaint* paint = fPictureData->getPaint(reader);
            TextContainer text;
            get_text(reader, &text);
            size_t xCount = reader->readInt();
            const SkScalar constY = reader->readScalar();
            const SkScalar* xpos = (const SkScalar*)reader->skip(xCount * sizeof(SkScalar));
            if (paint) {
                canvas->drawPosTextH(text.text(), text.length(), xpos, constY, *paint);
            }
        } break;
        case DRAW_POS_TEXT_H_TOP_BOTTOM: {
            const SkPaint* paint = fPictureData->getPaint(reader);
            TextContainer text;
            get_text(reader, &text);
            size_t xCount = reader->readInt();
            const SkScalar* xpos = (const SkScalar*)reader->skip((3 + xCount) * sizeof(SkScalar));
            const SkScalar top = *xpos++;
            const SkScalar bottom = *xpos++;
            const SkScalar constY = *xpos++;
            if (!canvas->quickRejectY(top, bottom) && paint) {
                canvas->drawPosTextH(text.text(), text.length(), xpos, constY, *paint);
            }
        } break;
        case DRAW_RECT: {
            const SkPaint* paint = fPictureData->getPaint(reader);
            SkRect rect;
            reader->readRect(&rect);
            if (paint) {
                canvas->drawRect(rect, *paint);
            }
        } break;
        case DRAW_RRECT: {
            const SkPaint* paint = fPictureData->getPaint(reader);
            SkRRect rrect;
            reader->readRRect(&rrect);
            if (paint) {
                canvas->drawRRect(rrect, *paint);
            }
        } break;
        case DRAW_SPRITE: {
            /* const SkPaint* paint = */ fPictureData->getPaint(reader);
            /* const SkBitmap bitmap = */ shallow_copy(fPictureData->getBitmap(reader));
            /* int left = */ reader->readInt();
            /* int top = */ reader->readInt();
            // drawSprite removed dec-2015
        } break;
        case DRAW_TEXT: {
            const SkPaint* paint = fPictureData->getPaint(reader);
            TextContainer text;
            get_text(reader, &text);
            SkScalar x = reader->readScalar();
            SkScalar y = reader->readScalar();
            if (paint) {
                canvas->drawText(text.text(), text.length(), x, y, *paint);
            }
        } break;
        case DRAW_TEXT_BLOB: {
            const SkPaint* paint = fPictureData->getPaint(reader);
            const SkTextBlob* blob = fPictureData->getTextBlob(reader);
            SkScalar x = reader->readScalar();
            SkScalar y = reader->readScalar();
            if (paint) {
                canvas->drawTextBlob(blob, x, y, *paint);
            }
        } break;
        case DRAW_TEXT_TOP_BOTTOM: {
            const SkPaint* paint = fPictureData->getPaint(reader);
            TextContainer text;
            get_text(reader, &text);
            const SkScalar* ptr = (const SkScalar*)reader->skip(4 * sizeof(SkScalar));
            // ptr[0] == x
            // ptr[1] == y
            // ptr[2] == top
            // ptr[3] == bottom
            if (!canvas->quickRejectY(ptr[2], ptr[3]) && paint) {
                canvas->drawText(text.text(), text.length(), ptr[0], ptr[1], *paint);
            }
        } break;
        case DRAW_TEXT_ON_PATH: {
            const SkPaint* paint = fPictureData->getPaint(reader);
            TextContainer text;
            get_text(reader, &text);
            const SkPath& path = fPictureData->getPath(reader);
            SkMatrix matrix;
            reader->readMatrix(&matrix);
            if (paint) {
                canvas->drawTextOnPath(text.text(), text.length(), path, &matrix, *paint);
            }
        } break;
        case DRAW_VERTICES: {
            sk_sp<SkXfermode> xfer;
            const SkPaint* paint = fPictureData->getPaint(reader);
            DrawVertexFlags flags = (DrawVertexFlags)reader->readInt();
            SkCanvas::VertexMode vmode = (SkCanvas::VertexMode)reader->readInt();
            int vCount = reader->readInt();
            const SkPoint* verts = (const SkPoint*)reader->skip(vCount * sizeof(SkPoint));
            const SkPoint* texs = nullptr;
            const SkColor* colors = nullptr;
            const uint16_t* indices = nullptr;
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
                xfer = SkXfermode::Make((SkXfermode::Mode)mode);
            }
            if (paint) {
                canvas->drawVertices(vmode, vCount, verts, texs, colors,
                                     xfer, indices, iCount, *paint);
            }
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
                if (reader->validate(8 == size)) {
                    reader->readInt();
                }
            }
            canvas->save();
            break;
        case SAVE_LAYER_SAVEFLAGS_DEPRECATED: {
            SkRect storage;
            const SkRect* boundsPtr = get_rect_ptr(reader, &storage);
            const SkPaint* paint = fPictureData->getPaint(reader);
            auto flags = SkCanvas::LegacySaveFlagsToSaveLayerFlags(reader->readInt());
            canvas->saveLayer(SkCanvas::SaveLayerRec(boundsPtr, paint, flags));
        } break;
        case SAVE_LAYER_SAVELAYERFLAGS_DEPRECATED_JAN_2016: {
            SkRect storage;
            const SkRect* boundsPtr = get_rect_ptr(reader, &storage);
            const SkPaint* paint = fPictureData->getPaint(reader);
            canvas->saveLayer(SkCanvas::SaveLayerRec(boundsPtr, paint, reader->readInt()));
        } break;
        case SAVE_LAYER_SAVELAYERREC: {
            SkCanvas::SaveLayerRec rec(nullptr, nullptr, nullptr, 0);
            const uint32_t flatFlags = reader->readInt();
            SkRect bounds;
            if (flatFlags & SAVELAYERREC_HAS_BOUNDS) {
                reader->readRect(&bounds);
                rec.fBounds = &bounds;
            }
            if (flatFlags & SAVELAYERREC_HAS_PAINT) {
                rec.fPaint = fPictureData->getPaint(reader);
            }
            if (flatFlags & SAVELAYERREC_HAS_BACKDROP) {
                const SkPaint* paint = fPictureData->getPaint(reader);
                rec.fBackdrop = paint->getImageFilter();
            }
            if (flatFlags & SAVELAYERREC_HAS_FLAGS) {
                rec.fSaveLayerFlags = reader->readInt();
            }
            canvas->saveLayer(rec);
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
            SkASSERTF(false, "Unknown draw type: %d", op);
    }
}
