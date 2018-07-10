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

SkArSession* SkArSessionPriv::getSkArSession() {
    return fSkArSession.get();
}

SkArSessionPriv::~SkArSessionPriv() {}

sk_sp<SkArSessionPriv> SkArSessionPriv::Make(sk_sp<SkArSession> session) {
    return sk_sp<SkArSessionPriv>(new SkArSessionPriv(std::move(session)));
}

SkArSessionPriv::SkArSessionPriv(sk_sp<SkArSession> session) : fSkArSession(std::move(session)) {}