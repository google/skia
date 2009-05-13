#ifndef SkGroupShape_DEFINED
#define SkGroupShape_DEFINED

#include "SkShape.h"
#include "SkTDArray.h"

class SkGroupShape : public SkShape {
public:
            SkGroupShape();
    virtual ~SkGroupShape();

    /** Return the number of child shapes in this group
     */
    int countShapes() const;

    /** Return the shape at the specified index. Note this does not affect the
        owner count of the index'd shape. If index is out of range, returns NULL
     */
    SkShape* getShape(int index) const;

    /** Ref the specified shape, and insert it into the child list at the
        specified index. If index == countShapes(), then the shape will be
        appended to the child list, otherwise if index is out of range, the
        shape is not added. Either way, the shape parameter is returned.
     
        Child shapes are drawn in order, after the parent, so the shape at index
        0 will be drawn first, and the shape at index countShapes() - 1 will be
        drawn last.
     */
    SkShape* addShape(int index, SkShape*);

    /** Helper method to append a shape, passing countShapes() for the index
     */
    SkShape* appendShape(SkShape* shape) {
        return this->addShape(this->countShapes(), shape);
    }

    /** Unref the specified index, and remove it from the child list. If index
        is out of range, does nothing.
     */
    void removeShape(int index);

    /** Unrefs and removes all of the child shapes
     */
    void removeAllShapes();

    // overrides
    virtual Factory getFactory();
    virtual void flatten(SkFlattenableWriteBuffer&);

protected:
    // overrides
    virtual void onDraw(SkCanvas*);

    SkGroupShape(SkFlattenableReadBuffer&);

private:
    SkTDArray<SkShape*> fList;

    static SkFlattenable* CreateProc(SkFlattenableReadBuffer&);

    typedef SkShape INHERITED;
};

#endif
