#ifndef SkBenchmark_DEFINED
#define SkBenchmark_DEFINED

#include "SkRefCnt.h"
#include "SkPoint.h"

class SkCanvas;

class SkBenchmark : public SkRefCnt {
public:
    const char* getName();
    SkIPoint getSize();
    void draw(SkCanvas*);

protected:
    virtual const char* onGetName() = 0;
    virtual SkIPoint onGetSize() = 0;
    virtual void onDraw(SkCanvas*) = 0;
};

static inline SkIPoint SkMakeIPoint(int x, int y) {
    SkIPoint p;
    p.set(x, y);
    return p;
}

///////////////////////////////////////////////////////////////////////////////

template <typename T> class SkTRegistry : SkNoncopyable {
public:
    typedef T* (*Factory)();
    
    SkTRegistry(Factory fact) {
        fFact = fact;
        fChain = gHead;
        gHead = this;
    }
    
    static const SkTRegistry* Head() { return gHead; }
    
    SkTRegistry* next() const { return fChain; }
    Factory factory() const { return fFact; }
    
private:
    Factory   fFact;
    SkTRegistry* fChain;
    
    static SkTRegistry* gHead;
};

template <typename T> SkTRegistry<T>* SkTRegistry<T>::gHead;

#endif

