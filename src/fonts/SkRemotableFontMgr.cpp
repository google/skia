/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/ports/SkRemotableFontMgr.h"
#include "include/private/SkOnce.h"

SkRemotableFontIdentitySet::SkRemotableFontIdentitySet(int count, SkFontIdentity** data)
      : fCount(count), fData(count)
{
    SkASSERT(data);
    *data = fData;
}

SkRemotableFontIdentitySet* SkRemotableFontIdentitySet::NewEmpty() {
    static SkOnce once;
    static SkRemotableFontIdentitySet* empty;
    once([]{ empty = new SkRemotableFontIdentitySet; });
    return SkRef(empty);
}
