/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAnnotation.h"
#include "SkCanvas.h"
#include "SkColorFilter.h"
#include "SkDrawLooper.h"
#include "SkImage.h"
#include "SkImageFilter.h"
#include "SkMaskFilter.h"
#include "SkNinePatchIter.h"
#include "SkPath.h"
#include "SkPathEffect.h"
#include "SkRasterizer.h"
#include "SkRect.h"
#include "SkRemote.h"
#include "SkShader.h"
#include "SkTHash.h"
#include "SkTextBlob.h"

namespace SkRemote {

    Misc Misc::CreateFrom(const SkPaint& paint) {
        Misc misc = {
            paint.getColor(),
            paint.getFilterQuality(),
            paint.isAntiAlias(),
            paint.isDither(),
        };
        return misc;
    }

    void Misc::applyTo(SkPaint* paint) const {
        paint->setColor        (fColor);
        paint->setFilterQuality(fFilterQuality);
        paint->setAntiAlias    (fAntiAlias);
        paint->setDither       (fDither);
    }

    static bool operator==(const Misc& a, const Misc& b) {
        return a.fColor         == b.fColor
            && a.fFilterQuality == b.fFilterQuality
            && a.fAntiAlias     == b.fAntiAlias
            && a.fDither        == b.fDither;
    }

    // Misc carries 10 bytes of data in a 12 byte struct, so we need a custom hash.
    static_assert(sizeof(Misc) > offsetof(Misc, fDither) + sizeof(Misc().fDither), "");
    struct MiscHash {
        uint32_t operator()(const Misc& misc) {
            return SkChecksum::Murmur3(&misc, offsetof(Misc, fDither) + sizeof(Misc().fDither));
        }
    };

    Stroke Stroke::CreateFrom(const SkPaint& paint) {
        Stroke stroke = {
            paint.getStrokeWidth(),
            paint.getStrokeMiter(),
            paint.getStrokeCap(),
            paint.getStrokeJoin(),
        };
        return stroke;
    }

    void Stroke::applyTo(SkPaint* paint) const {
        paint->setStrokeWidth(fWidth);
        paint->setStrokeMiter(fMiter);
        paint->setStrokeCap  (fCap);
        paint->setStrokeJoin (fJoin);
    }

    static bool operator==(const Stroke& a, const Stroke& b) {
        return a.fWidth == b.fWidth
            && a.fMiter == b.fMiter
            && a.fCap   == b.fCap
            && a.fJoin  == b.fJoin;
    }

    // The default SkGoodHash works fine for Stroke, as it's dense.
    static_assert(sizeof(Stroke) == offsetof(Stroke, fJoin) + sizeof(Stroke().fJoin), "");

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

    class Canvas final : public SkCanvas {
    public:
        explicit Canvas(Encoder* encoder)
            : SkCanvas(1,1)
            , fEncoder(encoder) {}

    private:
        // Calls Encoder::define() when created, Encoder::undefine() when destroyed.
        class AutoID : ::SkNoncopyable {
        public:
            template <typename T>
            explicit AutoID(Encoder* encoder, const T& val)
                : fEncoder(encoder)
                , fID(encoder->define(val)) {}
            ~AutoID() { if (fEncoder) fEncoder->undefine(fID); }

            AutoID(AutoID&& o) : fEncoder(o.fEncoder), fID(o.fID) {
                o.fEncoder = nullptr;
            }
            AutoID& operator=(AutoID&&) = delete;

            operator ID () const { return fID; }

        private:
            Encoder* fEncoder;
            const ID fID;
        };

