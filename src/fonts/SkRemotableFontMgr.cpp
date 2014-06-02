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

SkRemotableFontIdentitySet* SkRemotableFontIdentitySet::NewEmptyImpl() {
    return SkNEW(SkRemotableFontIdentitySet);
}

SkRemotableFontIdentitySet* SkRemotableFontIdentitySet::NewEmpty() {
    SK_DECLARE_STATIC_LAZY_PTR(SkRemotableFontIdentitySet, empty, NewEmptyImpl);
    return SkRef(empty.get());
}
