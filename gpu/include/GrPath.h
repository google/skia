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

    GrConvexHint getConvexHint() const { return fConvexHint; }
    void setConvexHint(GrConvexHint hint) { fConvexHint = hint; }

    void resetFromIter(GrPathIter*);

    bool operator ==(const GrPath& path) const;
    bool operator !=(const GrPath& path) const { return !(*this == path); }
    // overrides from GrPathSink

    virtual void moveTo(GrScalar x, GrScalar y);
    virtual void lineTo(GrScalar x, GrScalar y);
    virtual void quadTo(GrScalar x0, GrScalar y0, GrScalar x1, GrScalar y1);
    virtual void cubicTo(GrScalar x0, GrScalar y0, GrScalar x1, GrScalar y1,
                         GrScalar x2, GrScalar y2);
    virtual void close();

    /**
     *  Offset the path by (tx, ty), adding tx to the horizontal position
     *  and adds ty to the vertical position of every point.
     */
    void offset(GrScalar tx, GrScalar ty);

    class Iter : public GrPathIter {
    public:
        /**
         * Creates an uninitialized iterator
         */
        Iter();

        Iter(const GrPath& path);

        // overrides from GrPathIter
        virtual GrPathCmd next(GrPoint points[]);
        virtual GrConvexHint convexHint() const;
        virtual GrPathCmd next();
        virtual void rewind();

        /**
         * Sets iterator to begining of path
         */
        void reset(const GrPath& path);
    private:
        const GrPath* fPath;
        GrPoint       fLastPt;
        int           fCmdIndex;
        int           fPtIndex;
    };

    static void ConvexUnitTest();

private:

    GrTDArray<GrPathCmd>    fCmds;
    GrTDArray<GrPoint>      fPts;
    GrConvexHint  fConvexHint;

    // this ensures we have a moveTo at the start of each contour
    inline void ensureMoveTo();

    bool wasLastVerb(GrPathCmd cmd) const {
        int count = fCmds.count();
        return count > 0 && cmd == fCmds[count - 1];
    }

    friend class Iter;

    typedef GrPathSink INHERITED;
};

#endif