        // Like AutoID, but for CommonIDs.
        class AutoCommonIDs : ::SkNoncopyable {
        public:
            explicit AutoCommonIDs(Encoder* encoder, const SkPaint& paint)
                : fEncoder(encoder) {
                fIDs.misc        = fEncoder->define(Misc::CreateFrom(paint));
                fIDs.patheffect  = fEncoder->define(paint.getPathEffect());
                fIDs.shader      = fEncoder->define(paint.getShader());
                fIDs.xfermode    = fEncoder->define(paint.getXfermode());
                fIDs.maskfilter  = fEncoder->define(paint.getMaskFilter());
                fIDs.colorfilter = fEncoder->define(paint.getColorFilter());
                fIDs.rasterizer  = fEncoder->define(paint.getRasterizer());
                fIDs.looper      = fEncoder->define(paint.getLooper());
                fIDs.imagefilter = fEncoder->define(paint.getImageFilter());
                fIDs.annotation  = fEncoder->define(paint.getAnnotation());
            }
            ~AutoCommonIDs() {
                if (fEncoder) {
                    fEncoder->undefine(fIDs.misc);
                    fEncoder->undefine(fIDs.patheffect);
                    fEncoder->undefine(fIDs.shader);
                    fEncoder->undefine(fIDs.xfermode);
                    fEncoder->undefine(fIDs.maskfilter);
                    fEncoder->undefine(fIDs.colorfilter);
                    fEncoder->undefine(fIDs.rasterizer);
                    fEncoder->undefine(fIDs.looper);
                    fEncoder->undefine(fIDs.imagefilter);
                    fEncoder->undefine(fIDs.annotation);
                }
            }

            AutoCommonIDs(AutoCommonIDs&& o) : fEncoder(o.fEncoder), fIDs(o.fIDs) {
                o.fEncoder = nullptr;
            }
            AutoID& operator=(AutoID&&) = delete;

            operator Encoder::CommonIDs () const { return fIDs; }

        private:
            Encoder*           fEncoder;
            Encoder::CommonIDs fIDs;
        };

        template <typename T>
        AutoID id(const T& val) { return AutoID(fEncoder, val); }

        AutoCommonIDs commonIDs(const SkPaint& paint) { return AutoCommonIDs(fEncoder, paint); }

        void   willSave() override { fEncoder->   save(); }
        void didRestore() override { fEncoder->restore(); }
        SaveLayerStrategy getSaveLayerStrategy(const SaveLayerRec& rec) override {
            SkPath path;
            if (rec.fBounds) {
                path.addRect(*rec.fBounds);
            }
            const SkPaint defaultPaint;
            const SkPaint* paint = rec.fPaint;
            if (!paint) {
                paint = &defaultPaint;
            }
            fEncoder->saveLayer(this->id(path), this->commonIDs(*paint), rec.fSaveLayerFlags);
            return kNoLayer_SaveLayerStrategy;
        }

        void    didConcat(const SkMatrix&) override { this->didSetMatrix(this->getTotalMatrix()); }
        void didSetMatrix(const SkMatrix& matrix) override {
            fEncoder->setMatrix(this->id(matrix));
        }

        void onDrawOval(const SkRect& oval, const SkPaint& paint) override {
            SkPath path;
            path.addOval(oval);
            this->onDrawPath(path, paint);
        }

        void onDrawRect(const SkRect& rect, const SkPaint& paint) override {
            SkPath path;
            path.addRect(rect);
            this->onDrawPath(path, paint);
        }

        void onDrawRRect(const SkRRect& rrect, const SkPaint& paint) override {
            SkPath path;
            path.addRRect(rrect);
            this->onDrawPath(path, paint);
        }

        void onDrawDRRect(const SkRRect& outside, const SkRRect& inside,
                          const SkPaint& paint) override {
            SkPath path;
            path.addRRect(outside);
            path.addRRect(inside, SkPath::kCCW_Direction);
            this->onDrawPath(path, paint);
        }

        void onDrawPath(const SkPath& path, const SkPaint& paint) override {
            auto common = this->commonIDs(paint);
            auto p = this->id(path);

            if (paint.getStyle() == SkPaint::kFill_Style) {
                fEncoder->fillPath(p, common);
            } else {
                // TODO: handle kStrokeAndFill_Style
                fEncoder->strokePath(p, common, this->id(Stroke::CreateFrom(paint)));
            }
        }

        void onDrawPaint(const SkPaint& paint) override {
            SkPath path;
            path.setFillType(SkPath::kInverseWinding_FillType);  // Either inverse FillType is fine.
            this->onDrawPath(path, paint);
        }

        void onDrawPoints(PointMode mode,
                          size_t count,
                          const SkPoint pts[],
                          const SkPaint& paint) override {
            // TODO
        }

        void onDrawDrawable(SkDrawable* drawable, const SkMatrix* matrix) override {
            // TODO
            this->INHERITED::onDrawDrawable(drawable, matrix);
        }

