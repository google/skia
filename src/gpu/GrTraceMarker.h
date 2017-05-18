/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTDArray.h"

#ifndef GrTraceMarkerSet_DEFINED
#define GrTraceMarkerSet_DEFINED

class GrGpuTraceMarker {
public:
    GrGpuTraceMarker(const char* marker, int idCounter) : fMarker(marker), fID(idCounter) {}

    bool operator<(const GrGpuTraceMarker& rhs) const {
        return this->fMarker < rhs.fMarker || (this->fMarker == rhs.fMarker && this->fID < rhs.fID);
    }

    bool operator==(const GrGpuTraceMarker& rhs) const {
        return (this->fID == rhs.fID && this->fMarker == rhs.fMarker);
    }

    const char* fMarker;
    int fID;
};

///////////////////////////////////////////////////////////////////////////////

class SkString;

class GrTraceMarkerSet {
public:
    GrTraceMarkerSet() {}

    GrTraceMarkerSet(const GrTraceMarkerSet& other);

    // Adds marker to the set.
    void add(const GrGpuTraceMarker& marker);
    // Adds all markers from one set into this set.
    void addSet(const GrTraceMarkerSet& markerSet);

    void remove(const GrGpuTraceMarker& marker);

    int count() const;

    /**
     * First sorts fMarkerArray and returns a string of the format
     * MarkerName1(#,#,...)%MarkerName2(#,#,...):... where MarkerName is the
     * marker string used in the TraceMarker and the (#,#,..) is a list of instance
     * id's for the the given marker string
     */
    SkString toString() const;

    SkString toStringLast() const;

    class Iter;

    Iter begin() const;

    Iter end() const;

private:
    mutable SkTDArray<GrGpuTraceMarker> fMarkerArray;
};

class GrTraceMarkerSet::Iter {
public:
    Iter() {}
    Iter& operator=(const Iter& i) {
        fCurrentIndex = i.fCurrentIndex;
        fMarkers = i.fMarkers;
        return *this;
    }
    bool operator==(const Iter& i) const {
        return fCurrentIndex == i.fCurrentIndex && fMarkers == i.fMarkers;
    }
    bool operator!=(const Iter& i) const { return !(*this == i); }
    const GrGpuTraceMarker& operator*() const { return fMarkers->fMarkerArray[fCurrentIndex]; }
    Iter& operator++() {
        SkASSERT(*this != fMarkers->end());
        ++fCurrentIndex;
        return *this;
    }

private:
    friend class GrTraceMarkerSet;
    Iter(const GrTraceMarkerSet* markers, int index)
            : fMarkers(markers), fCurrentIndex(index) {
        SkASSERT(markers);
    }

    const GrTraceMarkerSet* fMarkers;
    int fCurrentIndex;
};

#endif
