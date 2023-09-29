/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkIDChangeListener.h"

#include "include/private/base/SkAssert.h"

#include <utility>
/**
 * Used to be notified when a gen/unique ID is invalidated, typically to preemptively purge
 * associated items from a cache that are no longer reachable. The listener can
 * be marked for deregistration if the cached item is remove before the listener is
 * triggered. This prevents unbounded listener growth when cache items are routinely
 * removed before the gen ID/unique ID is invalidated.
 */

SkIDChangeListener::SkIDChangeListener() : fShouldDeregister(false) {}

SkIDChangeListener::~SkIDChangeListener() = default;

using List = SkIDChangeListener::List;

List::List() = default;

List::~List() {
    // We don't need the mutex. No other thread should have this list while it's being
    // destroyed.
    for (auto& listener : fListeners) {
        if (!listener->shouldDeregister()) {
            listener->changed();
        }
    }
}

void List::add(sk_sp<SkIDChangeListener> listener) {
    if (!listener) {
        return;
    }
    SkASSERT(!listener->shouldDeregister());

    SkAutoMutexExclusive lock(fMutex);
    // Clean out any stale listeners before we append the new one.
    for (int i = 0; i < fListeners.size(); ++i) {
        if (fListeners[i]->shouldDeregister()) {
            fListeners.removeShuffle(i--);  // No need to preserve the order after i.
        }
    }
    fListeners.push_back(std::move(listener));
}

int List::count() const {
    SkAutoMutexExclusive lock(fMutex);
    return fListeners.size();
}

void List::changed() {
    SkAutoMutexExclusive lock(fMutex);
    for (auto& listener : fListeners) {
        if (!listener->shouldDeregister()) {
            listener->changed();
        }
    }
    fListeners.clear();
}

void List::reset() {
    SkAutoMutexExclusive lock(fMutex);
    fListeners.clear();
}