        void onDrawPicture(const SkPicture* pic,
                           const SkMatrix* matrix,
                           const SkPaint* paint) override {
            // TODO
            this->INHERITED::onDrawPicture(pic, matrix, paint);
        }

        void onDrawVertices(VertexMode vmode,
                            int vertexCount,
                            const SkPoint vertices[],
                            const SkPoint texs[],
                            const SkColor colors[],
                            SkXfermode* xmode,
                            const uint16_t indices[],
                            int indexCount,
                            const SkPaint& paint) override {
            // TODO
        }

        void onDrawPatch(const SkPoint cubics[12],
                         const SkColor colors[4],
                         const SkPoint texCoords[4],
                         SkXfermode* xmode,
                         const SkPaint& paint) override {
            // TODO
        }

        void onDrawAtlas(const SkImage* atlas,
                         const SkRSXform xform[],
                         const SkRect tex[],
                         const SkColor colors[],
                         int count,
                         SkXfermode::Mode mode,
                         const SkRect* cull,
                         const SkPaint* paint) override {
            // TODO
        }

        void onDrawBitmap(const SkBitmap& bitmap,
                          SkScalar left,
                          SkScalar top,
                          const SkPaint* paint) override {
            auto src = SkRect::MakeWH(bitmap.width(), bitmap.height()),
                 dst = src.makeOffset(left, top);
            this->onDrawBitmapRect(bitmap, &src, dst, paint, kStrict_SrcRectConstraint);
        }

        void onDrawBitmapRect(const SkBitmap& bitmap,
                              const SkRect* src,
                              const SkRect& dst,
                              const SkPaint* paint,
                              SrcRectConstraint constraint) override {
            SkAutoTUnref<SkImage> image(SkImage::NewFromBitmap(bitmap));
            this->onDrawImageRect(image, src, dst, paint, constraint);
        }

        void onDrawImage(const SkImage* image,
                         SkScalar left,
                         SkScalar top,
                         const SkPaint* paint) override {
            if (!image) {
                return;
            }
            auto src = SkRect::MakeWH(image->width(), image->height()),
                 dst = src.makeOffset(left, top);
            this->onDrawImageRect(image, &src, dst, paint, kStrict_SrcRectConstraint);
        }

        void onDrawImageRect(const SkImage* image,
                             const SkRect* src,
                             const SkRect& dst,
                             const SkPaint* paint,
                             SrcRectConstraint constraint) override {
            // TODO: this is all a (likely buggy) hack to get images drawing quickly.
            if (!image) {
                return;
            }

            auto bounds = SkRect::MakeWH(image->width(), image->height());
            if (!src) {
                src = &bounds;
            }
            auto matrix = SkMatrix::MakeRectToRect(*src, dst, SkMatrix::kFill_ScaleToFit);

            SkAutoTUnref<SkImage> subset;
            if (src) {
                if (!bounds.intersect(*src)) {
                    return;
                }
                subset.reset(image->newSubset(bounds.roundOut()));
                image = subset;
            }

            auto paintWithShader = paint ? *paint : SkPaint();
            SkAutoTUnref<SkShader> shader(
                image->newShader(SkShader::kClamp_TileMode, SkShader::kClamp_TileMode, &matrix));
            paintWithShader.setShader(shader);

            this->onDrawRect(dst, paintWithShader);
        }

        void onDrawBitmapNine(const SkBitmap& bitmap,
                              const SkIRect& center,
                              const SkRect& dst,
                              const SkPaint* paint) override {
            SkAutoTUnref<SkImage> image(SkImage::NewFromBitmap(bitmap));
            this->onDrawImageNine(image, center, dst, paint);
        }

        void onDrawImageNine(const SkImage* image,
                             const SkIRect& center,
                             const SkRect& dst,
                             const SkPaint* paint) override {
            SkNinePatchIter iter(image->width(), image->height(), center, dst);
            SkRect s,d;
            while (iter.next(&s, &d)) {
                this->onDrawImageRect(image, &s, d, paint, kStrict_SrcRectConstraint);
            }
        }

