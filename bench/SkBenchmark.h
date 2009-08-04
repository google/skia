#ifndef SkBenchmark_DEFINED
#define SkBenchmark_DEFINED

#include "SkRefCnt.h"
#include "SkPoint.h"
#include "SkTRegistry.h"

class SkCanvas;
class SkPaint;

class SkBenchmark : public SkRefCnt {
public:
    SkBenchmark();

    const char* getName();
    SkIPoint getSize();
    void draw(SkCanvas*);
    
    void setForceAlpha(int alpha) {
        fForceAlpha = alpha;
    }
    
    void setForceAA(bool aa) {
        fForceAA = aa;
    }
    
    void setForceFilter(bool filter) {
        fForceFilter = filter;
    }
    
protected:
    void setupPaint(SkPaint* paint);

    virtual const char* onGetName() = 0;
    virtual void onDraw(SkCanvas*) = 0;

    virtual SkIPoint onGetSize();

private:
    int     fForceAlpha;
    bool    fForceAA;
    bool    fForceFilter;
};

static inline SkIPoint SkMakeIPoint(int x, int y) {
    SkIPoint p;
    p.set(x, y);
    return p;
}

typedef SkTRegistry<SkBenchmark*, void*> BenchRegistry;

#endif

