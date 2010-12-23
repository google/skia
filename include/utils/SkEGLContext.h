#ifndef SkEGLContext_DEFINED
#define SkEGLContext_DEFINED

#include "SkTypes.h"

/**
 *  Create an offscreen opengl context
 */
class SkEGLContext {
public:
	SkEGLContext();
	~SkEGLContext();

	bool init(int width, int height);

private:
	void* fContext;
};

#endif