        void onDrawTextBlob(const SkTextBlob* text,
                            SkScalar x,
                            SkScalar y,
                            const SkPaint& paint) override {
            SkPoint offset{x,y};
            auto t = this->id(text);
            auto common = this->commonIDs(paint);

            if (paint.getStyle() == SkPaint::kFill_Style) {
                fEncoder->fillText(t, offset, common);
            } else {
                // TODO: handle kStrokeAndFill_Style
                fEncoder->strokeText(t, offset, common, this->id(Stroke::CreateFrom(paint)));
            }
        }

        void onDrawText(const void* text, size_t byteLength,
                        SkScalar x, SkScalar y, const SkPaint& paint) override {
            // Text-as-paths is a temporary hack.
            // TODO: send SkTextBlobs and SkTypefaces
            SkPath path;
            paint.getTextPath(text, byteLength, x, y, &path);
            this->onDrawPath(path, paint);
        }

        void onDrawPosText(const void* text, size_t byteLength,
                           const SkPoint pos[], const SkPaint& paint) override {
            // Text-as-paths is a temporary hack.
            // TODO: send SkTextBlobs and SkTypefaces
            SkPath path;
            paint.getPosTextPath(text, byteLength, pos, &path);
            this->onDrawPath(path, paint);
        }

        void onDrawPosTextH(const void* text, size_t byteLength,
                            const SkScalar xpos[], SkScalar constY, const SkPaint& paint) override {
            size_t length = paint.countText(text, byteLength);
            SkAutoTArray<SkPoint> pos(length);
            for(size_t i = 0; i < length; ++i) {
                pos[i].set(xpos[i], constY);
            }
            this->onDrawPosText(text, byteLength, &pos[0], paint);
        }

        // All clip calls need to call their parent method or we'll not get any quick rejects.
        void onClipRect(const SkRect& rect, SkRegion::Op op, ClipEdgeStyle edgeStyle) override {
            this->INHERITED::onClipRect(rect, op, edgeStyle);
            SkPath path;
            path.addRect(rect);
            this->onClipPath(path, op, edgeStyle);
        }

        void onClipRRect(const SkRRect& rrect, SkRegion::Op op, ClipEdgeStyle edgeStyle) override {
            this->INHERITED::onClipRRect(rrect, op, edgeStyle);
            SkPath path;
            path.addRRect(rrect);
            this->onClipPath(path, op, edgeStyle);
        }

        void onClipPath(const SkPath& path, SkRegion::Op op, ClipEdgeStyle edgeStyle) override {
            this->INHERITED::onClipPath(path, op, edgeStyle);
            fEncoder->clipPath(this->id(path), op, edgeStyle == kSoft_ClipEdgeStyle);
        }

        void onClipRegion(const SkRegion& region, SkRegion::Op op) override {
            this->INHERITED::onClipRegion(region, op);
            // TODO
        }

        Encoder* fEncoder;
        typedef SkCanvas INHERITED;
    };

    SkCanvas* NewCanvas(Encoder* encoder) { return new Canvas(encoder); }

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

    class Decoder final : public Encoder {
    public:
        explicit Decoder(SkCanvas* canvas) : fCanvas(canvas) {}

    private:
        template <typename Map, typename T>
        ID define(Type type, Map* map, const T& val) {
            ID id(type, fNextID++);
            map->set(id, val);
            return id;
        }

    #define O override
        ID define(const SkMatrix&   v)O{return this->define(Type::kMatrix,      &fMatrix,      v);}
        ID define(const Misc&       v)O{return this->define(Type::kMisc,        &fMisc,        v);}
        ID define(const SkPath&     v)O{return this->define(Type::kPath,        &fPath,        v);}
        ID define(const Stroke&     v)O{return this->define(Type::kStroke,      &fStroke,      v);}
        ID define(const SkTextBlob* v)O{return this->define(Type::kTextBlob,    &fTextBlob,    v);}
        ID define(SkPathEffect*     v)O{return this->define(Type::kPathEffect,  &fPathEffect,  v);}
        ID define(SkShader*         v)O{return this->define(Type::kShader,      &fShader,      v);}
        ID define(SkXfermode*       v)O{return this->define(Type::kXfermode,    &fXfermode,    v);}
        ID define(SkMaskFilter*     v)O{return this->define(Type::kMaskFilter,  &fMaskFilter,  v);}
        ID define(SkColorFilter*    v)O{return this->define(Type::kColorFilter, &fColorFilter, v);}
        ID define(SkRasterizer*     v)O{return this->define(Type::kRasterizer,  &fRasterizer,  v);}
        ID define(SkDrawLooper*     v)O{return this->define(Type::kDrawLooper,  &fDrawLooper,  v);}
        ID define(SkImageFilter*    v)O{return this->define(Type::kImageFilter, &fImageFilter, v);}
        ID define(SkAnnotation*     v)O{return this->define(Type::kAnnotation,  &fAnnotation,  v);}
    #undef O


