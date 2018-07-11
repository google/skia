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

class SkArSessionPriv {
public:
    ~SkArSessionPriv();

    /**
     * @return immutable ARCore-backed Session contained within this SkArsession
     */
    const ArSession* getArSession() const;

    /**
     * @return ARCore-backed Session contained within this SkArsession
     */
    ArSession* getArSession();


private:
    explicit SkArSessionPriv(SkArSession* session) : fSkArSession(session) {}

    SkArSessionPriv(const SkArSessionPriv&); // unimpl
    SkArSessionPriv& operator=(const SkArSessionPriv&); // unimpl

    // No taking addresses of this type.
    const SkArSessionPriv* operator&() const;
    SkArSessionPriv* operator&();

    SkArSession* fSkArSession;

    friend class SkArSession; // To construct & copy this type
};

inline SkArSessionPriv SkArSession::priv() { return SkArSessionPriv(this); };

inline const SkArSessionPriv SkArSession::priv() const {
    return SkArSessionPriv (const_cast<SkArSession*>(this));
}

#endif  // SkArSessionPriv_DEFINED
