/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOncePtr.h"
#include "SkRemotableFontMgr.h"

SkRemotableFontIdentitySet::SkRemotableFontIdentitySet(int count, SkFontIdentity** data)
      : fCount(count), fData(count)
{
    SkASSERT(data);
    *data = fData;
}

SK_DECLARE_STATIC_ONCE_PTR(SkRemotableFontIdentitySet, empty);
SkRemotableFontIdentitySet* SkRemotableFontIdentitySet::NewEmpty() {
    return SkRef(empty.get([]{ return new SkRemotableFontIdentitySet; }));
}
