
/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTraceMarker.h"
#include "GrTracing.h"
#include "SkString.h"
#include "SkTSort.h"

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

GrTraceMarkerSet::GrTraceMarkerSet(const GrTraceMarkerSet& other) {
   this->addSet(other);
}

void GrTraceMarkerSet::add(const GrGpuTraceMarker& marker) {
    this->fMarkerArray.push(marker);
}

void GrTraceMarkerSet::addSet(const GrTraceMarkerSet& markerSet) {
    for (Iter iter = markerSet.begin(); iter != markerSet.end(); ++iter) {
        this->add(*iter);
    }
}

void GrTraceMarkerSet::remove(const GrGpuTraceMarker& marker) {
    SkASSERT(-1 != fMarkerArray.find(marker));
    int index = this->fMarkerArray.find(marker);
    this->fMarkerArray.remove(index);
}

int GrTraceMarkerSet::count() const {
    return this->fMarkerArray.count();
}

SkString GrTraceMarkerSet::toStringLast() const {
    const int numMarkers = this->fMarkerArray.count();
    SkString marker_string;
    if (numMarkers > 0) {
        GrGpuTraceMarker& lastMarker = this->fMarkerArray[numMarkers - 1];
        marker_string.append(lastMarker.fMarker);
        if (lastMarker.fID != -1) {
            marker_string.append("(");
            marker_string.appendS32(lastMarker.fID);
            marker_string.append(")");
        }
    }
    return marker_string;
}

SkString GrTraceMarkerSet::toString() const {
    SkTQSort<GrGpuTraceMarker>(this->fMarkerArray.begin(), this->fMarkerArray.end() - 1);
    SkString marker_string;
    const char* prevMarkerName = "";
    int prevMarkerID = -1;
    int counter = 0;
    const int numMarkers = this->fMarkerArray.count();

    // check used for GrGLGpu device after we've already collapsed all markers
    if (1 == numMarkers && -1 == this->fMarkerArray[0].fID) {
        marker_string.append(this->fMarkerArray[0].fMarker);
        return marker_string;
    }

    for (int i = 0; i < numMarkers; ++i ) {
        GrGpuTraceMarker& currMarker = this->fMarkerArray[i];
        const char* currCmd = currMarker.fMarker;
        if (currCmd != prevMarkerName) {
            if (prevMarkerID != -1) {
                marker_string.append(") ");
            }
            marker_string.append(currCmd);
            if (currMarker.fID != -1) {
                marker_string.append("(");
                marker_string.appendS32(currMarker.fID);
            }
            prevMarkerName = currCmd;
        } else if (currMarker.fID != prevMarkerID) {
            marker_string.append(", ");
            marker_string.appendS32(currMarker.fID);
        }
        prevMarkerID = currMarker.fID;
        ++counter;
    }
    if (counter > 0 && prevMarkerID != -1) {
        marker_string.append(")");
    }
    return marker_string;
}

GrTraceMarkerSet::Iter GrTraceMarkerSet::begin() const {
    return Iter(this, 0);
}

GrTraceMarkerSet::Iter GrTraceMarkerSet::end() const {
    return Iter(this, this->fMarkerArray.count());
}

