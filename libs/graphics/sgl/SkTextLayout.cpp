#include "SkTextLayout.h"
#include "SkPaint.h"

int SkTextLayout::layout(const SkPaint& paint,
                          const char* text, size_t byteLength, SkUnicodeWalkerProc proc,
                          Rec rec[])
{
    const char* stop = text + byteLength;
    Rec*        recStart = rec;
    
    while (text < stop)
    {
        rec->fCharCode = proc(&text);
        rec += 1;
        // set private fields of Rec (when we use them)
    }
    
    int count = rec - recStart;
    if (count > 0)
        this->onLayout(paint, recStart, count);
    return count;
}

/////////////////////////////////////////////////////////////////////////////////////////////

class SkTrackingTextLayout : public SkTextLayout {
public:
    SkTrackingTextLayout(SkScalar charExtra, SkScalar spaceExtra)
        : fCharExtra(charExtra), fSpaceExtra(spaceExtra) {}

protected:
    // override
    virtual void onLayout(const SkPaint& paint, Rec rec[], int count)
    {
        SkScalar ce = fCharExtra;
        SkScalar se = fSpaceExtra;

        if (0 == se)    // special case no space-extra (so we don't have to read charCode()
        {
            for (int i = 0; i < count; i++)
                rec[i].fDeltaAdvance = ce;
        }
        else
        {
            for (int i = 0; i < count; i++)
            {
                SkScalar delta = ce;
                if (32 == rec[i].charCode())    // do I need a fancier test?
                    delta += se;
                rec[i].fDeltaAdvance = delta;
            }
        }
    }

private:
    SkScalar    fCharExtra, fSpaceExtra;
};

/////////////////////////////////////////////////////////////////////////////////////////////

SkTextLayout* SkTextLayout::CreateTrackingLayout(SkScalar charExtra, SkScalar spaceExtra)
{
    return SkNEW_ARGS(SkTrackingTextLayout, (charExtra, spaceExtra));
}