        void undefine(ID id) override {
            switch(id.type()) {
                case Type::kMatrix:      return fMatrix     .remove(id);
                case Type::kMisc:        return fMisc       .remove(id);
                case Type::kPath:        return fPath       .remove(id);
                case Type::kStroke:      return fStroke     .remove(id);
                case Type::kTextBlob:    return fTextBlob   .remove(id);
                case Type::kPathEffect:  return fPathEffect .remove(id);
                case Type::kShader:      return fShader     .remove(id);
                case Type::kXfermode:    return fXfermode   .remove(id);
                case Type::kMaskFilter:  return fMaskFilter .remove(id);
                case Type::kColorFilter: return fColorFilter.remove(id);
                case Type::kRasterizer:  return fRasterizer .remove(id);
                case Type::kDrawLooper:  return fDrawLooper .remove(id);
                case Type::kImageFilter: return fImageFilter.remove(id);
                case Type::kAnnotation:  return fAnnotation .remove(id);
            };
        }

        void applyCommon(const CommonIDs& common, SkPaint* paint) const {
            fMisc.find(common.misc).applyTo(paint);
            paint->setPathEffect (fPathEffect .find(common.patheffect));
            paint->setShader     (fShader     .find(common.shader));
            paint->setXfermode   (fXfermode   .find(common.xfermode));
            paint->setMaskFilter (fMaskFilter .find(common.maskfilter));
            paint->setColorFilter(fColorFilter.find(common.colorfilter));
            paint->setRasterizer (fRasterizer .find(common.rasterizer));
            paint->setLooper     (fDrawLooper .find(common.looper));
            paint->setImageFilter(fImageFilter.find(common.imagefilter));
            paint->setAnnotation (fAnnotation .find(common.annotation));
        }

        void    save() override { fCanvas->save(); }
        void restore() override { fCanvas->restore(); }
        void saveLayer(ID bounds, CommonIDs common, SkCanvas::SaveLayerFlags flags) override {
            SkPaint paint;
            this->applyCommon(common, &paint);
            SkRect rect;

            fCanvas->saveLayer({ fPath.find(bounds).isRect(&rect) ? &rect : nullptr,
                                 &paint, flags });
        }

        void setMatrix(ID matrix) override { fCanvas->setMatrix(fMatrix.find(matrix)); }

        void clipPath(ID path, SkRegion::Op op, bool aa) override {
            fCanvas->clipPath(fPath.find(path), op, aa);
        }
        void fillPath(ID path, CommonIDs common) override {
            SkPaint paint;
            paint.setStyle(SkPaint::kFill_Style);
            this->applyCommon(common, &paint);
            fCanvas->drawPath(fPath.find(path), paint);
        }
        void strokePath(ID path, CommonIDs common, ID stroke) override {
            SkPaint paint;
            paint.setStyle(SkPaint::kStroke_Style);
            this->applyCommon(common, &paint);
            fStroke.find(stroke).applyTo(&paint);
            fCanvas->drawPath(fPath.find(path), paint);
        }
        void fillText(ID text, SkPoint offset, CommonIDs common) override {
            SkPaint paint;
            paint.setStyle(SkPaint::kFill_Style);
            this->applyCommon(common, &paint);
            fCanvas->drawTextBlob(fTextBlob.find(text), offset.x(), offset.y(), paint);
        }
        void strokeText(ID text, SkPoint offset, CommonIDs common, ID stroke) override {
            SkPaint paint;
            this->applyCommon(common, &paint);
            fStroke.find(stroke).applyTo(&paint);
            fCanvas->drawTextBlob(fTextBlob.find(text), offset.x(), offset.y(), paint);
        }

