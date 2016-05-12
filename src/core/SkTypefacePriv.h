/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTypefacePriv_DEFINED
#define SkTypefacePriv_DEFINED

#include "SkTypeface.h"

/**
 *  Return a ref'd typeface, which must later be unref'd
 *
 *  If the parameter is non-null, it will be ref'd and returned, otherwise
 *  it will be the default typeface.
 */
static inline SkTypeface* ref_or_default(SkTypeface* face) {
    return face ? SkRef(face) : SkTypeface::RefDefault();
}

/**
 *  Always resolves to a non-null typeface, either the value passed to its
 *  constructor, or the default typeface if null was passed.
 */
class SkAutoResolveDefaultTypeface : public SkAutoTUnref<SkTypeface> {
public:
    SkAutoResolveDefaultTypeface() : INHERITED(SkTypeface::RefDefault()) {}

    SkAutoResolveDefaultTypeface(SkTypeface* face)
        : INHERITED(ref_or_default(face)) {}

private:
    typedef SkAutoTUnref<SkTypeface> INHERITED;
};

#endif
