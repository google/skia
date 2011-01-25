/*
    Copyright 2010 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */


#ifndef GrPath_DEFINED
#define GrPath_DEFINED

#include "GrPathSink.h"
#include "GrPathIter.h"
#include "GrTDArray.h"
#include "GrPoint.h"

class GrPath : public GrPathSink {
public:
    GrPath();
    GrPath(const GrPath&);
    explicit GrPath(GrPathIter&);
    virtual ~GrPath();

    GrPathIter::ConvexHint getConvexHint() const { return fConvexHint; }
    void setConvexHint(GrPathIter::ConvexHint hint) { fConvexHint = hint; }

    void resetFromIter(GrPathIter*);

    // overrides from GrPathSink

    virtual void moveTo(GrScalar x, GrScalar y);
    virtual void lineTo(GrScalar x, GrScalar y);
    virtual void quadTo(GrScalar x0, GrScalar y0, GrScalar x1, GrScalar y1);
    virtual void cubicTo(GrScalar x0, GrScalar y0, GrScalar x1, GrScalar y1,
                         GrScalar x2, GrScalar y2);
    virtual void close();

    class Iter : public GrPathIter {
    public:
        Iter(const GrPath& path);

        // overrides from GrPathIter
        virtual Command next(GrPoint points[]);
        virtual ConvexHint hint() const;
        virtual Command next();
        virtual void rewind();
    private:
        const GrPath& fPath;
        GrPoint       fLastPt;
        int           fVerbIndex;
        int           fPtIndex;
    };

private:
    enum Verb {
        kMove, kLine, kQuad, kCubic, kClose
    };

    GrTDArray<uint8_t>      fVerbs;
    GrTDArray<GrPoint>      fPts;
    GrPathIter::ConvexHint  fConvexHint;

    // this ensures we have a moveTo at the start of each contour
    inline void ensureMoveTo();

    bool wasLastVerb(Verb verb) const {
        int count = fVerbs.count();
        return count > 0 && verb == fVerbs[count - 1];
    }

    friend class Iter;

    typedef GrPathSink INHERITED;
};

#endif

