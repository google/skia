/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkArSessionPriv.h"

ArSession* SkArSessionPriv::getArSession() {
    return fSkArSession->getArSession();
}

const ArSession* SkArSessionPriv::getArSession() const {
    return fSkArSession->getArSession();
}

SkArSessionPriv::~SkArSessionPriv() {}
