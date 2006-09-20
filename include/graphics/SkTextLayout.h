#ifndef SkTextLayout_DEFINED
#define SkTextLayout_DEFINED

#include "SkRefCnt.h"
#include "SkPaint.h"
#include "SkScalar.h"

class SkTextLayout : public SkRefCnt {
public:
    /** Create a textlayout that implements the CSS features of letter-spacing
        and word-spacing. It takes values to add to the advance width for each
        letter (charExtra) and to add to the advance width for each space
        (spaceExtra).
        @param charExtra    amount to add to every character's advance width
        @param spaceExtra   amount to add to every space character's advance width
        @return a new textlayout subclass that implements tracking
    */
    static SkTextLayout* CreateTrackingLayout(SkScalar charExtra, SkScalar spaceExtra);

    class Rec;

    int layout( const SkPaint& paint,
                const char* text, size_t byteLength, SkUnicodeWalkerProc proc,
                Rec rec[]);

    class Rec {
    public:
        SkUnichar   charCode() const { return fCharCode; }
        SkScalar    fDeltaAdvance;  //!< set by the subclass in onLayout()
    
    private:
        SkUnichar   fCharCode;
        // to be used in the future
        uint16_t    fGlyphID;
        uint16_t    fFlags;
        
        friend class SkTextLayout;
    };

protected:
    virtual void onLayout(const SkPaint& paint, Rec rec[], int count) {}
};

#endif
