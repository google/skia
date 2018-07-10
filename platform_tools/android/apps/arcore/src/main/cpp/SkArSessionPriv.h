/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkArSessionPriv_DEFINED
#define SkArSessionPriv_DEFINED

#include <SkRefCnt.h>
#include "SkArSession.h"

/**
 * Class that allows public access to some private methods of an SkArSession. Intended for use
 * internal to Skia only.
 */

class SkArSessionPriv : public SkRefCnt {
public:

    /**
     * Factory method used to create an object of this class. The parameter session is meant to be
     * wrapped by this class to access private methods.
     * @param session
     * @return
     */
    static sk_sp<SkArSessionPriv> Make(sk_sp<SkArSession> session);

    ~SkArSessionPriv();

    /**
     * @return current SkArSession
     */
    SkArSession* getSkArSession();

    /**
     * @return immutable ARCore-backed Session contained within this SkArsession
     */
    const ArSession* getArSession() const;

    /**
     * @return ARCore-backed Session contained within this SkArsession
     */
    ArSession* getArSession();


private:
    SkArSessionPriv(sk_sp<SkArSession> session);

    sk_sp<SkArSession> fSkArSession;

    typedef SkRefCnt INHERITED;
};

#endif  // SkArSessionPriv_DEFINED
