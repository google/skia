/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRemotableFontMgr.h"

#include "SkLazyPtr.h"

SkRemotableFontIdentitySet::SkRemotableFontIdentitySet(int count, SkFontIdentity** data)
      : fCount(count), fData(count)
{
    SkASSERT(data);
    *data = fData;
}

// As a template argument, this must have external linkage.
SkRemotableFontIdentitySet* sk_remotable_font_identity_set_new() {
    return SkNEW(SkRemotableFontIdentitySet);
}

SK_DECLARE_STATIC_LAZY_PTR(SkRemotableFontIdentitySet, empty, sk_remotable_font_identity_set_new);
SkRemotableFontIdentitySet* SkRemotableFontIdentitySet::NewEmpty() {
    return SkRef(empty.get());
}
