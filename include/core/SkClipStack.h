#ifndef SkClipStack_DEFINED
#define SkClipStack_DEFINED

#include "SkDeque.h"
#include "SkRegion.h"

struct SkRect;
class SkPath;

class SkClipStack {
public:
    SkClipStack();
    ~SkClipStack() {}

    void reset();

    int getSaveCount() const { return fSaveCount; }
    void save();
    void restore();

    void clipDevRect(const SkIRect& ir,
                     SkRegion::Op op = SkRegion::kIntersect_Op) {
        SkRect r;
        r.set(ir);
        this->clipDevRect(r, op);
    }
    void clipDevRect(const SkRect&, SkRegion::Op = SkRegion::kIntersect_Op);
    void clipDevPath(const SkPath&, SkRegion::Op = SkRegion::kIntersect_Op);

    class B2FIter {
    public:
        B2FIter(const SkClipStack& stack);

        struct Clip {
            const SkRect*   fRect;  // if non-null, this is a rect clip
            const SkPath*   fPath;  // if non-null, this is a path clip
            SkRegion::Op    fOp;
        };

        /**
         *  Return the clip for this element in the iterator. If next() returns
         *  NULL, then the iterator is done. The type of clip is determined by
         *  the pointers fRect and fPath:
         *
         *  fRect==NULL  fPath!=NULL    path clip
         *  fRect!=NULL  fPath==NULL    rect clip
         *  fRect==NULL  fPath==NULL    empty clip
         */
        const Clip* next();

    private:
        Clip             fClip;
        SkDeque::F2BIter fIter;
    };

private:
    friend class B2FIter;
    struct Rec;

    SkDeque fDeque;
    int     fSaveCount;
};

#endif