        // Maps ID -> T.
        template <typename T, Type kType>
        class IDMap {
        public:
            ~IDMap() {
                // A well-behaved client always cleans up its definitions.
                SkASSERT(fMap.count() == 0);
            }

            void set(const ID& id, const T& val) {
                SkASSERT(id.type() == kType);
                fMap.set(id, val);
            }

            void remove(const ID& id) {
                SkASSERT(id.type() == kType);
                fMap.remove(id);
            }

            const T& find(const ID& id) const {
                SkASSERT(id.type() == kType);
                T* val = fMap.find(id);
                SkASSERT(val != nullptr);
                return *val;
            }

        private:
            SkTHashMap<ID, T> fMap;
        };

        // Maps ID -> T*, and keeps the T alive by reffing it.
        template <typename T, Type kType>
        class ReffedIDMap {
        public:
            ReffedIDMap() {}
            ~ReffedIDMap() {
                // A well-behaved client always cleans up its definitions.
                SkASSERT(fMap.count() == 0);
            }

            void set(const ID& id, T* val) {
                SkASSERT(id.type() == kType);
                fMap.set(id, SkSafeRef(val));
            }

            void remove(const ID& id) {
                SkASSERT(id.type() == kType);
                T** val = fMap.find(id);
                SkASSERT(val);
                SkSafeUnref(*val);
                fMap.remove(id);
            }

            T* find(const ID& id) const {
                SkASSERT(id.type() == kType);
                T** val = fMap.find(id);
                SkASSERT(val);
                return *val;
            }

        private:
            SkTHashMap<ID, T*> fMap;
        };


              IDMap<SkMatrix        , Type::kMatrix     > fMatrix;
              IDMap<Misc            , Type::kMisc       > fMisc;
              IDMap<SkPath          , Type::kPath       > fPath;
              IDMap<Stroke          , Type::kStroke     > fStroke;
        ReffedIDMap<const SkTextBlob, Type::kTextBlob   > fTextBlob;
        ReffedIDMap<SkPathEffect    , Type::kPathEffect > fPathEffect;
        ReffedIDMap<SkShader        , Type::kShader     > fShader;
        ReffedIDMap<SkXfermode      , Type::kXfermode   > fXfermode;
        ReffedIDMap<SkMaskFilter    , Type::kMaskFilter > fMaskFilter;
        ReffedIDMap<SkColorFilter   , Type::kColorFilter> fColorFilter;
        ReffedIDMap<SkRasterizer    , Type::kRasterizer > fRasterizer;
        ReffedIDMap<SkDrawLooper    , Type::kDrawLooper > fDrawLooper;
        ReffedIDMap<SkImageFilter   , Type::kImageFilter> fImageFilter;
        ReffedIDMap<SkAnnotation    , Type::kAnnotation > fAnnotation;

        SkCanvas* fCanvas;
        uint64_t fNextID = 0;
    };

    Encoder* NewDecoder(SkCanvas* canvas) { return new Decoder(canvas); }

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

    class CachingEncoder final : public Encoder {
    public:
        explicit CachingEncoder(Encoder* wrapped) : fWrapped(wrapped) {}

    private:
        struct Undef {
            Encoder* fEncoder;
            template <typename T>
            void operator()(const T&, ID* id) const { fEncoder->undefine(*id); }
        };

        ~CachingEncoder() override {
            Undef undef{fWrapped};
            fMatrix     .foreach(undef);
            fMisc       .foreach(undef);
            fPath       .foreach(undef);
            fStroke     .foreach(undef);
            fTextBlob   .foreach(undef);
            fPathEffect .foreach(undef);
            fShader     .foreach(undef);
            fXfermode   .foreach(undef);
            fMaskFilter .foreach(undef);
            fColorFilter.foreach(undef);
            fRasterizer .foreach(undef);
            fDrawLooper .foreach(undef);
            fImageFilter.foreach(undef);
            fAnnotation .foreach(undef);
        }

        template <typename Map, typename T>
        ID define(Map* map, const T& v) {
            if (const ID* id = map->find(v)) {
                return *id;
            }
            ID id = fWrapped->define(v);
            map->set(v, id);
            return id;
        }

