/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRemotableFontMgr.h"

#include "SkOnce.h"

SkRemotableFontIdentitySet::SkRemotableFontIdentitySet(int count, SkFontIdentity** data)
      : fCount(count), fData(count)
{
    SkASSERT(data);
    *data = fData;
}

static SkRemotableFontIdentitySet* gEmptyRemotableFontIdentitySet = NULL;
static void cleanup_gEmptyRemotableFontIdentitySet() { gEmptyRemotableFontIdentitySet->unref(); }

void SkRemotableFontIdentitySet::NewEmptyImpl(int) {
    gEmptyRemotableFontIdentitySet = new SkRemotableFontIdentitySet();
}

SkRemotableFontIdentitySet* SkRemotableFontIdentitySet::NewEmpty() {
    SK_DECLARE_STATIC_ONCE(once);
    SkOnce(&once, SkRemotableFontIdentitySet::NewEmptyImpl, 0,
           cleanup_gEmptyRemotableFontIdentitySet);
    gEmptyRemotableFontIdentitySet->ref();
    return gEmptyRemotableFontIdentitySet;
}
