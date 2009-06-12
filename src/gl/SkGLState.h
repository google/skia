
#ifndef SkGLState_DEFINED
#define SkGLState_DEFINED

#include "SkGL.h"
#include "SkSize.h"

class SkGLState {
public:
    static SkGLState& GlobalState() { return gState; }

    SkGLState();
    
    void reset();

    // internally, these are bit_shifts, so they must be 0, 1, ...
    enum Caps {
        kDITHER,
        kTEXTURE_2D,
    };
    void enable(Caps);
    void disable(Caps);
    
    // internally, these are bit_shifts, so they must be 0, 1, ...
    enum ClientState {
        kTEXTURE_COORD_ARRAY,
        kCOLOR_ARRAY,
    };
    void enableClientState(ClientState);
    void disableClientState(ClientState);

    // we use -1 for unknown, so the enum must be >= 0
    enum ShadeModel {
        kFLAT,
        kSMOOTH,
    };
    void shadeModel(ShadeModel);

    void scissor(int x, int y, int w, int h);
    
    void color(SkColor c) {
        this->pmColor(SkPreMultiplyColor(c));
    }
    void alpha(U8CPU a) {
        this->pmColor((a << 24) | (a << 16) | (a << 8) | a);
    }
    void pmColor(SkPMColor);

    void blendFunc(GLenum src, GLenum dst);
    
    void pointSize(float);
    void lineWidth(float);

private:
    void init();

    unsigned    fCapBits;
    unsigned    fClientStateBits;
    int         fShadeModel;
    SkIPoint    fScissorLoc;
    SkISize     fScissorSize;
    SkPMColor   fPMColor;
    GLenum      fSrcBlend, fDstBlend;
    float       fPointSize;
    float       fLineWidth;
    
    const GLenum* fCapsPtr;
    const GLenum* fClientPtr;
    const GLenum* fShadePtr;
    
    static SkGLState gState;
};

#endif
