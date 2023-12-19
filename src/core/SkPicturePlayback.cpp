/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/core/SkPicturePlayback.h"

#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkClipOp.h"
#include "include/core/SkColor.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRSXform.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkRegion.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkScalar.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkAlign.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTemplates.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkSafeMath.h"
#include "src/core/SkCanvasPriv.h"
#include "src/core/SkDrawShadowInfo.h"
#include "src/core/SkPictureData.h"
#include "src/core/SkPictureFlat.h"
#include "src/core/SkPicturePriv.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkVerticesPriv.h"
#include "src/utils/SkPatchUtils.h"

class SkDrawable;
class SkPath;
class SkTextBlob;
class SkVertices;

namespace sktext { namespace gpu { class Slug; } }

using namespace skia_private;

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
    reader.setVersion(fPictureData->info().getVersion());

    // Record this, so we can concat w/ it if we encounter a setMatrix()
    SkM44 initialMatrix = canvas->getLocalToDevice();

    SkAutoCanvasRestore acr(canvas, false);

    while (!reader.eof() && reader.isValid()) {
        if (callback && callback->abort()) {
            return;
        }

        fCurOffset = reader.offset();

        uint32_t bits = reader.readInt();
        uint32_t op   = bits >> 24,
                 size = bits & 0xffffff;
        if (size == 0xffffff) {
            size = reader.readInt();
        }

        if (!reader.validate(size > 0 && op > UNUSED && op <= LAST_DRAWTYPE_ENUM)) {
            return;
        }

        this->handleOp(&reader, (DrawType)op, size, canvas, initialMatrix);
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

static bool do_clip_op(SkReadBuffer* reader, SkCanvas* canvas, SkRegion::Op op,
                       SkClipOp* clipOpToUse) {
    switch(op) {
        case SkRegion::kDifference_Op:
        case SkRegion::kIntersect_Op:
            // Fully supported, identity mapping between SkClipOp and Region::Op
            *clipOpToUse = static_cast<SkClipOp>(op);
            return true;
        case SkRegion::kReplace_Op:
            // Emulate the replace by resetting first and following it up with an intersect
            SkASSERT(reader->isVersionLT(SkPicturePriv::kNoExpandingClipOps));
            SkCanvasPriv::ResetClip(canvas);
            *clipOpToUse = SkClipOp::kIntersect;
            return true;
        default:
            // An expanding clip op, which if encountered on an old SKP, we just silently ignore
            SkASSERT(reader->isVersionLT(SkPicturePriv::kNoExpandingClipOps));
            return false;
    }
}

void SkPicturePlayback::handleOp(SkReadBuffer* reader,
                                 DrawType op,
                                 uint32_t size,
                                 SkCanvas* canvas,
                                 const SkM44& initialMatrix) {
#define BREAK_ON_READ_ERROR(r)  if (!r->isValid()) break

    switch (op) {
        case NOOP: {
            SkASSERT(size >= 4);
            reader->skip(size - 4);
        } break;
        case FLUSH:
            break;
        case CLIP_PATH: {
            const SkPath& path = fPictureData->getPath(reader);
            uint32_t packed = reader->readInt();
            SkRegion::Op rgnOp = ClipParams_unpackRegionOp(reader, packed);
            bool doAA = ClipParams_unpackDoAA(packed);
            size_t offsetToRestore = reader->readInt();
            validate_offsetToRestore(reader, offsetToRestore);
            BREAK_ON_READ_ERROR(reader);

            SkClipOp clipOp;
            if (do_clip_op(reader, canvas, rgnOp, &clipOp)) {
                canvas->clipPath(path, clipOp, doAA);
            }
            if (canvas->isClipEmpty() && offsetToRestore) {
                reader->skip(offsetToRestore - reader->offset());
            }
        } break;
        case CLIP_REGION: {
            SkRegion region;
            reader->readRegion(&region);
            uint32_t packed = reader->readInt();
            SkRegion::Op rgnOp = ClipParams_unpackRegionOp(reader, packed);
            size_t offsetToRestore = reader->readInt();
            validate_offsetToRestore(reader, offsetToRestore);
            BREAK_ON_READ_ERROR(reader);

            SkClipOp clipOp;
            if (do_clip_op(reader, canvas, rgnOp, &clipOp)) {
                canvas->clipRegion(region, clipOp);
            }
            if (canvas->isClipEmpty() && offsetToRestore) {
                reader->skip(offsetToRestore - reader->offset());
            }
        } break;
        case CLIP_RECT: {
            SkRect rect;
            reader->readRect(&rect);
            uint32_t packed = reader->readInt();
            SkRegion::Op rgnOp = ClipParams_unpackRegionOp(reader, packed);
            bool doAA = ClipParams_unpackDoAA(packed);
            size_t offsetToRestore = reader->readInt();
            validate_offsetToRestore(reader, offsetToRestore);
            BREAK_ON_READ_ERROR(reader);

            SkClipOp clipOp;
            if (do_clip_op(reader, canvas, rgnOp, &clipOp)) {
                canvas->clipRect(rect, clipOp, doAA);
            }
            if (canvas->isClipEmpty() && offsetToRestore) {
                reader->skip(offsetToRestore - reader->offset());
            }
        } break;
        case CLIP_RRECT: {
            SkRRect rrect;
            reader->readRRect(&rrect);
            uint32_t packed = reader->readInt();
            SkRegion::Op rgnOp = ClipParams_unpackRegionOp(reader, packed);
            bool doAA = ClipParams_unpackDoAA(packed);
            size_t offsetToRestore = reader->readInt();
            validate_offsetToRestore(reader, offsetToRestore);
            BREAK_ON_READ_ERROR(reader);

            SkClipOp clipOp;
            if (do_clip_op(reader, canvas, rgnOp, &clipOp)) {
                canvas->clipRRect(rrect, clipOp, doAA);
            }
            if (canvas->isClipEmpty() && offsetToRestore) {
                reader->skip(offsetToRestore - reader->offset());
            }
        } break;
        case CLIP_SHADER_IN_PAINT: {
            const SkPaint& paint = fPictureData->requiredPaint(reader);
            // clipShader() was never used in conjunction with deprecated, expanding clip ops, so
            // it requires the op to just be intersect or difference.
            SkClipOp clipOp = reader->checkRange(SkClipOp::kDifference, SkClipOp::kIntersect);
            BREAK_ON_READ_ERROR(reader);

            canvas->clipShader(paint.refShader(), clipOp);
        } break;
        case RESET_CLIP:
            // For Android, an emulated "replace" clip op appears as a manual reset followed by
            // an intersect operation (equivalent to the above handling of replace ops encountered
            // in old serialized pictures).
            SkCanvasPriv::ResetClip(canvas);
            break;
        case PUSH_CULL: break;  // Deprecated, safe to ignore both push and pop.
        case POP_CULL:  break;
        case CONCAT: {
            SkMatrix matrix;
            reader->readMatrix(&matrix);
            BREAK_ON_READ_ERROR(reader);

            canvas->concat(matrix);
            break;
        }
        case CONCAT44: {
            const SkScalar* colMaj = reader->skipT<SkScalar>(16);
            BREAK_ON_READ_ERROR(reader);
            canvas->concat(SkM44::ColMajor(colMaj));
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
            const SkPaint& paint = fPictureData->requiredPaint(reader);
            SkRect rect;
            reader->readRect(&rect);
            SkScalar startAngle = reader->readScalar();
            SkScalar sweepAngle = reader->readScalar();
            int useCenter = reader->readInt();
            BREAK_ON_READ_ERROR(reader);

            canvas->drawArc(rect, startAngle, sweepAngle, SkToBool(useCenter), paint);
        } break;
        case DRAW_ATLAS: {
            const SkPaint* paint = fPictureData->optionalPaint(reader);
            const SkImage* atlas = fPictureData->getImage(reader);
            const uint32_t flags = reader->readUInt();
            const int count = reader->readUInt();
            const SkRSXform* xform = (const SkRSXform*)reader->skip(count, sizeof(SkRSXform));
            const SkRect* tex = (const SkRect*)reader->skip(count, sizeof(SkRect));
            const SkColor* colors = nullptr;
            SkBlendMode mode = SkBlendMode::kDst;
            if (flags & DRAW_ATLAS_HAS_COLORS) {
                colors = (const SkColor*)reader->skip(count, sizeof(SkColor));
                mode = reader->read32LE(SkBlendMode::kLastMode);
                BREAK_ON_READ_ERROR(reader);
            }
            const SkRect* cull = nullptr;
            if (flags & DRAW_ATLAS_HAS_CULL) {
                cull = (const SkRect*)reader->skip(sizeof(SkRect));
            }
            BREAK_ON_READ_ERROR(reader);

            SkSamplingOptions sampling;
            if (flags & DRAW_ATLAS_HAS_SAMPLING) {
                sampling = reader->readSampling();
                BREAK_ON_READ_ERROR(reader);
            }
            canvas->drawAtlas(atlas, xform, tex, colors, count, mode, sampling, cull, paint);
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
            const SkPaint& paint = fPictureData->requiredPaint(reader);
            SkRRect outer, inner;
            reader->readRRect(&outer);
            reader->readRRect(&inner);
            BREAK_ON_READ_ERROR(reader);

            canvas->drawDRRect(outer, inner, paint);
        } break;
        case DRAW_EDGEAA_QUAD: {
            SkRect rect;
            reader->readRect(&rect);
            SkCanvas::QuadAAFlags aaFlags = static_cast<SkCanvas::QuadAAFlags>(reader->read32());
            SkColor4f color;
            reader->readColor4f(&color);
            SkBlendMode blend = reader->read32LE(SkBlendMode::kLastMode);
            BREAK_ON_READ_ERROR(reader);
            bool hasClip = reader->readInt();
            const SkPoint* clip = nullptr;
            if (hasClip) {
                clip = (const SkPoint*) reader->skip(4, sizeof(SkPoint));
            }
            BREAK_ON_READ_ERROR(reader);
            canvas->experimental_DrawEdgeAAQuad(rect, clip, aaFlags, color, blend);
        } break;
        case DRAW_EDGEAA_IMAGE_SET:
        case DRAW_EDGEAA_IMAGE_SET2: {
            static const size_t kEntryReadSize =
                    4 * sizeof(uint32_t) + 2 * sizeof(SkRect) + sizeof(SkScalar);
            static const size_t kMatrixSize = 9 * sizeof(SkScalar); // != sizeof(SkMatrix)

            int cnt = reader->readInt();
            if (!reader->validate(cnt >= 0)) {
                break;
            }
            const SkPaint* paint = fPictureData->optionalPaint(reader);

            SkSamplingOptions sampling;
            if (op == DRAW_EDGEAA_IMAGE_SET2) {
                sampling = reader->readSampling();
            } else {
                sampling = SkSamplingOptions(SkFilterMode::kNearest);
            }

            SkCanvas::SrcRectConstraint constraint =
                    reader->checkRange(SkCanvas::kStrict_SrcRectConstraint,
                                       SkCanvas::kFast_SrcRectConstraint);

            if (!reader->validate(SkSafeMath::Mul(cnt, kEntryReadSize) <= reader->available())) {
                break;
            }

            // Track minimum necessary clip points and matrices that must be provided to satisfy
            // the entries.
            int expectedClips = 0;
            int maxMatrixIndex = -1;
            AutoTArray<SkCanvas::ImageSetEntry> set(cnt);
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
            const SkPoint* dstClips = nullptr;
            if (!reader->validate(dstClipCount >= 0) ||
                !reader->validate(expectedClips <= dstClipCount)) {
                // A bad dstClipCount (either negative, or not enough to satisfy entries).
                break;
            } else if (dstClipCount > 0) {
                dstClips = (const SkPoint*) reader->skip(dstClipCount, sizeof(SkPoint));
                if (dstClips == nullptr) {
                    // Not enough bytes remaining so the reader has been invalidated
                    break;
                }
            }
            int matrixCount = reader->readInt();
            if (!reader->validate(matrixCount >= 0) ||
                !reader->validate(maxMatrixIndex <= (matrixCount - 1)) ||
                !reader->validate(
                    SkSafeMath::Mul(matrixCount, kMatrixSize) <= reader->available())) {
                // Entries access out-of-bound matrix indices, given provided matrices or
                // there aren't enough bytes to provide that many matrices
                break;
            }
            TArray<SkMatrix> matrices(matrixCount);
            for (int i = 0; i < matrixCount && reader->isValid(); ++i) {
                reader->readMatrix(&matrices.push_back());
            }
            BREAK_ON_READ_ERROR(reader);

            canvas->experimental_DrawEdgeAAImageSet(set.get(), cnt, dstClips, matrices.begin(),
                                                    sampling, paint, constraint);
        } break;
        case DRAW_IMAGE: {
            const SkPaint* paint = fPictureData->optionalPaint(reader);
            const SkImage* image = fPictureData->getImage(reader);
            SkPoint loc;
            reader->readPoint(&loc);
            BREAK_ON_READ_ERROR(reader);

            canvas->drawImage(image, loc.fX, loc.fY,
                              SkSamplingOptions(SkFilterMode::kNearest),
                              paint);
        } break;
        case DRAW_IMAGE2: {
            const SkPaint* paint = fPictureData->optionalPaint(reader);
            const SkImage* image = fPictureData->getImage(reader);
            SkPoint loc;
            reader->readPoint(&loc);
            SkSamplingOptions sampling = reader->readSampling();
            BREAK_ON_READ_ERROR(reader);

            canvas->drawImage(image, loc.fX, loc.fY, sampling, paint);
        } break;
        case DRAW_IMAGE_LATTICE: {
            const SkPaint* paint = fPictureData->optionalPaint(reader);
            const SkImage* image = fPictureData->getImage(reader);
            SkCanvas::Lattice lattice;
            (void)SkCanvasPriv::ReadLattice(*reader, &lattice);
            const SkRect* dst = reader->skipT<SkRect>();
            BREAK_ON_READ_ERROR(reader);

            canvas->drawImageLattice(image, lattice, *dst, SkFilterMode::kNearest, paint);
        } break;
        case DRAW_IMAGE_LATTICE2: {
            const SkPaint* paint = fPictureData->optionalPaint(reader);
            const SkImage* image = fPictureData->getImage(reader);
            SkCanvas::Lattice lattice;
            (void)SkCanvasPriv::ReadLattice(*reader, &lattice);
            const SkRect* dst = reader->skipT<SkRect>();
            SkFilterMode filter = reader->read32LE(SkFilterMode::kLinear);
            BREAK_ON_READ_ERROR(reader);

            canvas->drawImageLattice(image, lattice, *dst, filter, paint);
        } break;
        case DRAW_IMAGE_NINE: {
            const SkPaint* paint = fPictureData->optionalPaint(reader);
            const SkImage* image = fPictureData->getImage(reader);
            SkIRect center;
            reader->readIRect(&center);
            SkRect dst;
            reader->readRect(&dst);
            BREAK_ON_READ_ERROR(reader);

            canvas->drawImageNine(image, center, dst, SkFilterMode::kNearest, paint);
        } break;
        case DRAW_IMAGE_RECT: {
            const SkPaint* paint = fPictureData->optionalPaint(reader);
            const SkImage* image = fPictureData->getImage(reader);
            SkRect storage;
            const SkRect* src = get_rect_ptr(reader, &storage);   // may be null
            SkRect dst;
            reader->readRect(&dst);     // required
            // DRAW_IMAGE_RECT_STRICT assumes this constraint, and doesn't store it
            SkCanvas::SrcRectConstraint constraint = SkCanvas::kStrict_SrcRectConstraint;
            if (DRAW_IMAGE_RECT == op) {
                // newer op-code stores the constraint explicitly
                constraint = reader->checkRange(SkCanvas::kStrict_SrcRectConstraint,
                                                SkCanvas::kFast_SrcRectConstraint);
            }
            BREAK_ON_READ_ERROR(reader);

            auto sampling = SkSamplingOptions(SkFilterMode::kNearest);
            if (src) {
                canvas->drawImageRect(image, *src, dst, sampling, paint, constraint);
            } else {
                canvas->drawImageRect(image, dst, sampling, paint);
            }
        } break;
        case DRAW_IMAGE_RECT2: {
            const SkPaint* paint = fPictureData->optionalPaint(reader);
            const SkImage* image = fPictureData->getImage(reader);
            SkRect src = reader->readRect();
            SkRect dst = reader->readRect();
            SkSamplingOptions sampling = reader->readSampling();
            auto constraint = reader->read32LE(SkCanvas::kFast_SrcRectConstraint);
            BREAK_ON_READ_ERROR(reader);

            canvas->drawImageRect(image, src, dst, sampling, paint, constraint);
        } break;
        case DRAW_OVAL: {
            const SkPaint& paint = fPictureData->requiredPaint(reader);
            SkRect rect;
            reader->readRect(&rect);
            BREAK_ON_READ_ERROR(reader);

            canvas->drawOval(rect, paint);
        } break;
        case DRAW_PAINT: {
            const SkPaint& paint = fPictureData->requiredPaint(reader);
            BREAK_ON_READ_ERROR(reader);

            canvas->drawPaint(paint);
        } break;
        case DRAW_BEHIND_PAINT: {
            const SkPaint& paint = fPictureData->requiredPaint(reader);
            BREAK_ON_READ_ERROR(reader);

            SkCanvasPriv::DrawBehind(canvas, paint);
        } break;
        case DRAW_PATCH: {
            const SkPaint& paint = fPictureData->requiredPaint(reader);

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

            canvas->drawPatch(cubics, colors, texCoords, bmode, paint);
        } break;
        case DRAW_PATH: {
            const SkPaint& paint = fPictureData->requiredPaint(reader);
            const auto& path = fPictureData->getPath(reader);
            BREAK_ON_READ_ERROR(reader);

            canvas->drawPath(path, paint);
        } break;
        case DRAW_PICTURE: {
            const auto* pic = fPictureData->getPicture(reader);
            BREAK_ON_READ_ERROR(reader);

            canvas->drawPicture(pic);
        } break;
        case DRAW_PICTURE_MATRIX_PAINT: {
            const SkPaint* paint = fPictureData->optionalPaint(reader);
            SkMatrix matrix;
            reader->readMatrix(&matrix);
            const SkPicture* pic = fPictureData->getPicture(reader);
            BREAK_ON_READ_ERROR(reader);

            canvas->drawPicture(pic, &matrix, paint);
        } break;
        case DRAW_POINTS: {
            const SkPaint& paint = fPictureData->requiredPaint(reader);
            SkCanvas::PointMode mode = reader->checkRange(SkCanvas::kPoints_PointMode,
                                                          SkCanvas::kPolygon_PointMode);
            size_t count = reader->readInt();
            const SkPoint* pts = (const SkPoint*)reader->skip(count, sizeof(SkPoint));
            BREAK_ON_READ_ERROR(reader);

            canvas->drawPoints(mode, count, pts, paint);
        } break;
        case DRAW_RECT: {
            const SkPaint& paint = fPictureData->requiredPaint(reader);
            SkRect rect;
            reader->readRect(&rect);
            BREAK_ON_READ_ERROR(reader);

            canvas->drawRect(rect, paint);
        } break;
        case DRAW_REGION: {
            const SkPaint& paint = fPictureData->requiredPaint(reader);
            SkRegion region;
            reader->readRegion(&region);
            BREAK_ON_READ_ERROR(reader);

            canvas->drawRegion(region, paint);
        } break;
        case DRAW_RRECT: {
            const SkPaint& paint = fPictureData->requiredPaint(reader);
            SkRRect rrect;
            reader->readRRect(&rrect);
            BREAK_ON_READ_ERROR(reader);

            canvas->drawRRect(rrect, paint);
        } break;
        case DRAW_SHADOW_REC: {
            const auto& path = fPictureData->getPath(reader);
            SkDrawShadowRec rec;
            reader->readPoint3(&rec.fZPlaneParams);
            reader->readPoint3(&rec.fLightPos);
            rec.fLightRadius = reader->readScalar();
            rec.fAmbientColor = reader->read32();
            rec.fSpotColor = reader->read32();
            rec.fFlags = reader->read32();
            BREAK_ON_READ_ERROR(reader);

            canvas->private_draw_shadow_rec(path, rec);
        } break;
        case DRAW_TEXT_BLOB: {
            const SkPaint& paint = fPictureData->requiredPaint(reader);
            const SkTextBlob* blob = fPictureData->getTextBlob(reader);
            SkScalar x = reader->readScalar();
            SkScalar y = reader->readScalar();
            BREAK_ON_READ_ERROR(reader);

            canvas->drawTextBlob(blob, x, y, paint);
        } break;
        case DRAW_SLUG: {
            const sktext::gpu::Slug* slug = fPictureData->getSlug(reader);
            BREAK_ON_READ_ERROR(reader);

            canvas->drawSlug(slug);
        } break;
        case DRAW_VERTICES_OBJECT: {
            const SkPaint& paint = fPictureData->requiredPaint(reader);
            const SkVertices* vertices = fPictureData->getVertices(reader);
            const int boneCount = reader->readInt();
            (void)reader->skip(boneCount, sizeof(SkVertices_DeprecatedBone));
            SkBlendMode bmode = reader->read32LE(SkBlendMode::kLastMode);
            BREAK_ON_READ_ERROR(reader);

            if (vertices) {  // TODO: read error if vertices == null?
                canvas->drawVertices(vertices, bmode, paint);
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
        case SAVE_LAYER_SAVELAYERREC: {
            SkCanvas::SaveLayerRec rec(nullptr, nullptr, nullptr, 0);
            const uint32_t flatFlags = reader->readInt();
            SkRect bounds;
            skia_private::AutoSTArray<2, sk_sp<SkImageFilter>> filters;
            if (flatFlags & SAVELAYERREC_HAS_BOUNDS) {
                reader->readRect(&bounds);
                rec.fBounds = &bounds;
            }
            if (flatFlags & SAVELAYERREC_HAS_PAINT) {
                rec.fPaint = &fPictureData->requiredPaint(reader);
            }
            if (flatFlags & SAVELAYERREC_HAS_BACKDROP) {
                const SkPaint& paint = fPictureData->requiredPaint(reader);
                rec.fBackdrop = paint.getImageFilter();
            }
            if (flatFlags & SAVELAYERREC_HAS_FLAGS) {
                rec.fSaveLayerFlags = reader->readInt();
            }
            if (flatFlags & SAVELAYERREC_HAS_CLIPMASK_OBSOLETE) {
                (void)fPictureData->getImage(reader);
            }
            if (flatFlags & SAVELAYERREC_HAS_CLIPMATRIX_OBSOLETE) {
                SkMatrix clipMatrix_ignored;
                reader->readMatrix(&clipMatrix_ignored);
            }
            if (!reader->isVersionLT(SkPicturePriv::Version::kBackdropScaleFactor) &&
                (flatFlags & SAVELAYERREC_HAS_BACKDROP_SCALE)) {
                SkCanvasPriv::SetBackdropScaleFactor(&rec, reader->readScalar());
            }
            if (!reader->isVersionLT(SkPicturePriv::Version::kMultipleFiltersOnSaveLayer) &&
                (flatFlags & SAVELAYERREC_HAS_MULTIPLE_FILTERS)) {
                int filterCount = reader->readUInt();
                reader->validate(filterCount > 0 && filterCount <= SkCanvas::kMaxFiltersPerLayer);
                BREAK_ON_READ_ERROR(reader);
                filters.reset(filterCount);
                for (int i = 0; i < filterCount; ++i) {
                    const SkPaint& paint = fPictureData->requiredPaint(reader);
                    filters[i] = paint.refImageFilter();
                }
                rec.fFilters = filters;
            }
            BREAK_ON_READ_ERROR(reader);

            canvas->saveLayer(rec);
        } break;
        case SCALE: {
            SkScalar sx = reader->readScalar();
            SkScalar sy = reader->readScalar();
            canvas->scale(sx, sy);
        } break;
        case SET_M44: {
            SkM44 m;
            reader->read(&m);
            canvas->setMatrix(initialMatrix * m);
        } break;
        case SET_MATRIX: {
            SkMatrix matrix;
            reader->readMatrix(&matrix);
            canvas->setMatrix(initialMatrix * SkM44(matrix));
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
