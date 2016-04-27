/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrShape.h"

GrShape& GrShape::operator=(const GrShape& that) {
    bool wasPath = Type::kPath == fType;
    fStyle = that.fStyle;
    fType = that.fType;
    switch (fType) {
        case Type::kEmpty:
            if (wasPath) {
                fPath.reset();
            }
            break;
        case Type::kRRect:
            if (wasPath) {
                fPath.reset();
            }
            fRRect = that.fRRect;
            break;
        case Type::kPath:
            if (wasPath) {
                *fPath.get() = *that.fPath.get();
            } else {
                fPath.set(*that.fPath.get());
            }
            break;
    }
    fInheritedKey.reset(that.fInheritedKey.count());
    memcpy(fInheritedKey.get(), that.fInheritedKey.get(),
           sizeof(uint32_t) * fInheritedKey.count());
    return *this;
}

int GrShape::unstyledKeySize() const {
    if (fInheritedKey.count()) {
        return fInheritedKey.count();
    }
    switch (fType) {
        case Type::kEmpty:
            return 1;
        case Type::kRRect:
            SkASSERT(!fInheritedKey.count());
            SkASSERT(0 == SkRRect::kSizeInMemory % sizeof(uint32_t));
            return SkRRect::kSizeInMemory / sizeof(uint32_t);
        case Type::kPath:
            if (fPath.get()->isVolatile()) {
                return -1;
            } else {
                return 1;
            }
    }
    SkFAIL("Should never get here.");
    return 0;
}

void GrShape::writeUnstyledKey(uint32_t* key) const {
    SkASSERT(this->unstyledKeySize());
    SkDEBUGCODE(uint32_t* origKey = key;)
    if (fInheritedKey.count()) {
        memcpy(key, fInheritedKey.get(), sizeof(uint32_t) * fInheritedKey.count());
        SkDEBUGCODE(key += fInheritedKey.count();)
    } else {
        switch (fType) {
            case Type::kEmpty:
                *key++ = 1;
                break;
            case Type::kRRect:
                fRRect.writeToMemory(key);
                key += SkRRect::kSizeInMemory / sizeof(uint32_t);
                break;
            case Type::kPath:
                SkASSERT(!fPath.get()->isVolatile());
                *key++ = fPath.get()->getGenerationID();
                break;
        }
    }
    SkASSERT(key - origKey == this->unstyledKeySize());
}

int GrShape::StyleKeySize(const GrStyle& style, bool stopAfterPE) {
    GR_STATIC_ASSERT(sizeof(uint32_t) == sizeof(SkScalar));
    int size = 0;
    if (style.isDashed()) {
        // One scalar for dash phase and one for each dash value.
        size += 1 + style.dashIntervalCnt();
    } else if (style.pathEffect()) {
        // No key for a generic path effect.
        return -1;
    }

    if (stopAfterPE) {
        return size;
    }

    if (style.strokeRec().needToApply()) {
        // One for style/cap/join, 2 for miter and width.
        size += 3;
    }
    return size;
}

void GrShape::StyleKey(uint32_t* key, const GrStyle& style, bool stopAfterPE) {
    SkASSERT(key);
    SkASSERT(StyleKeySize(style, stopAfterPE) >= 0);
    GR_STATIC_ASSERT(sizeof(uint32_t) == sizeof(SkScalar));

    int i = 0;
    if (style.isDashed()) {
        GR_STATIC_ASSERT(sizeof(style.dashPhase()) == sizeof(uint32_t));
        SkScalar phase = style.dashPhase();
        memcpy(&key[i++], &phase, sizeof(SkScalar));

        int32_t count = style.dashIntervalCnt();
        // Dash count should always be even.
        SkASSERT(0 == (count & 0x1));
        const SkScalar* intervals = style.dashIntervals();
        int intervalByteCnt = count * sizeof(SkScalar);
        memcpy(&key[i], intervals, intervalByteCnt);
        i += count;
    } else {
        SkASSERT(!style.pathEffect());
    }

    if (!stopAfterPE && style.strokeRec().needToApply()) {
        enum {
            kStyleBits = 2,
            kJoinBits = 2,
            kCapBits = 32 - kStyleBits - kJoinBits,

            kJoinShift = kStyleBits,
            kCapShift = kJoinShift + kJoinBits,
        };
        GR_STATIC_ASSERT(SkStrokeRec::kStyleCount <= (1 << kStyleBits));
        GR_STATIC_ASSERT(SkPaint::kJoinCount <= (1 << kJoinBits));
        GR_STATIC_ASSERT(SkPaint::kCapCount <= (1 << kCapBits));
        key[i++]  = style.strokeRec().getStyle() |
                    style.strokeRec().getJoin() << kJoinShift |
                    style.strokeRec().getCap() << kCapShift;

        SkScalar scalar;
        // Miter limit only affects miter joins
        scalar = SkPaint::kMiter_Join == style.strokeRec().getJoin()
                 ? style.strokeRec().getMiter()
                 : -1.f;
        memcpy(&key[i++], &scalar, sizeof(scalar));

        scalar = style.strokeRec().getWidth();
        memcpy(&key[i++], &scalar, sizeof(scalar));
    }
    SkASSERT(StyleKeySize(style, stopAfterPE) == i);
}

