#include "SkGLState.h"
#include "SkColorPriv.h"

// here is our global instance
SkGLState SkGLState::gState;

// this is an illegal pmcolor, since its alpha (0) is less than its red
#define UNKNOWN_PMCOLOR (SK_R32_MASK << SK_R32_SHIFT)

#define UNKNOWN_GLENUM  ((GLenum)-1)

// MUST be in the same order as SkGLState::Caps enum
static const GLenum gCapsTable[] = {
    GL_DITHER,
    GL_TEXTURE_2D,
};

// MUST be in the same order as SkGLState::ClientState enum
static const GLenum gClientStateTable[] = {
    GL_TEXTURE_COORD_ARRAY,
    GL_COLOR_ARRAY,
};

static const GLenum gShadeModelTable[] = {
    GL_FLAT,
    GL_SMOOTH
};

///////////////////////////////////////////////////////////////////////////////

SkGLState::SkGLState() :
        fCapsPtr(gCapsTable),
        fClientPtr(gClientStateTable),
        fShadePtr(gShadeModelTable) {
    this->init();
}

void SkGLState::init() {
    fCapBits = 0;
    fClientStateBits = 0;
    fShadeModel = UNKNOWN_GLENUM;
    fScissorSize.set(-1, -1);
    fPMColor = UNKNOWN_PMCOLOR;
    fSrcBlend = fDstBlend = UNKNOWN_GLENUM;
    fPointSize = fLineWidth = -1;
}

void SkGLState::reset() {
    this->init();

    size_t i;
    for (i = 0; i < SK_ARRAY_COUNT(gCapsTable); i++) {
        glDisable(fCapsPtr[i]);
    }
    for (i = 0; i < SK_ARRAY_COUNT(gClientStateTable); i++) {
        glDisableClientState(fClientPtr[i]);
    }
}

///////////////////////////////////////////////////////////////////////////////

void SkGLState::enable(Caps c) {
    unsigned mask = 1 << c;
    if ((fCapBits & mask) == 0) {
        fCapBits |= mask;
        glEnable(fCapsPtr[c]);
    }
}

void SkGLState::disable(Caps c) {
    unsigned mask = 1 << c;
    if (fCapBits & mask) {
        fCapBits &= ~mask;
        glDisable(fCapsPtr[c]);
    }
}

void SkGLState::enableClientState(ClientState c) {
    unsigned mask = 1 << c;
    if ((fClientStateBits & mask) == 0) {
        fClientStateBits |= mask;
        glEnableClientState(fClientPtr[c]);
    }
}

void SkGLState::disableClientState(ClientState c) {
    unsigned mask = 1 << c;
    if (fClientStateBits & mask) {
        fClientStateBits &= ~mask;
        glDisableClientState(fClientPtr[c]);
    }
}

void SkGLState::shadeModel(ShadeModel s) {
    if (fShadeModel != s) {
        fShadeModel = s;
        glShadeModel(fShadePtr[s]);
    }
}

void SkGLState::scissor(int x, int y, int w, int h) {
    SkASSERT(w >= 0 && h >= 0);
    if (!fScissorLoc.equals(x, y) || !fScissorSize.equals(w, h)) {
        fScissorLoc.set(x, y);
        fScissorSize.set(w, h);
        glScissor(x, y, w, h);
    }
}

void SkGLState::pointSize(float x) {
    if (fPointSize != x) {
        fPointSize = x;
        glPointSize(x);
    }
}

void SkGLState::lineWidth(float x) {
    if (fLineWidth != x) {
        fLineWidth = x;
        glLineWidth(x);
    }
}

void SkGLState::blendFunc(GLenum src, GLenum dst) {
    if (fSrcBlend != src || fDstBlend != dst) {
        fSrcBlend = src;
        fDstBlend = dst;
        glBlendFunc(src, dst);
    }
}

///////////////////////////////////////////////////////////////////////////////

#ifdef SK_GL_HAS_COLOR4UB
    static inline void gl_pmcolor(U8CPU r, U8CPU g, U8CPU b, U8CPU a) {
        glColor4ub(r, g, b, a);
    }
#else
    static inline SkFixed byte2fixed(U8CPU value) {
        return ((value << 8) | value) + (value >> 7);
    }

    static inline void gl_pmcolor(U8CPU r, U8CPU g, U8CPU b, U8CPU a) {
        glColor4x(byte2fixed(r), byte2fixed(g), byte2fixed(b), byte2fixed(a));
    }
#endif

void SkGLState::pmColor(SkPMColor c) {
    if (fPMColor != c) {
        fPMColor = c;
        gl_pmcolor(SkGetPackedR32(c), SkGetPackedG32(c),
                   SkGetPackedB32(c), SkGetPackedA32(c));
    }
}

