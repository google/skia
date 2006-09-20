#ifndef SkShaderExtras_DEFINED
#define SkShaderExtras_DEFINED

#include "SkShader.h"

class SkColorCombine :  public SkRefCnt {
public:
    /** Called with two scanlines of color. The implementation writes out its combination of
        those into the result[] scaline.
    */
    virtual void combineSpan(const SkPMColor srcA[], const SkPMColor srcB[], int count, SkPMColor result[]) = 0;
};

///////////////////////////////////////////////////////////////////////////////////////////

class SkComposeShader : public SkShader {
public:
    SkComposeShader(SkShader* sA, SkShader* sB, SkColorCombine* combine);
    virtual ~SkComposeShader();
    
    // override
    virtual bool setContext(const SkBitmap& device, const SkPaint& paint, const SkMatrix& matrix);
    virtual void shadeSpan(int x, int y, SkPMColor result[], int count);

private:
    enum {
        COUNT = 32
    };
    SkShader*       fShaderA;
    SkShader*       fShaderB;
    SkColorCombine* fCombine;

    typedef SkShader INHERITED;
};

#endif
