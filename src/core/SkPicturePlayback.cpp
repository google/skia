/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkRSXform.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTypes.h"
#include "include/private/SkTDArray.h"
#include "src/core/SkCanvasPriv.h"
#include "src/core/SkDrawShadowInfo.h"
#include "src/core/SkFontPriv.h"
#include "src/core/SkPaintPriv.h"
#include "src/core/SkPictureData.h"
#include "src/core/SkPicturePlayback.h"
#include "src/core/SkPictureRecord.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkSafeMath.h"
#include "src/utils/SkPatchUtils.h"

// matches old SkCanvas::SaveFlags
enum LegacySaveFlags {
    kClipToLayer_LegacySaveFlags      = 0x10,
};

SkCanvas::SaveLayerFlags SkCanvasPriv::LegacySaveFlagsToSaveLayerFlags(uint32_t flags) {
    uint32_t layerFlags = 0;

    if (0 == (flags & kClipToLayer_LegacySaveFlags)) {
        layerFlags |= kDontClipToLayer_SaveLayerFlag;
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
    if ((temp & 0xFF) == temp) {
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

void SkPicturePlayback::draw(SkCanvas* canvas,
                             SkPicture::AbortCallback* callback,
                             SkReadBuffer* buffer) {
    AutoResetOpID aroi(this);
    SkASSERT(0 == fCurOffset);

    SkReadBuffer reader(fPictureData->opData()->bytes(),
                        fPictureData->opData()->size());

    // Record this, so we can concat w/ it if we encounter a setMatrix()
    SkMatrix initialMatrix = canvas->getTotalMatrix();

    SkAutoCanvasRestore acr(canvas, false);

    while (!reader.eof()) {
        if (callback && callback->abort()) {
            return;
        }

        fCurOffset = reader.offset();
        uint32_t size;
        DrawType op = ReadOpAndSize(&reader, &size);
        if (!reader.validate(op > UNUSED && op <= LAST_DRAWTYPE_ENUM)) {
            return;
        }

        this->handleOp(&reader, op, size, canvas, initialMatrix);
    }

    // need to propagate invalid state to the parent reader
    if (buffer) {
        buffer->validate(reader.isValid());
    }
}

static void validate_offsetToRestore(SkReadBuffer* reader, size_t offsetToRestore) {
    if (offsetToRestore) {
        reader->validate(SkIsAlign4(offsetToRestore) && offsetToRestore >= reader->offset());
    }
}

void SkPicturePlayback::handleOp(SkReadBuffer* reader,
                                 DrawType op,
                                 uint32_t size,
                                 SkCanvas* canvas,
                                 const SkMatrix& initialMatrix) {
#define BREAK_ON_READ_ERROR(r) if (!r->isValid()) break

    switch (op) {
        case NOOP: {
            SkASSERT(size >= 4);
            reader->skip(size - 4);
        } break;
        case FLUSH:
            canvas->flush();
            break;
        case CLIP_PATH: {
            const SkPath& path = fPictureData->getPath(reader);
            uint32_t packed = reader->readInt();
            SkClipOp clipOp = ClipParams_unpackRegionOp(reader, packed);
            bool doAA = ClipParams_unpackDoAA(packed);
            size_t offsetToRestore = reader->readInt();
            validate_offsetToRestore(reader, offsetToRestore);
            BREAK_ON_READ_ERROR(reader);

            canvas->clipPath(path, clipOp, doAA);
            if (canvas->isClipEmpty() && offsetToRestore) {
                reader->skip(offsetToRestore - reader->offset());
            }
        } break;
        case CLIP_REGION: {
            SkRegion region;
            reader->readRegion(&region);
            uint32_t packed = reader->readInt();
            SkClipOp clipOp = ClipParams_unpackRegionOp(reader, packed);
            size_t offsetToRestore = reader->readInt();
            validate_offsetToRestore(reader, offsetToRestore);
            BREAK_ON_READ_ERROR(reader);

            canvas->clipRegion(region, clipOp);
            if (canvas->isClipEmpty() && offsetToRestore) {
                reader->skip(offsetToRestore - reader->offset());
            }
        } break;
        case CLIP_RECT: {
            SkRect rect;
            reader->readRect(&rect);
            uint32_t packed = reader->readInt();
            SkClipOp clipOp = ClipParams_unpackRegionOp(reader, packed);
            bool doAA = ClipParams_unpackDoAA(packed);
            size_t offsetToRestore = reader->readInt();
            validate_offsetToRestore(reader, offsetToRestore);
            BREAK_ON_READ_ERROR(reader);

            canvas->clipRect(rect, clipOp, doAA);
            if (canvas->isClipEmpty() && offsetToRestore) {
                reader->skip(offsetToRestore - reader->offset());
            }
        } break;
        case CLIP_RRECT: {
            SkRRect rrect;
            reader->readRRect(&rrect);
            uint32_t packed = reader->readInt();
            SkClipOp clipOp = ClipParams_unpackRegionOp(reader, packed);
            bool doAA = ClipParams_unpackDoAA(packed);
            size_t offsetToRestore = reader->readInt();
            validate_offsetToRestore(reader, offsetToRestore);
            BREAK_ON_READ_ERROR(reader);

            canvas->clipRRect(rrect, clipOp, doAA);
            if (canvas->isClipEmpty() && offsetToRestore) {
                reader->skip(offsetToRestore - reader->offset());
            }
        } break;
        case PUSH_CULL: break;  // Deprecated, safe to ignore both push and pop.
        case POP_CULL:  break;
        case CONCAT: {
            SkMatrix matrix;
            reader->readMatrix(&matrix);
            BREAK_ON_READ_ERROR(reader);

            canvas->concat(matrix);
            break;
        }
        case DRAW_ANNOTATION: {
            SkRect rect;
            reader->readRect(&rect);
            SkString key;
            reader->readString(&key);
            sk_sp<SkData> data = reader->readByteArrayAsData();
            BREAK_ON_READ_ERROR(reader);
            SkASSERT(data);

            canvas->drawAnnotation(rect, key.c_str(), data.get());
        } break;
        case DRAW_ARC: {
            const SkPaint* paint = fPictureData->getPaint(reader);
            SkRect rect;
            reader->readRect(&rect);
            SkScalar startAngle = reader->readScalar();
            SkScalar sweepAngle = reader->readScalar();
            int useCenter = reader->readInt();
            BREAK_ON_READ_ERROR(reader);

            if (paint) {
                canvas->drawArc(rect, startAngle, sweepAngle, SkToBool(useCenter), *paint);
            }
        } break;
        case DRAW_ATLAS: {
            const SkPaint* paint = fPictureData->getPaint(reader);
            const SkImage* atlas = fPictureData->getImage(reader);
            const uint32_t flags = reader->readUInt();
            const int count = reader->readUInt();
            const SkRSXform* xform = (const SkRSXform*)reader->skip(count, sizeof(SkRSXform));
            const SkRect* tex = (const SkRect*)reader->skip(count, sizeof(SkRect));
            const SkColor* colors = nullptr;
            SkBlendMode mode = SkBlendMode::kDst;
            if (flags & DRAW_ATLAS_HAS_COLORS) {
                colors = (const SkColor*)reader->skip(count, sizeof(SkColor));
                mode = (SkBlendMode)reader->readUInt();
            }
            const SkRect* cull = nullptr;
            if (flags & DRAW_ATLAS_HAS_CULL) {
                cull = (const SkRect*)reader->skip(sizeof(SkRect));
            }
            BREAK_ON_READ_ERROR(reader);

            canvas->drawAtlas(atlas, xform, tex, colors, count, mode, cull, paint);
        } break;
        case DRAW_CLEAR: {
            auto c = reader->readInt();
            BREAK_ON_READ_ERROR(reader);

            canvas->clear(c);
        } break;
        case DRAW_DATA: {
            // This opcode is now dead, just need to skip it for backwards compatibility
            size_t length = reader->readInt();
            (void)reader->skip(length);
            // skip handles padding the read out to a multiple of 4
        } break;
        case DRAW_DRAWABLE: {
            auto* d = fPictureData->getDrawable(reader);
            BREAK_ON_READ_ERROR(reader);

            canvas->drawDrawable(d);
        } break;
        case DRAW_DRAWABLE_MATRIX: {
            SkMatrix matrix;
            reader->readMatrix(&matrix);
            SkDrawable* drawable = fPictureData->getDrawable(reader);
            BREAK_ON_READ_ERROR(reader);

            canvas->drawDrawable(drawable, &matrix);
        } break;
        case DRAW_DRRECT: {
            const SkPaint* paint = fPictureData->getPaint(reader);
            SkRRect outer, inner;
            reader->readRRect(&outer);
            reader->readRRect(&inner);
            BREAK_ON_READ_ERROR(reader);

            if (paint) {
                canvas->drawDRRect(outer, inner, *paint);
            }
        } break;
        case DRAW_EDGEAA_QUAD: {
            SkRect rect;
            reader->readRect(&rect);
            SkCanvas::QuadAAFlags aaFlags = static_cast<SkCanvas::QuadAAFlags>(reader->read32());
            SkColor color = reader->read32();
            SkBlendMode blend = static_cast<SkBlendMode>(reader->read32());
            bool hasClip = reader->readInt();
            SkPoint* clip = nullptr;
            if (hasClip) {
                clip = (SkPoint*) reader->skip(4, sizeof(SkPoint));
            }
            BREAK_ON_READ_ERROR(reader);
            canvas->experimental_DrawEdgeAAQuad(rect, clip, aaFlags, color, blend);
        } break;
        case DRAW_EDGEAA_IMAGE_SET: {
            static const size_t kEntryReadSize =
                    4 * sizeof(uint32_t) + 2 * sizeof(SkRect) + sizeof(SkScalar);
            static const size_t kMatrixSize = 9 * sizeof(SkScalar); // != sizeof(SkMatrix)

            int cnt = reader->readInt();
            if (!reader->validate(cnt >= 0)) {
                break;
            }
            const SkPaint* paint = fPictureData->getPaint(reader);
            SkCanvas::SrcRectConstraint constraint =
                    static_cast<SkCanvas::SrcRectConstraint>(reader->readInt());

            if (!reader->validate(SkSafeMath::Mul(cnt, kEntryReadSize) <= reader->available())) {
                break;
            }

            // Track minimum necessary clip points and matrices that must be provided to satisfy
            // the entries.
            int expectedClips = 0;
            int maxMatrixIndex = -1;
            SkAutoTArray<SkCanvas::ImageSetEntry> set(cnt);
            for (int i = 0; i < cnt && reader->isValid(); ++i) {
                set[i].fImage = sk_ref_sp(fPictureData->getImage(reader));
                reader->readRect(&set[i].fSrcRect);
                reader->readRect(&set[i].fDstRect);
                set[i].fMatrixIndex = reader->readInt();
                set[i].fAlpha = reader->readScalar();
                set[i].fAAFlags = reader->readUInt();
                set[i].fHasClip = reader->readInt();

                expectedClips += set[i].fHasClip ? 1 : 0;
                if (set[i].fMatrixIndex > maxMatrixIndex) {
                    maxMatrixIndex = set[i].fMatrixIndex;
                }
            }

            int dstClipCount = reader->readInt();
            SkPoint* dstClips = nullptr;
            if (!reader->validate(expectedClips <= dstClipCount)) {
                // Entries request more dstClip points than are provided in the buffer
                break;
            } else if (dstClipCount > 0) {
                dstClips = (SkPoint*) reader->skip(dstClipCount, sizeof(SkPoint));
                if (dstClips == nullptr) {
                    // Not enough bytes remaining so the reader has been invalidated
                    break;
                }
            }
            int matrixCount = reader->readInt();
            if (!reader->validate((maxMatrixIndex + 1) <= matrixCount) ||
                !reader->validate(
                    SkSafeMath::Mul(matrixCount, kMatrixSize) <= reader->available())) {
                // Entries access out-of-bound matrix indices, given provided matrices or
                // there aren't enough bytes to provide that many matrices
                break;
            }
            SkTArray<SkMatrix> matrices(matrixCount);
            for (int i = 0; i < matrixCount && reader->isValid(); ++i) {
                reader->readMatrix(&matrices.push_back());
            }
            BREAK_ON_READ_ERROR(reader);

            canvas->experimental_DrawEdgeAAImageSet(set.get(), cnt, dstClips, matrices.begin(),
                                                    paint, constraint);
        } break;
        case DRAW_IMAGE: {
            const SkPaint* paint = fPictureData->getPaint(reader);
            const SkImage* image = fPictureData->getImage(reader);
            SkPoint loc;
            reader->readPoint(&loc);
            BREAK_ON_READ_ERROR(reader);

            canvas->drawImage(image, loc.fX, loc.fY, paint);
        } break;
        case DRAW_IMAGE_LATTICE: {
            const SkPaint* paint = fPictureData->getPaint(reader);
            const SkImage* image = fPictureData->getImage(reader);
            SkCanvas::Lattice lattice;
            (void)SkCanvasPriv::ReadLattice(*reader, &lattice);
            const SkRect* dst = reader->skipT<SkRect>();
            BREAK_ON_READ_ERROR(reader);

            canvas->drawImageLattice(image, lattice, *dst, paint);
        } break;
        case DRAW_IMAGE_NINE: {
            const SkPaint* paint = fPictureData->getPaint(reader);
            const SkImage* image = fPictureData->getImage(reader);
            SkIRect center;
            reader->readIRect(&center);
            SkRect dst;
            reader->readRect(&dst);
            BREAK_ON_READ_ERROR(reader);

            canvas->drawImageNine(image, center, dst, paint);
        } break;
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
            BREAK_ON_READ_ERROR(reader);

            canvas->legacy_drawImageRect(image, src, dst, paint, constraint);
        } break;
        case DRAW_OVAL: {
            const SkPaint* paint = fPictureData->getPaint(reader);
            SkRect rect;
            reader->readRect(&rect);
            BREAK_ON_READ_ERROR(reader);

            if (paint) {
                canvas->drawOval(rect, *paint);
            }
        } break;
        case DRAW_PAINT: {
            const SkPaint* paint = fPictureData->getPaint(reader);
            BREAK_ON_READ_ERROR(reader);

            if (paint) {
                canvas->drawPaint(*paint);
            }
        } break;
        case DRAW_BEHIND_PAINT: {
            const SkPaint* paint = fPictureData->getPaint(reader);
            BREAK_ON_READ_ERROR(reader);

            if (paint) {
                SkCanvasPriv::DrawBehind(canvas, *paint);
            }
        } break;
        case DRAW_PATCH: {
            const SkPaint* paint = fPictureData->getPaint(reader);

            const SkPoint* cubics = (const SkPoint*)reader->skip(SkPatchUtils::kNumCtrlPts,
                                                                 sizeof(SkPoint));
            uint32_t flag = reader->readInt();
            const SkColor* colors = nullptr;
            if (flag & DRAW_VERTICES_HAS_COLORS) {
                colors = (const SkColor*)reader->skip(SkPatchUtils::kNumCorners, sizeof(SkColor));
            }
            const SkPoint* texCoords = nullptr;
            if (flag & DRAW_VERTICES_HAS_TEXS) {
                texCoords = (const SkPoint*)reader->skip(SkPatchUtils::kNumCorners,
                                                         sizeof(SkPoint));
            }
            SkBlendMode bmode = SkBlendMode::kModulate;
            if (flag & DRAW_VERTICES_HAS_XFER) {
                unsigned mode = reader->readInt();
                if (mode <= (unsigned)SkBlendMode::kLastMode) {
                    bmode = (SkBlendMode)mode;
                }
            }
            BREAK_ON_READ_ERROR(reader);

            if (paint) {
                canvas->drawPatch(cubics, colors, texCoords, bmode, *paint);
            }
        } break;
        case DRAW_PATH: {
            const SkPaint* paint = fPictureData->getPaint(reader);
            const auto& path = fPictureData->getPath(reader);
            BREAK_ON_READ_ERROR(reader);

            if (paint) {
                canvas->drawPath(path, *paint);
            }
        } break;
        case DRAW_PICTURE: {
            const auto* pic = fPictureData->getPicture(reader);
            BREAK_ON_READ_ERROR(reader);

            canvas->drawPicture(pic);
        } break;
        case DRAW_PICTURE_MATRIX_PAINT: {
            const SkPaint* paint = fPictureData->getPaint(reader);
            SkMatrix matrix;
            reader->readMatrix(&matrix);
            const SkPicture* pic = fPictureData->getPicture(reader);
            BREAK_ON_READ_ERROR(reader);

            canvas->drawPicture(pic, &matrix, paint);
        } break;
        case DRAW_POINTS: {
            const SkPaint* paint = fPictureData->getPaint(reader);
            SkCanvas::PointMode mode = (SkCanvas::PointMode)reader->readInt();
            size_t count = reader->readInt();
            const SkPoint* pts = (const SkPoint*)reader->skip(count, sizeof(SkPoint));
            BREAK_ON_READ_ERROR(reader);

            if (paint) {
                canvas->drawPoints(mode, count, pts, *paint);
            }
        } break;
        case DRAW_RECT: {
            const SkPaint* paint = fPictureData->getPaint(reader);
            SkRect rect;
            reader->readRect(&rect);
            BREAK_ON_READ_ERROR(reader);

            if (paint) {
                canvas->drawRect(rect, *paint);
            }
        } break;
        case DRAW_REGION: {
            const SkPaint* paint = fPictureData->getPaint(reader);
            SkRegion region;
            reader->readRegion(&region);
            BREAK_ON_READ_ERROR(reader);

            if (paint) {
                canvas->drawRegion(region, *paint);
            }
        } break;
        case DRAW_RRECT: {
            const SkPaint* paint = fPictureData->getPaint(reader);
            SkRRect rrect;
            reader->readRRect(&rrect);
            BREAK_ON_READ_ERROR(reader);

            if (paint) {
                canvas->drawRRect(rrect, *paint);
            }
        } break;
        case DRAW_SHADOW_REC: {
            const auto& path = fPictureData->getPath(reader);
            SkDrawShadowRec rec;
            reader->readPoint3(&rec.fZPlaneParams);
            reader->readPoint3(&rec.fLightPos);
            rec.fLightRadius = reader->readScalar();
            if (reader->isVersionLT(SkReadBuffer::kTwoColorDrawShadow_Version)) {
                SkScalar ambientAlpha = reader->readScalar();
                SkScalar spotAlpha = reader->readScalar();
                SkColor color = reader->read32();
                rec.fAmbientColor = SkColorSetA(color, SkColorGetA(color)*ambientAlpha);
                rec.fSpotColor = SkColorSetA(color, SkColorGetA(color)*spotAlpha);
            } else {
                rec.fAmbientColor = reader->read32();
                rec.fSpotColor = reader->read32();
            }
            rec.fFlags = reader->read32();
            BREAK_ON_READ_ERROR(reader);

            canvas->private_draw_shadow_rec(path, rec);
        } break;
        case DRAW_TEXT_BLOB: {
            const SkPaint* paint = fPictureData->getPaint(reader);
            const SkTextBlob* blob = fPictureData->getTextBlob(reader);
            SkScalar x = reader->readScalar();
            SkScalar y = reader->readScalar();
            BREAK_ON_READ_ERROR(reader);

            if (paint) {
                canvas->drawTextBlob(blob, x, y, *paint);
            }
        } break;
        case DRAW_VERTICES_OBJECT: {
            const SkPaint* paint = fPictureData->getPaint(reader);
            const SkVertices* vertices = fPictureData->getVertices(reader);
            const int boneCount = reader->readInt();
            const SkVertices::Bone* bones = boneCount ?
                    (const SkVertices::Bone*) reader->skip(boneCount, sizeof(SkVertices::Bone)) :
                    nullptr;
            SkBlendMode bmode = reader->read32LE(SkBlendMode::kLastMode);
            BREAK_ON_READ_ERROR(reader);

            if (paint && vertices) {
                canvas->drawVertices(vertices, bones, boneCount, bmode, *paint);
            }
        } break;
        case RESTORE:
            canvas->restore();
            break;
        case ROTATE: {
            auto deg = reader->readScalar();
            canvas->rotate(deg);
        } break;
        case SAVE:
            canvas->save();
            break;
        case SAVE_BEHIND: {
            uint32_t flags = reader->readInt();
            const SkRect* subset = nullptr;
            SkRect storage;
            if (flags & SAVEBEHIND_HAS_SUBSET) {
                reader->readRect(&storage);
                subset = &storage;
            }
            SkCanvasPriv::SaveBehind(canvas, subset);
        } break;
        case SAVE_LAYER_SAVEFLAGS_DEPRECATED: {
            SkRect storage;
            const SkRect* boundsPtr = get_rect_ptr(reader, &storage);
            const SkPaint* paint = fPictureData->getPaint(reader);
            auto flags = SkCanvasPriv::LegacySaveFlagsToSaveLayerFlags(reader->readInt());
            BREAK_ON_READ_ERROR(reader);

            canvas->saveLayer(SkCanvas::SaveLayerRec(boundsPtr, paint, flags));
        } break;
        case SAVE_LAYER_SAVELAYERREC: {
            SkCanvas::SaveLayerRec rec(nullptr, nullptr, nullptr, nullptr, nullptr, 0);
            SkMatrix clipMatrix;
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
                if (const auto* paint = fPictureData->getPaint(reader)) {
                    rec.fBackdrop = paint->getImageFilter();
                }
            }
            if (flatFlags & SAVELAYERREC_HAS_FLAGS) {
                rec.fSaveLayerFlags = reader->readInt();
            }
            if (flatFlags & SAVELAYERREC_HAS_CLIPMASK) {
                rec.fClipMask = fPictureData->getImage(reader);
            }
            if (flatFlags & SAVELAYERREC_HAS_CLIPMATRIX) {
                reader->readMatrix(&clipMatrix);
                rec.fClipMatrix = &clipMatrix;
            }
            BREAK_ON_READ_ERROR(reader);

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
            reader->validate(false);    // unknown op
            break;
    }

#undef BREAK_ON_READ_ERROR
}
