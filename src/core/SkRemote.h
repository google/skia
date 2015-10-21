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
#include "SkShader.h"
#include "SkTHash.h"
#include "SkTypes.h"

// TODO: document

namespace SkRemote {

    // General purpose identifier.  Holds a Type and a 56-bit value.
    class ID {
    public:
        ID() {}
        ID(Type type, uint64_t val) {
            fVal = (uint64_t)type << 56 | val;
            SkASSERT(this->type() == type && this->val() == val);
        }

        Type    type() const { return (Type)(fVal >> 56); }
        uint64_t val() const { return fVal & ~((uint64_t)0xFF << 56); }

        bool operator==(ID o) const { return fVal == o.fVal; }

    private:
        uint64_t fVal;
    };

    // Fields from SkPaint used by stroke, fill, and text draws.
    struct Misc {
        SkColor         fColor;
        SkFilterQuality fFilterQuality;
        bool fAntiAlias, fDither;

        static Misc CreateFrom(const SkPaint&);
        void applyTo(SkPaint*) const;
    };

    // Fields from SkPaint used by stroke draws only.
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

        static Encoder* CreateCachingEncoder(Encoder*);

        virtual ID define(const SkMatrix&) = 0;
        virtual ID define(const Misc&)     = 0;
        virtual ID define(const SkPath&)   = 0;
        virtual ID define(const Stroke&)   = 0;
        virtual ID define(SkShader*)       = 0;
        virtual ID define(SkXfermode*)     = 0;

        virtual void undefine(ID) = 0;

        virtual void    save() = 0;
        virtual void restore() = 0;

        virtual void setMatrix(ID matrix) = 0;

        // TODO: struct CommonIDs { ID misc, shader, xfermode; ... }
        // for IDs that affect both fill + stroke?

        virtual void   clipPath(ID path, SkRegion::Op, bool aa)                      = 0;
        virtual void   fillPath(ID path, ID misc, ID shader, ID xfermode)            = 0;
        virtual void strokePath(ID path, ID misc, ID shader, ID xfermode, ID stroke) = 0;
    };

    // An SkCanvas that translates to Encoder calls.
    class Client final : public SkCanvas {
    public:
        explicit Client(Encoder*);

    private:
        class AutoID;

        template <typename T>
        AutoID id(const T&);

        void   willSave() override;
        void didRestore() override;

        void    didConcat(const SkMatrix&) override;
        void didSetMatrix(const SkMatrix&) override;

        void onClipPath (const SkPath&,  SkRegion::Op, ClipEdgeStyle) override;
        void onClipRRect(const SkRRect&, SkRegion::Op, ClipEdgeStyle) override;
        void onClipRect (const SkRect&,  SkRegion::Op, ClipEdgeStyle) override;

        void onDrawOval(const SkRect&, const SkPaint&) override;
        void onDrawRRect(const SkRRect&, const SkPaint&) override;
        void onDrawDRRect(const SkRRect&, const SkRRect&, const SkPaint&) override;
        void onDrawPath(const SkPath&, const SkPaint&) override;
        void onDrawRect(const SkRect&, const SkPaint&) override;
        void onDrawPaint(const SkPaint&) override;

        void onDrawText(const void*, size_t, SkScalar,
                        SkScalar, const SkPaint&) override;
        void onDrawPosText(const void*, size_t, const SkPoint[],
                           const SkPaint&) override;
        void onDrawPosTextH(const void*, size_t, const SkScalar[], SkScalar,
                            const SkPaint&) override;

        Encoder* fEncoder;
    };

    // An Encoder that translates back to SkCanvas calls.
    class Server final : public Encoder {
    public:
        explicit Server(SkCanvas*);

    private:
        ID define(const SkMatrix&) override;
        ID define(const Misc&)     override;
        ID define(const SkPath&)   override;
        ID define(const Stroke&)   override;
        ID define(SkShader*)       override;
        ID define(SkXfermode*)     override;

        void undefine(ID) override;

        void    save() override;
        void restore() override;

        void setMatrix(ID matrix) override;

        void   clipPath(ID path, SkRegion::Op, bool aa)                      override;
        void   fillPath(ID path, ID misc, ID shader, ID xfermode)            override;
        void strokePath(ID path, ID misc, ID shader, ID xfermode, ID stroke) override;

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

        template <typename Map, typename T>
        ID define(Type, Map*, const T&);

        IDMap<SkMatrix, Type::kMatrix>           fMatrix;
        IDMap<Misc    , Type::kMisc  >           fMisc;
        IDMap<SkPath  , Type::kPath  >           fPath;
        IDMap<Stroke  , Type::kStroke>           fStroke;
        ReffedIDMap<SkShader,   Type::kShader>   fShader;
        ReffedIDMap<SkXfermode, Type::kXfermode> fXfermode;

        SkCanvas* fCanvas;
        uint64_t fNextID = 0;
    };

}  // namespace SkRemote

#endif//SkRemote_DEFINED
