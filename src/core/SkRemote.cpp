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

    class LookupScope {
    public:
        LookupScope(Cache* cache, Encoder* encoder) : fCache(cache), fEncoder(encoder) {}
        ~LookupScope() { for (ID id : fToUndefine) { fEncoder->undefine(id); } }
        void undefineWhenDone(ID id) { fToUndefine.push_back(id); }

        template <typename T>
        ID lookup(const T& val) {
            ID id;
            if (!fCache->lookup(val, &id, this)) {
                fEncoder->define(id, val);
            }
            return id;
        }

    private:
        Cache*   fCache;
        Encoder* fEncoder;
        SkSTArray<4, ID> fToUndefine;
    };


    Cache* Cache::CreateNeverCache() {
        struct NeverCache final : public Cache {
            NeverCache()
                : fNextMatrix  (Type::kMatrix)
                , fNextMisc    (Type::kMisc)
                , fNextPath    (Type::kPath)
                , fNextStroke  (Type::kStroke)
                , fNextShader  (Type::kShader)
                , fNextXfermode(Type::kXfermode)
            {}
            void cleanup(Encoder*) override {}

            static bool Helper(ID* next, ID* id, LookupScope* ls) {
                *id = ++(*next);
                ls->undefineWhenDone(*id);
                return false;
            }

            bool lookup(const SkMatrix&, ID* id, LookupScope* ls) override {
                return Helper(&fNextMatrix, id, ls);
            }
            bool lookup(const Misc&, ID* id, LookupScope* ls) override {
                return Helper(&fNextMisc, id, ls);
            }
            bool lookup(const SkPath&, ID* id, LookupScope* ls) override {
                return Helper(&fNextPath, id, ls);
            }
            bool lookup(const Stroke&, ID* id, LookupScope* ls) override {
                return Helper(&fNextStroke, id, ls);
            }
            bool lookup(const SkShader* shader, ID* id, LookupScope* ls) override {
                if (!shader) {
                    *id = ID(Type::kShader);
                    return true;  // Null IDs are always defined.
                }
                return Helper(&fNextShader, id, ls);
            }
            bool lookup(const SkXfermode* xfermode, ID* id, LookupScope* ls) override {
                if (!xfermode) {
                    *id = ID(Type::kXfermode);
                    return true;  // Null IDs are always defined.
                }
                return Helper(&fNextXfermode, id, ls);
            }

            ID fNextMatrix,
               fNextMisc,
               fNextPath,
               fNextStroke,
               fNextShader,
               fNextXfermode;
        };
        return new NeverCache;
    }

    // These can't be declared locally inside AlwaysCache because of the templating.  :(
    namespace {
        template <typename T, typename Map>
        static bool always_cache_lookup(const T& val, Map* map, ID* next, ID* id) {
            if (const ID* found = map->find(val)) {
                *id = *found;
                return true;
            }
            *id = ++(*next);
            map->set(val, *id);
            return false;
        }

        struct Undefiner {
            Encoder* fEncoder;

            template <typename T>
            void operator()(const T&, ID* id) const { fEncoder->undefine(*id); }
        };

        // Maps const T* -> ID, and refs the key.  nullptr always maps to ID(kType).
        template <typename T, Type kType>
        class RefKeyMap {
        public:
            RefKeyMap() {}
            ~RefKeyMap() { fMap.foreach([](const T* key, ID*) { key->unref(); }); }

            void set(const T* key, const ID& id) {
                SkASSERT(key && id.type() == kType);
                fMap.set(SkRef(key), id);
            }

            void remove(const T* key) {
                SkASSERT(key);
                fMap.remove(key);
                key->unref();
            }

            const ID* find(const T* key) const {
                static const ID nullID(kType);
                return key ? fMap.find(key) : &nullID;
            }

            template <typename Fn>
            void foreach(const Fn& fn) { fMap.foreach(fn); }
        private:
            SkTHashMap<const T*, ID> fMap;
        };
    } // namespace

    Cache* Cache::CreateAlwaysCache() {
        struct AlwaysCache final : public Cache {
            AlwaysCache()
                : fNextMatrix  (Type::kMatrix)
                , fNextMisc    (Type::kMisc)
                , fNextPath    (Type::kPath)
                , fNextStroke  (Type::kStroke)
                , fNextShader  (Type::kShader)
                , fNextXfermode(Type::kXfermode)
            {}

            void cleanup(Encoder* encoder) override {
                Undefiner undef{encoder};
                fMatrix  .foreach(undef);
                fMisc    .foreach(undef);
                fPath    .foreach(undef);
                fStroke  .foreach(undef);
                fShader  .foreach(undef);
                fXfermode.foreach(undef);
            }


            bool lookup(const SkMatrix& matrix, ID* id, LookupScope*) override {
                return always_cache_lookup(matrix, &fMatrix, &fNextMatrix, id);
            }
            bool lookup(const Misc& misc, ID* id, LookupScope*) override {
                return always_cache_lookup(misc, &fMisc, &fNextMisc, id);
            }
            bool lookup(const SkPath& path, ID* id, LookupScope*) override {
                return always_cache_lookup(path, &fPath, &fNextPath, id);
            }
            bool lookup(const Stroke& stroke, ID* id, LookupScope*) override {
                return always_cache_lookup(stroke, &fStroke, &fNextStroke, id);
            }
            bool lookup(const SkShader* shader, ID* id, LookupScope*) override {
                return always_cache_lookup(shader, &fShader, &fNextShader, id);
            }
            bool lookup(const SkXfermode* xfermode, ID* id, LookupScope*) override {
                return always_cache_lookup(xfermode, &fXfermode, &fNextXfermode, id);
            }

            SkTHashMap<SkMatrix, ID>               fMatrix;
            SkTHashMap<Misc,     ID, MiscHash>     fMisc;
            SkTHashMap<SkPath,   ID>               fPath;
            SkTHashMap<Stroke,   ID>               fStroke;
            RefKeyMap<SkShader,   Type::kShader>   fShader;
            RefKeyMap<SkXfermode, Type::kXfermode> fXfermode;

            ID fNextMatrix,
               fNextMisc,
               fNextPath,
               fNextStroke,
               fNextShader,
               fNextXfermode;
        };
        return new AlwaysCache;
    }

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

    Client::Client(Cache* cache, Encoder* encoder)
        : SkCanvas(1,1)
        , fCache(cache)
        , fEncoder(encoder)
    {}

    Client::~Client() {
        fCache->cleanup(fEncoder);
    }

    void Client::willSave()   { fEncoder->save(); }
    void Client::didRestore() { fEncoder->restore(); }

    void Client::didConcat   (const SkMatrix&) { this->didSetMatrix(this->getTotalMatrix()); }
    void Client::didSetMatrix(const SkMatrix& matrix) {
        LookupScope ls(fCache, fEncoder);
        fEncoder->setMatrix(ls.lookup(matrix));
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
        LookupScope ls(fCache, fEncoder);
        ID p = ls.lookup(path),
           m = ls.lookup(Misc::CreateFrom(paint)),
           s = ls.lookup(paint.getShader()),
           x = ls.lookup(paint.getXfermode());

        if (paint.getStyle() == SkPaint::kFill_Style) {
            fEncoder->fillPath(p, m, s, x);
        } else {
            // TODO: handle kStrokeAndFill_Style
            fEncoder->strokePath(p, m, s, x, ls.lookup(Stroke::CreateFrom(paint)));
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
        LookupScope ls(fCache, fEncoder);
        fEncoder->clipPath(ls.lookup(path), op, edgeStyle == kSoft_ClipEdgeStyle);
    }

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

    Server::Server(SkCanvas* canvas) : fCanvas(canvas) {}

    void Server::define(ID id, const SkMatrix& v) { fMatrix  .set(id, v); }
    void Server::define(ID id, const Misc&     v) { fMisc    .set(id, v); }
    void Server::define(ID id, const SkPath&   v) { fPath    .set(id, v); }
    void Server::define(ID id, const Stroke&   v) { fStroke  .set(id, v); }
    void Server::define(ID id, SkShader*       v) { fShader  .set(id, v); }
    void Server::define(ID id, SkXfermode*     v) { fXfermode.set(id, v); }

    void Server::undefine(ID id) {
        switch(id.type()) {
            case Type::kMatrix:   return fMatrix  .remove(id);
            case Type::kMisc:     return fMisc    .remove(id);
            case Type::kPath:     return fPath    .remove(id);
            case Type::kStroke:   return fStroke  .remove(id);
            case Type::kShader:   return fShader  .remove(id);
            case Type::kXfermode: return fXfermode.remove(id);

            case Type::kNone: SkASSERT(false);
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
