/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRemote_DEFINED
#define SkRemote_DEFINED

#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkRemote_protocol.h"
#include "SkTHash.h"
#include "SkTypes.h"

// TODO: document

namespace SkRemote {
    // TODO: document
    struct Misc {
        SkColor         fColor;
        SkFilterQuality fFilterQuality;
        bool fAntiAlias, fDither;

        static Misc CreateFrom(const SkPaint&);
        void applyTo(SkPaint*) const;
    };

    // TODO: document
    struct Stroke {
        SkScalar fWidth, fMiter;
        SkPaint::Cap  fCap;
        SkPaint::Join fJoin;

        static Stroke CreateFrom(const SkPaint&);
        void applyTo(SkPaint*) const;
    };

    // TODO: document
    struct Encoder {
        virtual ~Encoder() {}

        virtual void define(ID, const SkMatrix&) = 0;
        virtual void define(ID, const Misc&)     = 0;
        virtual void define(ID, const SkPath&)   = 0;
        virtual void define(ID, const Stroke&)   = 0;

        virtual void undefine(ID) = 0;

        virtual void    save() = 0;
        virtual void restore() = 0;

        virtual void setMatrix(ID matrix) = 0;

        virtual void   clipPath(ID path, SkRegion::Op, bool aa) = 0;
        virtual void   fillPath(ID path, ID misc)               = 0;
        virtual void strokePath(ID path, ID misc, ID stroke)    = 0;
    };

    class LookupScope;

    // TODO: document
    struct Cache {
        virtual ~Cache() {}

        static Cache* CreateNeverCache();
        static Cache* CreateAlwaysCache();

        virtual bool lookup(const SkMatrix&, ID*, LookupScope*) = 0;
        virtual bool lookup(const Misc&,     ID*, LookupScope*) = 0;
        virtual bool lookup(const SkPath&,   ID*, LookupScope*) = 0;
        virtual bool lookup(const Stroke&,   ID*, LookupScope*) = 0;

        virtual void cleanup(Encoder*) = 0;
    };

    // TODO: document
    class Client final : public SkCanvas {
    public:
        Client(Cache*, Encoder*);
        ~Client();

    private:
        void   willSave() override;
        void didRestore() override;

        void    didConcat(const SkMatrix&) override;
        void didSetMatrix(const SkMatrix&) override;

        void onClipPath (const SkPath&,  SkRegion::Op, ClipEdgeStyle) override;
        void onClipRRect(const SkRRect&, SkRegion::Op, ClipEdgeStyle) override;
        void onClipRect (const SkRect&,  SkRegion::Op, ClipEdgeStyle) override;

        void onDrawOval(const SkRect&, const SkPaint&) override;
        void onDrawPath(const SkPath&, const SkPaint&) override;
        void onDrawRect(const SkRect&, const SkPaint&) override;

        Cache*   fCache;
        Encoder* fEncoder;
    };

    // TODO: document
    class Server final : public Encoder {
    public:
        explicit Server(SkCanvas*);

    private:
        void define(ID, const SkMatrix&) override;
        void define(ID, const Misc&)     override;
        void define(ID, const SkPath&)   override;
        void define(ID, const Stroke&)   override;

        void undefine(ID) override;

        void    save() override;
        void restore() override;

        void setMatrix(ID matrix) override;

        void   clipPath(ID path, SkRegion::Op, bool aa) override;
        void   fillPath(ID path, ID misc) override;
        void strokePath(ID path, ID misc, ID stroke) override;

        template <typename T, Type kType>
        class IDMap {
        public:
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

        IDMap<SkMatrix, Type::kMatrix> fMatrix;
        IDMap<Misc    , Type::kMisc  > fMisc;
        IDMap<SkPath  , Type::kPath  > fPath;
        IDMap<Stroke  , Type::kStroke> fStroke;

        SkCanvas* fCanvas;
    };

}  // namespace SkRemote

#endif//SkRemote_DEFINED