void GrShape::setInheritedKey(const GrShape &parent, bool stopAfterPE) {
    SkASSERT(!fInheritedKey.count());
    // If the output shape turns out to be simple, then we will just use its geometric key
    if (Type::kPath == fType) {
        // We want ApplyFullStyle(ApplyPathEffect(shape)) to have the same key as
        // ApplyFullStyle(shape).
        // The full key is structured as (geo,path_effect,stroke).
        // If we do ApplyPathEffect we get get,path_effect as the inherited key. If we then
        // do ApplyFullStyle we'll memcpy geo,path_effect into the new inherited key
        // and then append the style key (which should now be stroke only) at the end.
        int parentCnt = parent.fInheritedKey.count();
        bool useParentGeoKey = !parentCnt;
        if (useParentGeoKey) {
            parentCnt = parent.unstyledKeySize();
            if (parentCnt < 0) {
                // The parent's geometry has no key so we will have no key.
                fPath.get()->setIsVolatile(true);
                return;
            }
        }
        int styleCnt = StyleKeySize(parent.fStyle, stopAfterPE);
        if (styleCnt < 0) {
            // The style doesn't allow a key, set the path to volatile so that we fail when
            // we try to get a key for the shape.
            fPath.get()->setIsVolatile(true);
            return;
        }
        fInheritedKey.reset(parentCnt + styleCnt);
        if (useParentGeoKey) {
            // This will be the geo key.
            parent.writeUnstyledKey(fInheritedKey.get());
        } else {
            // This should be (geo,path_effect).
            memcpy(fInheritedKey.get(), parent.fInheritedKey.get(),
                   parentCnt * sizeof(uint32_t));
        }
        // Now turn (geo,path_effect) or (geo) into (geo,path_effect,stroke)
        StyleKey(fInheritedKey.get() + parentCnt, parent.fStyle, stopAfterPE);
    }
}

GrShape::GrShape(const GrShape& that) : fType(that.fType), fStyle(that.fStyle) {
    switch (fType) {
        case Type::kEmpty:
            return;
        case Type::kRRect:
            fRRect = that.fRRect;
            return;
        case Type::kPath:
            fPath.set(*that.fPath.get());
            return;
    }
    fInheritedKey.reset(that.fInheritedKey.count());
    memcpy(fInheritedKey.get(), that.fInheritedKey.get(),
           sizeof(uint32_t) * fInheritedKey.count());
}

GrShape::GrShape(const GrShape& parent, bool stopAfterPE) {
    fType = Type::kEmpty;
    SkPathEffect* pe = parent.fStyle.pathEffect();
    const SkPath* inPath;
    SkStrokeRec strokeRec = parent.fStyle.strokeRec();
    bool appliedPE = false;
    if (pe) {
        fType = Type::kPath;
        fPath.init();
        if (parent.fType == Type::kPath) {
            inPath = parent.fPath.get();
        } else {
            inPath = fPath.get();
            parent.asPath(fPath.get());
        }
        // Should we consider bounds? Would have to include in key, but it'd be nice to know
        // if the bounds actually modified anything before including in key.
        if (!pe->filterPath(fPath.get(), *inPath, &strokeRec, nullptr)) {
            // Make an empty unstyled shape if filtering fails.
            fType = Type::kEmpty;
            fStyle = GrStyle();
            fPath.reset();
            return;
        }
        appliedPE = true;
        inPath = fPath.get();
    } else if (stopAfterPE || !strokeRec.needToApply()) {
        *this = parent;
        return;
    } else {
        fType = Type::kPath;
        fPath.init();
        if (parent.fType == Type::kPath) {
            inPath = parent.fPath.get();
        } else {
            inPath = fPath.get();
            parent.asPath(fPath.get());
        }
    }
    const GrShape* effectiveParent = &parent;
    SkTLazy<GrShape> tmpParent;
    if (!stopAfterPE) {
        if (appliedPE) {
            // If the intermediate shape from just the PE is not a path then we capture that here
            // so that we can pass the non-path parent to setInheritedKey.
            SkRRect rrect;
            Type parentType = AttemptToReduceFromPathImpl(*fPath.get(), &rrect, nullptr, strokeRec);
            switch (parentType) {
                case Type::kEmpty:
                    tmpParent.init();
                    effectiveParent = tmpParent.get();
                    break;
                case Type::kRRect:
                    tmpParent.init(rrect, GrStyle(strokeRec, nullptr));
                    effectiveParent = tmpParent.get();
                case Type::kPath:
                    break;
            }
        }
        strokeRec.applyToPath(fPath.get(), *inPath);
    } else {
        fStyle = GrStyle(strokeRec, nullptr);
    }
    this->attemptToReduceFromPath();
    this->setInheritedKey(*effectiveParent, stopAfterPE);
}
