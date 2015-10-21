/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPath.h"
#include "SkRect.h"
#include "SkRemote.h"

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
            fMatrix  .foreach(undef);
            fMisc    .foreach(undef);
            fPath    .foreach(undef);
            fStroke  .foreach(undef);
            fShader  .foreach(undef);
            fXfermode.foreach(undef);
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

        ID define(const SkMatrix& v) override { return this->define(&fMatrix,   v); }
        ID define(const Misc&     v) override { return this->define(&fMisc,     v); }
        ID define(const SkPath&   v) override { return this->define(&fPath,     v); }
        ID define(const Stroke&   v) override { return this->define(&fStroke,   v); }
        ID define(SkShader*       v) override { return this->define(&fShader,   v); }
        ID define(SkXfermode*     v) override { return this->define(&fXfermode, v); }

        void undefine(ID) override {}

        void    save() override { fWrapped->   save(); }
        void restore() override { fWrapped->restore(); }

        void setMatrix(ID matrix) override { fWrapped->setMatrix(matrix); }

        void clipPath(ID path, SkRegion::Op op, bool aa) override {
            fWrapped->clipPath(path, op, aa);
        }
        void fillPath(ID path, ID misc, ID shader, ID xfermode) override {
            fWrapped->fillPath(path, misc, shader, xfermode);
        }
        void strokePath(ID path, ID misc, ID shader, ID xfermode, ID stroke) override {
            fWrapped->strokePath(path, misc, shader, xfermode, stroke);
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

        SkTHashMap<SkMatrix, ID>               fMatrix;
        SkTHashMap<Misc, ID, MiscHash>         fMisc;
        SkTHashMap<SkPath, ID>                 fPath;
        SkTHashMap<Stroke, ID>                 fStroke;
        RefKeyMap<SkShader, Type::kShader>     fShader;
        RefKeyMap<SkXfermode, Type::kXfermode> fXfermode;

        Encoder* fWrapped;
    };

    Encoder* Encoder::CreateCachingEncoder(Encoder* wrapped) { return new CachingEncoder(wrapped); }

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

    // Calls Encoder::define() when created, Encoder::undefine() when destroyed.
    class Client::AutoID : ::SkNoncopyable {
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

    template <typename T>
    Client::AutoID Client::id(const T& val) { return AutoID(fEncoder, val); }

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

    Client::Client(Encoder* encoder)
        : SkCanvas(1,1)
        , fEncoder(encoder)
    {}

    void Client::willSave()   { fEncoder->save(); }
    void Client::didRestore() { fEncoder->restore(); }

    void Client::didConcat   (const SkMatrix&) { this->didSetMatrix(this->getTotalMatrix()); }
    void Client::didSetMatrix(const SkMatrix& matrix) {
        fEncoder->setMatrix(this->id(matrix));
    }

    void Client::onDrawOval(const SkRect& oval, const SkPaint& paint) {
        SkPath path;
        path.addOval(oval);
        this->onDrawPath(path, paint);
    }

    void Client::onDrawRect(const SkRect& rect, const SkPaint& paint) {
        SkPath path;
        path.addRect(rect);
        this->onDrawPath(path, paint);
    }

    void Client::onDrawRRect(const SkRRect& rrect, const SkPaint& paint) {
        SkPath path;
        path.addRRect(rrect);
        this->onDrawPath(path, paint);
    }

    void Client::onDrawDRRect(const SkRRect& outside,
                              const SkRRect& inside,
                              const SkPaint& paint) {
        SkPath path;
        path.addRRect(outside);
        path.addRRect(inside, SkPath::kCCW_Direction);
        this->onDrawPath(path, paint);
    }

    void Client::onDrawPath(const SkPath& path, const SkPaint& paint) {
        auto p = this->id(path),
             m = this->id(Misc::CreateFrom(paint)),
             s = this->id(paint.getShader()),
             x = this->id(paint.getXfermode());

        if (paint.getStyle() == SkPaint::kFill_Style) {
            fEncoder->fillPath(p, m, s, x);
        } else {
            // TODO: handle kStrokeAndFill_Style
            fEncoder->strokePath(p, m, s, x, this->id(Stroke::CreateFrom(paint)));
        }
    }

    void Client::onDrawPaint(const SkPaint& paint) {
        SkPath path;
        path.setFillType(SkPath::kInverseWinding_FillType);  // Either inverse FillType works fine.
        this->onDrawPath(path, paint);
    }

    void Client::onDrawText(const void* text, size_t byteLength, SkScalar x,
                            SkScalar y, const SkPaint& paint) {
        // Text-as-paths is a temporary hack.
        // TODO: send SkTextBlobs and SkTypefaces
        SkPath path;
        paint.getTextPath(text, byteLength, x, y, &path);
        this->onDrawPath(path, paint);
    }

    void Client::onDrawPosText(const void* text, size_t byteLength,
                               const SkPoint pos[], const SkPaint& paint) {
        // Text-as-paths is a temporary hack.
        // TODO: send SkTextBlobs and SkTypefaces
        SkPath path;
        paint.getPosTextPath(text, byteLength, pos, &path);
        this->onDrawPath(path, paint);
    }

    void Client::onDrawPosTextH(const void* text, size_t byteLength,
                                const SkScalar xpos[], SkScalar constY,
                                const SkPaint& paint) {
        size_t length = paint.countText(text, byteLength);
        SkAutoTArray<SkPoint> pos(length);
        for(size_t i = 0; i < length; ++i) {
            pos[i].set(xpos[i], constY);
        }
        this->onDrawPosText(text, byteLength, &pos[0], paint);
    }

    void Client::onClipRect(const SkRect& rect, SkRegion::Op op, ClipEdgeStyle edgeStyle) {
        SkPath path;
        path.addRect(rect);
        this->onClipPath(path, op, edgeStyle);
    }

    void Client::onClipRRect(const SkRRect& rrect, SkRegion::Op op, ClipEdgeStyle edgeStyle) {
        SkPath path;
        path.addRRect(rrect);
        this->onClipPath(path, op, edgeStyle);
    }

    void Client::onClipPath(const SkPath& path, SkRegion::Op op, ClipEdgeStyle edgeStyle) {
        fEncoder->clipPath(this->id(path), op, edgeStyle == kSoft_ClipEdgeStyle);
    }

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

    Server::Server(SkCanvas* canvas) : fCanvas(canvas) {}

    template <typename Map, typename T>
    ID Server::define(Type type, Map* map, const T& val) {
        ID id(type, fNextID++);
        map->set(id, val);
        return id;
    }

    ID Server::define(const SkMatrix& v) { return this->define(Type::kMatrix,   &fMatrix,   v); }
    ID Server::define(const Misc&     v) { return this->define(Type::kMisc,     &fMisc,     v); }
    ID Server::define(const SkPath&   v) { return this->define(Type::kPath,     &fPath,     v); }
    ID Server::define(const Stroke&   v) { return this->define(Type::kStroke,   &fStroke,   v); }
    ID Server::define(SkShader*       v) { return this->define(Type::kShader,   &fShader,   v); }
    ID Server::define(SkXfermode*     v) { return this->define(Type::kXfermode, &fXfermode, v); }

    void Server::undefine(ID id) {
        switch(id.type()) {
            case Type::kMatrix:   return fMatrix  .remove(id);
            case Type::kMisc:     return fMisc    .remove(id);
            case Type::kPath:     return fPath    .remove(id);
            case Type::kStroke:   return fStroke  .remove(id);
            case Type::kShader:   return fShader  .remove(id);
            case Type::kXfermode: return fXfermode.remove(id);
        };
    }

    void Server::   save() { fCanvas->save(); }
    void Server::restore() { fCanvas->restore(); }

    void Server::setMatrix(ID matrix) { fCanvas->setMatrix(fMatrix.find(matrix)); }

    void Server::clipPath(ID path, SkRegion::Op op, bool aa) {
        fCanvas->clipPath(fPath.find(path), op, aa);
    }
    void Server::fillPath(ID path, ID misc, ID shader, ID xfermode) {
        SkPaint paint;
        paint.setStyle(SkPaint::kFill_Style);
        fMisc.find(misc).applyTo(&paint);
        paint.setShader  (fShader  .find(shader));
        paint.setXfermode(fXfermode.find(xfermode));
        fCanvas->drawPath(fPath.find(path), paint);
    }
    void Server::strokePath(ID path, ID misc, ID shader, ID xfermode, ID stroke) {
        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);
        fMisc  .find(misc  ).applyTo(&paint);
        fStroke.find(stroke).applyTo(&paint);
        paint.setShader  (fShader  .find(shader));
        paint.setXfermode(fXfermode.find(xfermode));
        fCanvas->drawPath(fPath.find(path), paint);
    }

} // namespace SkRemote
