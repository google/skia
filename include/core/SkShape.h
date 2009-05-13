#ifndef SkShape_DEFINED
#define SkShape_DEFINED

#include "SkFlattenable.h"
#include "SkMatrix.h"

class SkCanvas;
class SkWStream;

class SkShape : public SkFlattenable {
public:
            SkShape() : fMatrix(NULL) {}
    virtual ~SkShape();

    void getMatrix(SkMatrix*) const;
    void setMatrix(const SkMatrix&);
    void resetMatrix();

    void draw(SkCanvas*);

    /** Draw the shape translated by (dx,dy), which is applied before the
        shape's matrix (if any).
     */
    void drawXY(SkCanvas*, SkScalar dx, SkScalar dy);
    
    /** Draw the shape with the specified matrix, applied before the shape's
        matrix (if any).
     */
    void drawMatrix(SkCanvas*, const SkMatrix&);

    // overrides
    virtual void flatten(SkFlattenableWriteBuffer&);

protected:
    virtual void onDraw(SkCanvas*) = 0;

    SkShape(SkFlattenableReadBuffer&);

private:
    SkMatrix* fMatrix;

    static SkFlattenable* CreateProc(SkFlattenableReadBuffer&);

    typedef SkFlattenable INHERITED;
};

#endif
