#ifndef SkDrawable_DEFINED
#define SkDrawable_DEFINED

#include "SkFlattenable.h"
#include "SkMatrix.h"

class SkCanvas;
struct SkRect;

class SkDrawable : public SkFlattenable {
public:
            SkDrawable();
    virtual ~SkDrawable();

    void getMatrix(SkMatrix*) const;
    void setMatrix(const SkMatrix&);
    void resetMatrix();
        
    void draw(SkCanvas*);
    
    void inval() {}

    SkDrawable* attachChildToFront(SkDrawable* child);
    SkDrawable* attachChildToBack(SkDrawable* child);

    SkDrawable* getParent() const { return fParent; }
    void detachFromParent();
    void detachAllChildren();

    class B2FIter {
    public:
        B2FIter(const SkDrawable* parent);
        SkDrawable* next();
    private:
        SkDrawable* fFirstChild;
        SkDrawable* fChild;
    };
    
protected:
    virtual void onDraw(SkCanvas*) {}
    
private:
    SkMatrix    fMatrix;

    SkDrawable* fParent;
    SkDrawable* fFirstChild;
    SkDrawable* fNextSibling;
    SkDrawable* fPrevSibling;

    typedef SkFlattenable INHERITED;
};

#endif

