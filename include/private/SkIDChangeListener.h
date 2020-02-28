/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGenIDChangeListener_DEFINED
#define SkGenIDChangeListener_DEFINED

/**
 * Used to be notified when a gen ID changes/expires or a unique ID expires, typically to
 * preemptively purge items from a cache that are no longer reachable.
 */
class SkIDChangeListener : public SkRefCnt {
public:
    SkIDChangeListener() = default;

    //virtual ~SkIDChangeListener() = default;

    virtual void onChange() = 0;

    // The caller can use this method to notify it no longer needs to listen. Once
    // called, the object that would call onChange should remove the listener from
    // its set. This is used to prevent unbounded growth of listeners when items
    // that would be listening have been purged before the ID changes.
    void markShouldUnregister() {
        fShouldUnregister.store(true, std::memory_order_relaxed);
    }
    bool shouldUnregister() {
        return fShouldUnregister.load(std::memory_order_acquire);
    }

private:
    std::atomic<bool> fShouldUnregister = false;
};

#endif