        ID define(const SkMatrix&   v) override { return this->define(&fMatrix     , v); }
        ID define(const Misc&       v) override { return this->define(&fMisc       , v); }
        ID define(const SkPath&     v) override { return this->define(&fPath       , v); }
        ID define(const Stroke&     v) override { return this->define(&fStroke     , v); }
        ID define(const SkTextBlob* v) override { return this->define(&fTextBlob   , v); }
        ID define(SkPathEffect*     v) override { return this->define(&fPathEffect , v); }
        ID define(SkShader*         v) override { return this->define(&fShader     , v); }
        ID define(SkXfermode*       v) override { return this->define(&fXfermode   , v); }
        ID define(SkMaskFilter*     v) override { return this->define(&fMaskFilter , v); }
        ID define(SkColorFilter*    v) override { return this->define(&fColorFilter, v); }
        ID define(SkRasterizer*     v) override { return this->define(&fRasterizer , v); }
        ID define(SkDrawLooper*     v) override { return this->define(&fDrawLooper , v); }
        ID define(SkImageFilter*    v) override { return this->define(&fImageFilter, v); }
        ID define(SkAnnotation*     v) override { return this->define(&fAnnotation , v); }

        void undefine(ID) override {}

        void    save() override { fWrapped->   save(); }
        void restore() override { fWrapped->restore(); }
        void saveLayer(ID bounds, CommonIDs common, SkCanvas::SaveLayerFlags flags) override {
            fWrapped->saveLayer(bounds, common, flags);
        }

        void setMatrix(ID matrix) override { fWrapped->setMatrix(matrix); }

        void clipPath(ID path, SkRegion::Op op, bool aa) override {
            fWrapped->clipPath(path, op, aa);
        }
        void fillPath(ID path, CommonIDs common) override {
            fWrapped->fillPath(path, common);
        }
        void strokePath(ID path, CommonIDs common, ID stroke) override {
            fWrapped->strokePath(path, common, stroke);
        }
        void fillText(ID text, SkPoint offset, CommonIDs common) override {
            fWrapped->fillText(text, offset, common);
        }
        void strokeText(ID text, SkPoint offset, CommonIDs common, ID stroke) override {
            fWrapped->strokeText(text, offset, common, stroke);
        }

        // Maps const T* -> ID, and refs the key.
        template <typename T, Type kType>
        class RefKeyMap {
        public:
            RefKeyMap() {}
            ~RefKeyMap() { fMap.foreach([](const T* key, ID*) { SkSafeUnref(key); }); }

            void set(const T* key, ID id) {
                SkASSERT(id.type() == kType);
                fMap.set(SkSafeRef(key), id);
            }

            void remove(const T* key) {
                fMap.remove(key);
                SkSafeUnref(key);
            }

            const ID* find(const T* key) const {
                return fMap.find(key);
            }

            template <typename Fn>
            void foreach(const Fn& fn) {
                fMap.foreach(fn);
            }
        private:
            SkTHashMap<const T*, ID> fMap;
        };

        SkTHashMap<SkMatrix, ID>                        fMatrix;
        SkTHashMap<Misc, ID, MiscHash>                  fMisc;
        SkTHashMap<SkPath, ID>                          fPath;
        SkTHashMap<Stroke, ID>                          fStroke;
        RefKeyMap<const SkTextBlob, Type::kTextBlob   > fTextBlob;
        RefKeyMap<SkPathEffect    , Type::kPathEffect > fPathEffect;
        RefKeyMap<SkShader        , Type::kShader     > fShader;
        RefKeyMap<SkXfermode      , Type::kXfermode   > fXfermode;
        RefKeyMap<SkMaskFilter    , Type::kMaskFilter > fMaskFilter;
        RefKeyMap<SkColorFilter   , Type::kColorFilter> fColorFilter;
        RefKeyMap<SkRasterizer    , Type::kRasterizer > fRasterizer;
        RefKeyMap<SkDrawLooper    , Type::kDrawLooper > fDrawLooper;
        RefKeyMap<SkImageFilter   , Type::kImageFilter> fImageFilter;
        RefKeyMap<SkAnnotation    , Type::kAnnotation > fAnnotation;

        Encoder* fWrapped;
    };

    Encoder* NewCachingEncoder(Encoder* wrapped) { return new CachingEncoder(wrapped); }

} // namespace SkRemote
