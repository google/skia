#ifndef SkEGLContext_DEFINED
#define SkEGLContext_DEFINED

#if defined(SK_BUILD_FOR_MAC)
    #include <AGL/agl.h>
#elif defined(SK_BUILD_FOR_UNIX)
    #include <X11/Xlib.h>
    #include <GL/glx.h>
#else

#endif

/**
 *  Create an offscreen opengl context
 */
class SkEGLContext {
public:
    SkEGLContext();
    ~SkEGLContext();

    bool init(int width, int height);

private:
#if defined(SK_BUILD_FOR_MAC)
    AGLContext context;
#elif defined(SK_BUILD_FOR_UNIX)
    GLXContext context;
    Display *display;
    Pixmap pixmap;
    GLXPixmap glxPixmap;
#else

#endif
};

#endif
